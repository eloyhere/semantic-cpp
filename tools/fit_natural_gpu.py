#!/usr/bin/env python3
"""Optional PyTorch/CUDA cross-check of the C++ natural-calendar fit.

This is not part of the required pipeline; it re-fits the same two compact
models on the GPU with chunked normal equations and reports the agreement with
the CPU C++ fit.  It exists to document the GPU experiment requested during the
fit session.

Usage:
    python tools/fit_natural_gpu.py
"""

from __future__ import annotations

import json
import sys
import time
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "temporal" / "natural" / "data"
CPP = DATA / "cpp"

K = 36525.0
J2000 = 2451545.0
PI = np.pi


def read_table(path: Path, doubles: int, ints: int):
    with path.open("rb") as fh:
        magic = fh.read(8)
        if magic != b"NFIT0001":
            raise RuntimeError(f"bad magic in {path}")
        n = int(np.fromfile(fh, dtype=np.int64, count=1)[0])
        arrays = []
        for _ in range(doubles):
            arrays.append(np.fromfile(fh, dtype=np.float64, count=n))
        for _ in range(ints):
            arrays.append(np.fromfile(fh, dtype=np.int32, count=n))
    return n, arrays


def omega(period_days: float) -> float:
    return 2.0 * PI * K / period_days


SUN_EXTRA = [18.61, 11.862, 1.0921, 1.5987, 0.7994, 29.457, 0.6152, 2.135,
             0.317, 8.1, 3.983, 2.47, 1.2028, 1.1408, 1.068, 0.546, 0.533,
             0.572, 0.333, 0.444]


def torch_fit(name: str, jd: np.ndarray, y: np.ndarray, kind: str):
    import torch

    if not torch.cuda.is_available():
        raise RuntimeError("CUDA is not available")
    device = "cuda"
    dtype = torch.float64
    x = torch.as_tensor((jd - J2000) / K, device=device, dtype=dtype)
    xs = x / 100.0
    yt = torch.as_tensor(y, device=device, dtype=dtype)

    wa = omega(365.259636)
    wsyn = omega(29.530588)
    wano = omega(27.554550)
    wdra = omega(27.212221)
    wann = omega(365.259636)
    weve = 2.0 * wsyn - wano
    wvar = 2.0 * wsyn

    specs = []
    poly = 0
    if kind == "sun":
        poly = 6
        specs = [(k * wa, d) for k, d in enumerate([4, 2, 1, 1], start=1)]
        specs += [(omega(p * 365.25), 0) for p in SUN_EXTRA]
    else:
        poly = 4
        specs = [
            (wano, 4), (2 * wano, 3), (3 * wano, 2), (weve, 3), (wvar, 2),
            (wann, 2), (wdra, 2), (3 * wsyn, 1), (2 * wsyn - 2 * wano, 2),
            (2 * wsyn - wann, 2), (wsyn - 2 * wano, 2), (wsyn - wann, 2),
            (wsyn - wdra, 2),
        ]
    p = poly + (1 if kind == "moon" else 0) + sum(2 * (d + 1) for _, d in specs)

    def build(xc, xsc):
        cols = [xsc ** i for i in range(poly)]
        if kind == "moon":
            cols.append(xc)
        for w, d in specs:
            for m in range(d + 1):
                cols.append(torch.cos(w * xc) * xsc ** m)
                cols.append(torch.sin(w * xc) * xsc ** m)
        return torch.stack(cols, dim=1)

    t0 = time.time()
    ata = torch.zeros((p, p), device=device, dtype=dtype)
    aty = torch.zeros(p, device=device, dtype=dtype)
    chunk = 50_000
    for start in range(0, len(jd), chunk):
        end = min(start + chunk, len(jd))
        a = build(x[start:end], xs[start:end])
        ata += a.T @ a
        aty += a.T @ yt[start:end]
    ata.diagonal().add_(1e-12 * torch.trace(ata) / p)
    coef = torch.linalg.solve(ata, aty)

    sse = 0.0
    max_abs = 0.0
    for start in range(0, len(jd), chunk):
        end = min(start + chunk, len(jd))
        a = build(x[start:end], xs[start:end])
        r = yt[start:end] - a @ coef
        sse += torch.sum(r * r).item()
        max_abs = max(max_abs, torch.max(torch.abs(r)).item())
    seconds = time.time() - t0
    return coef.cpu().numpy(), float(np.sqrt(sse / len(jd))), max_abs, seconds, p


def main() -> int:
    sys.stdout.reconfigure(encoding="utf-8")
    import torch

    print("torch", torch.__version__, "cuda", torch.version.cuda,
          "device", torch.cuda.get_device_name(0))
    n_sun, sun = read_table(CPP / "sun_events.bin", 2, 3)
    n_moon, moon = read_table(CPP / "moon_events.bin", 2, 2)
    print("events", n_sun, n_moon)

    from fit_natural import unwrap_angles
    sun_y = unwrap_angles(sun[0], sun[1])
    moon_y = unwrap_angles(moon[0], moon[1])

    results = {"device": torch.cuda.get_device_name(0),
               "torch": torch.__version__, "cuda": torch.version.cuda}
    for name, n, jd, y, kind in (("sun", n_sun, sun[0], sun_y, "sun"),
                                 ("moon", n_moon, moon[0], moon_y, "moon")):
        coef, rms, max_abs, seconds, p = torch_fit(name, jd, y, kind)
        results[name] = {"params": p, "rmsDegrees": rms, "maxDegrees": max_abs,
                         "gpuSeconds": seconds}
        print(f"{name}: p={p} rms={rms:.9f} max={max_abs:.9f} gpu={seconds:.1f}s")

    # Compare with the C++ coefficients by evaluating the same basis.
    cpp_path = DATA / "cpp_coefficients.json"
    if cpp_path.exists():
        cpp = json.loads(cpp_path.read_text(encoding="utf-8"))
        for name in ("sun", "moon"):
            if name == "sun":
                # Evaluate the C++ sun model directly from its JSON description.
                s = cpp["sun"]
                jd = sun[0]
                x = (jd - J2000) / K
                xs = x / 100.0
                v = np.zeros_like(x)
                for i, c in enumerate(s["polynomial"]):
                    v += c * xs ** i
                for e in s["envelopes"]:
                    for m, (c, ss) in enumerate(zip(e["cos"], e["sin"])):
                        v += (c * np.cos(e["omega"] * x) + ss * np.sin(e["omega"] * x)) * xs ** m
                for term in s["extras"]:
                    v += term["amplitude"] * np.cos(term["omega"] * x + term["phase"])
                r = sun_y - v
            else:
                s = cpp["moon"]
                jd = moon[0]
                x = (jd - J2000) / K
                xs = x / 100.0
                v = s["ratePerCentury"] * x
                for i, c in enumerate(s["polynomial"]):
                    v += c * xs ** i
                for e in s["envelopes"]:
                    for m, (c, ss) in enumerate(zip(e["cos"], e["sin"])):
                        v += (c * np.cos(e["omega"] * x) + ss * np.sin(e["omega"] * x)) * xs ** m
                r = moon_y - v
            results[name]["cppRms"] = float(np.sqrt(np.mean(r * r)))
            results[name]["cppMax"] = float(np.max(np.abs(r)))
            print(f"{name}: cpp rms={results[name]['cppRms']:.9f}")

    out = DATA / "gpu_metrics.json"
    out.write_text(json.dumps(results, ensure_ascii=False, indent=2), encoding="utf-8")
    print("wrote", out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
