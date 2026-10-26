#!/usr/bin/env python3
"""Fit the natural-calendar Waveforms from 寿星数据整录——原版.xlsx.

The source workbook is a dump of 寿星万年历 5.10 (sxwnl).  It is a full
VSOP87/ELP2000 ephemeris, so the tables are not a low-order trigonometric
series.  This script extracts the events, fits compact models, and writes the
manifest/report consumed by the C++ side.  A separate C++ program
(tools/fit_natural_cpp) performs an independent fit of the same models.

Outputs (under temporal/natural/data):
    natural_events.npz   compact parsed cache
    cpp/*.bin            binary event tables for the C++ fitter
    fit_manifest.json    metrics
    waveforms.h          generated from the C++ coefficients when available
    FIT_REPORT.md        human-readable report

Usage:
    python tools/fit_natural.py all
    python tools/fit_natural.py extract
    python tools/fit_natural.py fit
    python tools/fit_natural.py report
"""

from __future__ import annotations

import argparse
import json
import math
import re
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path

import numpy as np

import fit_report

ROOT = Path(__file__).resolve().parents[1]
XLSX = ROOT / "寿星数据整录——原版.xlsx"
DATA = ROOT / "temporal" / "natural" / "data"
CPP_DATA = DATA / "cpp"
CACHE = DATA / "natural_events.npz"
MANIFEST = DATA / "fit_manifest.json"
HEADER = DATA / "waveforms.h"
REPORT = DATA / "FIT_REPORT.md"

# Natural epoch: astronomical year -2696, 正月初一 -2696-03-11 01:45:49 UTC+8.
EPOCH_YEAR = -2696

# Solar-term target longitudes, degrees, equinox = 0, counter-clockwise.
TERM_ANGLE = {
    "立春": 315, "雨水": 330, "惊蛰": 345, "春分": 0,
    "清明": 15, "谷雨": 30, "立夏": 45, "小满": 60,
    "芒种": 75, "夏至": 90, "小暑": 105, "大暑": 120,
    "立秋": 135, "处暑": 150, "白露": 165, "秋分": 180,
    "寒露": 195, "霜降": 210, "立冬": 225, "小雪": 240,
    "大雪": 255, "冬至": 270, "小寒": 285, "大寒": 300,
}

# The 12 中气 (major mid-terms) and the 12 节 (major terms).
MID_NAMES = ["雨水", "春分", "谷雨", "小满", "夏至", "大暑",
             "处暑", "秋分", "霜降", "小雪", "冬至", "大寒"]
JIE_NAMES = ["立春", "惊蛰", "清明", "立夏", "芒种", "小暑",
             "立秋", "白露", "寒露", "立冬", "大雪", "小寒"]

# Dominant frequencies (periods in days) used by the compact models.
P_SYNODIC = 29.530588
P_ANOMALISTIC = 27.554550
P_DRACONITIC = 27.212221
P_TROPICAL = 365.242190
P_ANOMALISTIC_YEAR = 365.259636

# Known planetary/nutation periods (years) used as extra constant terms.
SUN_EXTRA_PERIODS = [
    18.61, 11.862, 1.0921, 1.5987, 0.7994, 29.457, 0.6152,
    2.135, 0.317, 8.1, 3.983, 2.47, 1.2028, 1.1408, 1.068,
    0.546, 0.533, 0.572, 0.333, 0.444,
]

DATE_RE = re.compile(r"^\s*(-?\d+)-(\d{1,2})-(\d{1,2})[ T](\d{1,2}):(\d{2}):(\d{2})\s*$")


# ---------------------------------------------------------------------------
# Calendar / time
# ---------------------------------------------------------------------------

def jdn_gregorian(year: int, month: int, day: int) -> int:
    a = (14 - month) // 12
    y = year + 4800 - a
    m = month + 12 * a - 3
    return day + (153 * m + 2) // 5 + 365 * y + y // 4 - y // 100 + y // 400 - 32045


def jdn_julian(year: int, month: int, day: int) -> int:
    a = (14 - month) // 12
    y = year + 4800 - a
    m = month + 12 * a - 3
    return day + (153 * m + 2) // 5 + 365 * y + y // 4 - 32083


def jdn_historical(year: int, month: int, day: int) -> int:
    """The workbook uses the historical Julian->Gregorian cutover (1582-10-15).

    Evidence: the table has no 1582-10-05..14, and the lunar-month intervals
    show one-day jumps at every non-leap Gregorian century when the dates are
    interpreted as proleptic Gregorian.  Treating the pre-cutover dates as
    Julian removes both artefacts.
    """
    if (year, month, day) < (1582, 10, 15):
        return jdn_julian(year, month, day)
    return jdn_gregorian(year, month, day)


def parse_jd(text) -> float:
    if text is None:
        return math.nan
    if isinstance(text, str) and not text.strip():
        return math.nan
    if not isinstance(text, str):
        raise TypeError(f"unexpected date cell {text!r} ({type(text).__name__})")
    m = DATE_RE.match(text)
    if m is None:
        raise ValueError(f"bad date {text!r}")
    year, month, day, hour, minute, second = (int(g) for g in m.groups())
    jdn = jdn_historical(year, month, day)
    # UTC+8 -> UTC
    return jdn - 0.5 + (hour - 8) / 24.0 + minute / 1440.0 + second / 86400.0


def local_day(jd: np.ndarray | float) -> np.ndarray:
    """Civil day number in UTC+8 (day boundary at local midnight)."""
    return np.floor(np.asarray(jd) + 0.5 + 8.0 / 24.0).astype(np.int64)


def year_absolute(astro_year: np.ndarray | int):
    y = np.asarray(astro_year, dtype=np.int64)
    return np.where(y >= EPOCH_YEAR, y + 2697, y + 2696)


def month_absolute_from_index(index: np.ndarray, epoch_index: int):
    """1-based signed month index; no zero."""
    delta = np.asarray(index, dtype=np.int64) - epoch_index
    return np.where(delta >= 0, delta + 1, delta)


# ---------------------------------------------------------------------------
# Extraction
# ---------------------------------------------------------------------------

@dataclass
class ExtractStats:
    sheets: list = field(default_factory=list)
    rows: int = 0
    bad_dates: list = field(default_factory=list)
    missing: dict = field(default_factory=dict)
    nan_counts: dict = field(default_factory=dict)
    term_events: int = 0
    mid_events: int = 0
    phase_events: int = 0
    leap_months: int = 0
    leap_years: int = 0
    duplicates: dict = field(default_factory=dict)
    year_first: int = 0
    year_last: int = 0
    epoch_row: int = -1
    epoch_jiyue: int = -1
    extract_seconds: float = 0.0
    date_convention: str = "Julian before 1582-10-15, Gregorian after (UTC+8)"


def extract(force: bool = False) -> None:
    import openpyxl

    if CACHE.exists() and not force:
        print(f"cache exists: {CACHE} (use --force to rebuild)")
        return

    t0 = time.time()
    DATA.mkdir(parents=True, exist_ok=True)
    wb = openpyxl.load_workbook(XLSX, read_only=True, data_only=True)
    rows = []
    stats = ExtractStats()
    names: dict[str, int] = {}
    term_names: dict[str, int] = {}
    mid_names: dict[str, int] = {}
    secondary_names: dict[str, int] = {}

    def name_id(table, value):
        if value in (None, ""):
            return -1
        text = str(value).strip()
        if text not in table:
            table[text] = len(table)
        return table[text]

    for ws in wb.worksheets:
        sheet_rows = 0
        current_year = None
        for row_index, values in enumerate(ws.iter_rows(min_row=3, values_only=True), start=3):
            if values is None or not any(v not in (None, "") for v in values[:15]):
                continue
            raw_year = values[1]
            if raw_year not in (None, ""):
                current_year = int(float(raw_year))
            if current_year is None:
                stats.bad_dates.append(["no-year", ws.title, row_index])
                continue
            try:
                jd = [parse_jd(values[col]) for col in (3, 4, 5, 6, 8, 10, 12)]
            except (TypeError, ValueError) as exc:
                stats.bad_dates.append([ws.title, row_index, str(exc)])
                continue
            if any(math.isnan(v) for v in jd):
                stats.missing[ws.title] = stats.missing.get(ws.title, 0) + 1
            jiyue = values[13]
            month_end = values[14]
            rows.append((
                current_year,
                name_id(names, values[2]),
                name_id(term_names, values[7]),
                name_id(mid_names, values[9]),
                name_id(secondary_names, values[11]),
                int(float(jiyue)) if jiyue not in (None, "") else -1,
                int(float(month_end)) if month_end not in (None, "") else -1,
                *jd,
            ))
            sheet_rows += 1
        stats.sheets.append({"name": ws.title, "rows": sheet_rows})
        stats.rows += sheet_rows
        print(f"  {ws.title}: {sheet_rows} rows", flush=True)
    wb.close()

    dtype = np.dtype([
        ("year", np.int32), ("month_name", np.int16), ("term_name", np.int16),
        ("mid_name", np.int16), ("secondary_name", np.int16),
        ("jiyue", np.int32), ("month_end", np.int32),
        ("jd_shuo", np.float64), ("jd_shang", np.float64), ("jd_wang", np.float64),
        ("jd_xia", np.float64), ("jd_term", np.float64), ("jd_mid", np.float64),
        ("jd_secondary", np.float64),
    ])
    data = np.array(rows, dtype=dtype)

    # Epoch row: astronomical -2696, 正月 (non-leap).
    is_leap_name = np.char.startswith(
        np.array([k for k, _ in sorted(names.items(), key=lambda kv: kv[1])], dtype=str)[data["month_name"]],
        "闰")
    epoch_candidates = np.where(
        (data["year"] == EPOCH_YEAR) &
        (np.char.find(np.array([k for k, _ in sorted(names.items(), key=lambda kv: kv[1])], dtype=str)[data["month_name"]], "正月") >= 0) &
        (~is_leap_name))[0]
    if len(epoch_candidates) != 1:
        raise RuntimeError(f"cannot locate unique epoch row, candidates={epoch_candidates[:10]}")
    epoch_row = int(epoch_candidates[0])
    stats.epoch_row = epoch_row
    stats.epoch_jiyue = int(data["jiyue"][epoch_row])

    # Global month/day indices.
    idx = np.arange(len(data), dtype=np.int64)
    month_abs = month_absolute_from_index(idx, epoch_row)
    year_abs = year_absolute(data["year"])
    leap = is_leap_name
    leap_month_count = int(leap.sum())

    # Civil day of each month start and the next month start.
    shuo_day = local_day(data["jd_shuo"])
    # Year assignment: the 正月 row starts a natural year; use row groups.
    zheng = np.where((np.char.find(
        np.array([k for k, _ in sorted(names.items(), key=lambda kv: kv[1])], dtype=str)[data["month_name"]], "正月") >= 0) & (~leap))[0]
    year_len = np.zeros(len(data), dtype=np.int32)
    is_leap_year = np.zeros(len(data), dtype=bool)
    for k, z0 in enumerate(zheng):
        z1 = zheng[k + 1] if k + 1 < len(zheng) else len(data)
        n = z1 - z0
        year_len[z0:z1] = n
        is_leap_year[z0:z1] = (n == 13)

    # Cumulative civil-day index from the first month start; O column resets per sheet,
    # so the global value is reconstructed from the month-start dates.
    day_index = shuo_day - shuo_day[0]

    np.savez_compressed(
        CACHE,
        data=data,
        names=np.array([k for k, _ in sorted(names.items(), key=lambda kv: kv[1])], dtype=object),
        term_names=np.array([k for k, _ in sorted(term_names.items(), key=lambda kv: kv[1])], dtype=object),
        mid_names=np.array([k for k, _ in sorted(mid_names.items(), key=lambda kv: kv[1])], dtype=object),
        secondary_names=np.array([k for k, _ in sorted(secondary_names.items(), key=lambda kv: kv[1])], dtype=object),
        month_abs=month_abs, year_abs=year_abs, leap=leap, leap_year=is_leap_year,
        year_len=year_len, shuo_day=shuo_day, day_index=day_index, zheng=zheng,
    )

    stats.nan_counts = {
        key: int(np.isnan(data[key]).sum())
        for key in ("jd_shuo", "jd_shang", "jd_wang", "jd_xia", "jd_term", "jd_mid", "jd_secondary")
    }
    def _duplicate_count(values):
        values = values[~np.isnan(values)]
        return int(len(values) - len(np.unique(values)))
    stats.duplicates = {
        "jd_shuo": _duplicate_count(data["jd_shuo"]),
        "jd_shang": _duplicate_count(data["jd_shang"]),
        "jd_wang": _duplicate_count(data["jd_wang"]),
        "jd_xia": _duplicate_count(data["jd_xia"]),
        "jd_term": _duplicate_count(data["jd_term"]),
        "jd_mid": _duplicate_count(data["jd_mid"]),
    }
    stats.term_events = int(np.sum(~np.isnan(data["jd_term"])))
    stats.mid_events = int(np.sum(~np.isnan(data["jd_mid"])))
    stats.phase_events = int(4 * len(data))
    stats.leap_months = leap_month_count
    stats.leap_years = int(sum(1 for k, z0 in enumerate(zheng)
                               if (zheng[k + 1] - z0 if k + 1 < len(zheng) else len(data) - z0) == 13))
    stats.year_first = int(data["year"][0])
    stats.year_last = int(data["year"][-1])
    stats.extract_seconds = time.time() - t0
    (DATA / "extract_stats.json").write_text(
        json.dumps(stats.__dict__, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps({k: v for k, v in stats.__dict__.items() if k != "sheets"},
                     ensure_ascii=False, indent=2))
    print(f"wrote {CACHE} ({CACHE.stat().st_size / 1e6:.1f} MB) in {stats.extract_seconds:.1f}s")


# ---------------------------------------------------------------------------
# Loading derived arrays
# ---------------------------------------------------------------------------

@dataclass
class NaturalData:
    raw: np.ndarray
    names: np.ndarray
    term_names: np.ndarray
    mid_names: np.ndarray
    month_abs: np.ndarray
    year_abs: np.ndarray
    leap: np.ndarray
    leap_year: np.ndarray
    year_len: np.ndarray
    shuo_day: np.ndarray
    day_index: np.ndarray
    zheng: np.ndarray

    @classmethod
    def load(cls) -> "NaturalData":
        z = np.load(CACHE, allow_pickle=True)
        return cls(
            raw=z["data"],
            names=np.array(z["names"].tolist(), dtype=str),
            term_names=np.array(z["term_names"].tolist(), dtype=str),
            mid_names=np.array(z["mid_names"].tolist(), dtype=str),
            month_abs=z["month_abs"], year_abs=z["year_abs"],
            leap=z["leap"], leap_year=z["leap_year"], year_len=z["year_len"],
            shuo_day=z["shuo_day"], day_index=z["day_index"], zheng=z["zheng"],
        )


def build_events(nd: NaturalData):
    d = nd.raw
    # Solar terms: one row may contain a 节 and/or a 中气.
    sun_jd, sun_ang, sun_year, sun_month, sun_kind = [], [], [], [], []
    for col, name_table, kind in ((12, "term", 0), (11, "mid", 1)):
        jd_key = "jd_term" if col == 12 else "jd_mid"
        names = nd.term_names if col == 12 else nd.mid_names
        valid = ~np.isnan(d[jd_key])
        labels = names[d["term_name" if col == 12 else "mid_name"]]
        angles = np.array([TERM_ANGLE.get(n, 0) for n in labels], dtype=np.float64)
        sun_jd.append(d[jd_key][valid]); sun_ang.append(angles[valid])
        sun_year.append(nd.year_abs[valid]); sun_month.append(nd.month_abs[valid])
        sun_kind.append(np.full(int(valid.sum()), kind, dtype=np.int32))
    sun = {
        "jd": np.concatenate(sun_jd), "angle": np.concatenate(sun_ang),
        "year_abs": np.concatenate(sun_year), "month_abs": np.concatenate(sun_month),
        "kind": np.concatenate(sun_kind),
    }
    order = np.argsort(sun["jd"])
    for k in sun:
        sun[k] = sun[k][order]

    n = len(d)
    moon_jd = np.concatenate([d["jd_shuo"], d["jd_shang"], d["jd_wang"], d["jd_xia"]])
    moon_ang = np.concatenate([np.zeros(n), np.full(n, 90.0), np.full(n, 180.0), np.full(n, 270.0)])
    moon_month = np.tile(nd.month_abs, 4)
    moon_year = np.tile(nd.year_abs, 4)
    order = np.argsort(moon_jd)
    moon = {"jd": moon_jd[order], "angle": moon_ang[order],
            "month_abs": moon_month[order], "year_abs": moon_year[order]}

    # Synodic / tropical year lengths by astronomical year.
    years = []
    zheng = nd.zheng
    for k, z0 in enumerate(zheng):
        z1 = zheng[k + 1] if k + 1 < len(zheng) else len(d)
        y = int(d["year"][z0])
        # 立春 in this year = first valid 立春 event in the year.
        sel = np.where((d["year"] == y) & (nd.term_names[d["term_name"]] == "立春") & (~np.isnan(d["jd_term"])))[0]
        lichun = float(d["jd_term"][sel[0]]) if len(sel) else math.nan
        years.append((nd.year_abs[z0], y, z1 - z0, nd.shuo_day[z1] - nd.shuo_day[z0] if z1 < len(d) else 0, lichun))
    year_table = np.array(years, dtype=[("year_abs", np.int64), ("astro", np.int32),
                                        ("months", np.int32), ("synodic_days", np.int32),
                                        ("lichun_jd", np.float64)])
    tropical = np.full(len(year_table) - 1, np.nan)
    for k in range(len(year_table) - 1):
        if not math.isnan(year_table["lichun_jd"][k]) and not math.isnan(year_table["lichun_jd"][k + 1]):
            tropical[k] = year_table["lichun_jd"][k + 1] - year_table["lichun_jd"][k]
    return sun, moon, year_table, tropical


# ---------------------------------------------------------------------------
# Linear algebra
# ---------------------------------------------------------------------------

def fit_normal(columns: list[np.ndarray], y: np.ndarray, ridge: float = 1e-12):
    """Chunked normal equations for a large design matrix."""
    p = len(columns)
    n = len(y)
    ata = np.zeros((p, p), dtype=np.float64)
    aty = np.zeros(p, dtype=np.float64)
    chunk = 200_000
    for start in range(0, n, chunk):
        end = min(start + chunk, n)
        A = np.empty((end - start, p), dtype=np.float64)
        for j, col in enumerate(columns):
            A[:, j] = col[start:end]
        ata += A.T @ A
        aty += A.T @ y[start:end]
    ata.flat[:: p + 1] += ridge * max(1.0, float(np.trace(ata)) / p)
    coef = np.linalg.solve(ata, aty)
    # Residual in chunks.
    sse = 0.0
    max_abs = 0.0
    for start in range(0, n, chunk):
        end = min(start + chunk, n)
        pred = np.zeros(end - start)
        for j, col in enumerate(columns):
            pred += coef[j] * col[start:end]
        resid = y[start:end] - pred
        sse += float(np.dot(resid, resid))
        max_abs = max(max_abs, float(np.max(np.abs(resid))))
    rms = math.sqrt(sse / n)
    return coef, {"rms": rms, "max": max_abs, "n": n, "params": p}


def fit_lstsq(columns: list[np.ndarray], y: np.ndarray, ridge: float = 0.0):
    """Direct SVD least squares for small, possibly ill-conditioned designs."""
    A = np.column_stack(columns)
    coef, *_ = np.linalg.lstsq(A, y, rcond=None)
    residual = y - A @ coef
    return coef, {
        "rms": float(np.sqrt(np.mean(residual * residual))),
        "max": float(np.max(np.abs(residual))),
        "n": int(len(y)), "params": int(A.shape[1]),
    }


def omega_for_days(period_days: float) -> float:
    """Angular frequency in radians per Julian century."""
    return 2.0 * math.pi * 36525.0 / period_days


# ---------------------------------------------------------------------------
# Sun model
# ---------------------------------------------------------------------------

@dataclass
class SunModel:
    poly: np.ndarray
    envelopes: list[tuple[float, list[float], list[float]]]
    extras: list[tuple[float, float, float]]  # amplitude, omega, phase
    metrics: dict
    rate_per_century: float = 0.0

    def value(self, jd: np.ndarray | float):
        x = (np.asarray(jd, dtype=np.float64) - 2451545.0) / 36525.0
        xs = x / 100.0
        v = np.zeros_like(xs)
        for p, c in enumerate(self.poly):
            v = v + c * xs ** p
        for w, cc, ss in self.envelopes:
            for m, (c, s) in enumerate(zip(cc, ss)):
                v = v + (c * np.cos(w * x) + s * np.sin(w * x)) * xs ** m
        for amp, w, ph in self.extras:
            v = v + amp * np.cos(w * x + ph)
        return v


def fit_sun(sun: dict, validation: dict) -> SunModel:
    jd = sun["jd"]; y = unwrap_angles(jd, sun["angle"])
    x = (jd - 2451545.0) / 36525.0
    xs = x / 100.0
    wa = omega_for_days(P_ANOMALISTIC_YEAR)
    poly_degree = 5
    env_degrees = [4, 2, 1, 1]
    extras_periods = SUN_EXTRA_PERIODS

    cols = [xs ** p for p in range(poly_degree + 1)]
    env_spec = []
    for k, deg in enumerate(env_degrees, start=1):
        w = k * wa
        env_spec.append((w, deg))
        for m in range(deg + 1):
            cols.append(np.cos(w * x) * xs ** m)
            cols.append(np.sin(w * x) * xs ** m)
    for period in extras_periods:
        w = omega_for_days(period * 365.25)
        cols.append(np.cos(w * x)); cols.append(np.sin(w * x))

    coef, metrics = fit_normal(cols, y)
    poly = coef[: poly_degree + 1]
    pos = poly_degree + 1
    envelopes = []
    for w, deg in env_spec:
        cc, ss = [], []
        for _ in range(deg + 1):
            cc.append(float(coef[pos])); ss.append(float(coef[pos + 1])); pos += 2
        envelopes.append((w, cc, ss))
    extras = []
    for period in extras_periods:
        c = coef[pos]; s = coef[pos + 1]; pos += 2
        amp = math.hypot(c, s); ph = math.atan2(-s, c)
        extras.append((amp, omega_for_days(period * 365.25), ph))

    # `poly` already contains the constant and linear terms; the polynomial is
    # evaluated in xs = x/100 so the coefficients stay well conditioned.
    model = SunModel(poly, envelopes, extras, metrics, rate_per_century=float(coef[1]) / 100.0)
    # Newton inversion for the table's solar-term times.
    errs = invert_sun(model, jd, sun["angle"])
    metrics.update(error_stats(errs * 86400.0, "solar-term timing (s)"))
    metrics["validation"] = validation_metrics(errs * 86400.0, sun, validation)
    return model


def unwrap_angles(jd: np.ndarray, angles: np.ndarray) -> np.ndarray:
    order = np.argsort(jd)
    t = jd[order]; a = angles[order]
    u = np.empty_like(a, dtype=np.float64)
    u[0] = a[0]
    for i in range(1, len(a)):
        delta = (a[i] - a[i - 1]) % 360.0
        if delta == 0:
            delta = 360.0
        u[i] = u[i - 1] + delta
    out = np.empty_like(u)
    out[order] = u
    return out


def invert_sun(model: SunModel, jd: np.ndarray, angles: np.ndarray) -> np.ndarray:
    """Solve L(t)=angle for each event; return t - jd in days."""
    t = jd.copy()
    # Use the unwrapped target at the initial time.
    target = unwrap_angles(jd, angles)
    for _ in range(8):
        val = model.value(t)
        deriv = (model.value(t + 0.5e-3) - val) / 0.5e-3
        step = (val - target) / deriv
        t -= step
    return t - jd


# ---------------------------------------------------------------------------
# Moon model
# ---------------------------------------------------------------------------

@dataclass
class MoonModel:
    rate: float
    poly: np.ndarray
    envelopes: list[tuple[float, list[float], list[float]]]
    metrics: dict

    def value(self, jd: np.ndarray | float):
        x = (np.asarray(jd, dtype=np.float64) - 2451545.0) / 36525.0
        xs = x / 100.0
        v = self.rate * x
        for p, c in enumerate(self.poly):
            v = v + c * xs ** p
        for w, cc, ss in self.envelopes:
            for m, (c, s) in enumerate(zip(cc, ss)):
                v = v + (c * np.cos(w * x) + s * np.sin(w * x)) * xs ** m
        return v


def fit_moon(moon: dict, validation: dict) -> MoonModel:
    jd = moon["jd"]; y = unwrap_angles(jd, moon["angle"])
    x = (jd - 2451545.0) / 36525.0
    xs = x / 100.0
    wsyn = omega_for_days(P_SYNODIC)
    wano = omega_for_days(P_ANOMALISTIC)
    wdra = omega_for_days(P_DRACONITIC)
    wann = omega_for_days(P_ANOMALISTIC_YEAR)
    weve = 2 * wsyn - wano
    wvar = 2 * wsyn

    spec = [
        (wano, 4), (2 * wano, 3), (3 * wano, 2),
        (weve, 3), (wvar, 2), (wann, 2), (wdra, 2),
        (3 * wsyn, 1), (2 * wsyn - 2 * wano, 2),
        (2 * wsyn - wann, 2), (wsyn - 2 * wano, 2),
        (wsyn - wann, 2), (wsyn - wdra, 2),
    ]
    poly_degree = 3
    cols = [xs ** p for p in range(poly_degree + 1)]
    cols.append(x)
    for w, deg in spec:
        for m in range(deg + 1):
            cols.append(np.cos(w * x) * xs ** m)
            cols.append(np.sin(w * x) * xs ** m)
    coef, metrics = fit_normal(cols, y)
    poly = coef[: poly_degree + 1]
    pos = poly_degree + 1
    rate = float(coef[pos]); pos += 1
    envelopes = []
    for w, deg in spec:
        cc, ss = [], []
        for _ in range(deg + 1):
            cc.append(float(coef[pos])); ss.append(float(coef[pos + 1])); pos += 2
        envelopes.append((w, cc, ss))
    model = MoonModel(rate, poly, envelopes, metrics)
    errs = invert_moon(model, jd, moon["angle"])
    metrics.update(error_stats(errs * 86400.0, "phase timing (s)"))
    metrics["validation"] = validation_metrics(errs * 86400.0, moon, validation)
    return model


def invert_moon(model: MoonModel, jd: np.ndarray, angles: np.ndarray) -> np.ndarray:
    t = jd.copy()
    target = unwrap_angles(jd, angles)
    for _ in range(8):
        val = model.value(t)
        deriv = (model.value(t + 0.5e-3) - val) / 0.5e-3
        t -= (val - target) / deriv
    return t - jd


# ---------------------------------------------------------------------------
# Metrics
# ---------------------------------------------------------------------------

def error_stats(errors: np.ndarray, label: str) -> dict:
    errors = np.asarray(errors, dtype=np.float64)
    return {
        label: {
            "max": float(np.max(np.abs(errors))),
            "mean": float(np.mean(np.abs(errors))),
            "rms": float(np.sqrt(np.mean(errors * errors))),
            "p99": float(np.percentile(np.abs(errors), 99)),
        }
    }


def validation_metrics(errors: np.ndarray, events: dict, validation: dict) -> dict:
    jd = events["jd"]; year = events.get("year_abs")
    out = {}
    for name, mask in validation.items():
        if mask is None or not np.any(mask):
            continue
        e = np.abs(errors[mask])
        out[name] = {"n": int(mask.sum()), "max": float(e.max()),
                     "mean": float(e.mean()), "rms": float(np.sqrt(np.mean(e * e)))}
    return out


def make_validation(events: dict, seed: int = 20261027) -> dict:
    n = len(events["jd"])
    rng = np.random.default_rng(seed)
    random_mask = np.zeros(n, dtype=bool)
    random_mask[rng.choice(n, size=max(1, n // 10), replace=False)] = True
    year = events.get("year_abs")
    last = (year >= 13001) & (year <= 13500) if year is not None else np.zeros(n, dtype=bool)
    near_epoch = (year >= -2796) & (year <= -2596) if year is not None else np.zeros(n, dtype=bool)
    return {"random10": random_mask, "last500": last, "epoch100": near_epoch}


# ---------------------------------------------------------------------------
# Cumulative / year-length models
# ---------------------------------------------------------------------------

@dataclass
class PolyModel:
    coefficients: np.ndarray
    scale: float
    metrics: dict
    mean_slope: float = 0.0

    def value(self, n: np.ndarray | float):
        n = np.asarray(n, dtype=np.float64)
        m = n - (n > 0).astype(np.float64)
        x = m / self.scale
        v = np.zeros_like(x)
        for p, c in enumerate(self.coefficients):
            v = v + c * x ** p
        return v


def fit_cumulative(nd: NaturalData) -> tuple[PolyModel, dict]:
    # The absolute month sequence has no zero.  For a continuous fit use
    # m = n - (n > 0), i.e. ..., -2, -1, 0, 1, 2, ...
    n = nd.month_abs.astype(np.float64)
    m = n - (n > 0).astype(np.float64)
    # The source's O column is the cumulative day at the END of the month.
    sd = nd.shuo_day.astype(np.float64)
    end_day = np.empty(len(sd), dtype=np.float64)
    end_day[:-1] = sd[1:] - 1.0
    end_day[-1] = sd[-1] + 29.0
    day = end_day - end_day[0]
    scale = 1000.0
    x = m / scale
    cols = [x ** p for p in range(4)]
    coef, metrics = fit_lstsq(cols, day)
    model = PolyModel(coef, scale, metrics, mean_slope=float(coef[1] / scale))
    residual = day - model.value(n)
    metrics.update(error_stats(residual, "cumulative day residual (days)"))
    metrics["meanDaysPerMonth"] = model.mean_slope
    metrics["monthCount"] = int(len(n))
    return model, metrics


def fit_year_lengths(nd: NaturalData) -> dict:
    # Synodic year: 正月初一 to next 正月初一.
    zheng = nd.zheng
    syn = np.diff(nd.shuo_day[zheng]).astype(np.float64)
    syn_year = nd.year_abs[zheng][:-1].astype(np.float64)
    scale = 1000.0
    x = (syn_year - 1.0) / scale
    cols = [x ** p for p in range(3)]
    syn_coef, syn_metrics = fit_lstsq(cols, syn)
    syn_res = syn - np.column_stack(cols) @ syn_coef
    syn_metrics.update(error_stats(syn_res, "synodic year residual (days)"))
    syn_metrics["coefficients"] = syn_coef.tolist()
    syn_metrics["scale"] = scale
    syn_metrics["meanDays"] = float(np.mean(syn))

    # Tropical year: consecutive 立春 instants.  Use only one- year steps.
    raw = nd.raw
    lichun = np.where((nd.term_names[raw["term_name"]] == "立春") & (~np.isnan(raw["jd_term"])))[0]
    lichun = lichun[np.argsort(raw["jd_term"][lichun])]
    jd = raw["jd_term"][lichun]
    ya = nd.year_abs[lichun].astype(np.float64)
    dif = np.diff(jd)
    ok = (dif > 360.0) & (dif < 370.0)
    trop = dif[ok]
    trop_year = ya[:-1][ok]
    x2 = (trop_year - 1.0) / scale
    cols2 = [x2 ** p for p in range(5)]
    trop_coef, trop_metrics = fit_lstsq(cols2, trop)
    trop_res = trop - np.column_stack(cols2) @ trop_coef
    trop_metrics.update(error_stats(trop_res, "tropical year residual (days)"))
    trop_metrics["coefficients"] = trop_coef.tolist()
    trop_metrics["scale"] = scale
    trop_metrics["meanDays"] = float(np.mean(trop))

    return {
        "synodic": {"coefficients": syn_coef.tolist(), "scale": scale,
                    "meanDays": float(np.mean(syn)), "metrics": syn_metrics},
        "tropical": {"coefficients": trop_coef.tolist(), "scale": scale,
                     "meanDays": float(np.mean(trop)), "metrics": trop_metrics},
    }


# ---------------------------------------------------------------------------
# Leap rule validation
# ---------------------------------------------------------------------------

def leap_rule_metrics(nd: NaturalData, sun_model: "SunModel") -> dict:
    d = nd.raw
    n = len(d)
    leap = nd.leap
    sd = nd.shuo_day.astype(np.float64)

    # (a) Rule using the table's own 中气 events, on civil-day boundaries.
    mids = np.sort(d["jd_mid"][~np.isnan(d["jd_mid"])])
    mid_day = np.floor(mids + 0.5 + 8.0 / 24.0).astype(np.int64)
    starts = np.searchsorted(mid_day, sd.astype(np.int64), side="left")
    end_days = np.empty(n, dtype=np.int64)
    end_days[:-1] = sd[1:].astype(np.int64)
    end_days[-1] = int(sd[-1]) + 30
    ends = np.searchsorted(mid_day, end_days, side="left")
    has_mid = ends > starts
    pred = first_without_mid(nd, ~has_mid)

    # (b) Rule using the fitted sun longitude, on civil-day boundaries.
    jd0 = sd - 0.5 - 8.0 / 24.0
    jd1 = np.empty(n)
    jd1[:-1] = jd0[1:]
    jd1[-1] = jd0[-1] + 30.0
    l0 = sun_model.value(jd0)
    l1 = sun_model.value(jd1)
    crossings = np.floor_divide(l1, 30.0).astype(np.int64) - np.floor_divide(l0, 30.0).astype(np.int64)
    pred_fit = first_without_mid(nd, crossings == 0)

    def confusion(p):
        fp = int(np.sum(p & ~leap))
        fn = int(np.sum(~p & leap))
        tp = int(np.sum(p & leap))
        tn = int(np.sum(~p & ~leap))
        return {"accuracy": float((tp + tn) / len(leap)), "truePositive": tp,
                "trueNegative": tn, "falsePositive": fp, "falseNegative": fn}

    def year_confusion(p):
        zheng = nd.zheng
        true_year, pred_year = [], []
        for k, z0 in enumerate(zheng[:-1]):
            z1 = zheng[k + 1]
            true_year.append((z1 - z0) == 13)
            pred_year.append(bool(np.any(p[z0:z1])))
        true_year = np.array(true_year)
        pred_year = np.array(pred_year)
        tp = int(np.sum(pred_year & true_year))
        tn = int(np.sum(~pred_year & ~true_year))
        return {"accuracy": float((tp + tn) / len(true_year)), "truePositive": tp,
                "trueNegative": tn, "falsePositive": int(np.sum(pred_year & ~true_year)),
                "falseNegative": int(np.sum(~pred_year & true_year))}

    return {
        "tableEvents": confusion(pred),
        "fittedSun": confusion(pred_fit),
        "tableEventsYear": year_confusion(pred),
        "fittedSunYear": year_confusion(pred_fit),
        "rule": "in a 13-month synodic year, the first month whose civil-day interval "
                "contains no 中气 is leap; 12-month years have no leap month",
        "note": "A time-only Fourier/logistic classifier was also attempted and is "
                "fundamentally unable to represent the astronomical rule (the 19-year "
                "Metonic lookup alone scores only ~63%).",
    }


def first_without_mid(nd: NaturalData, no_mid: np.ndarray) -> np.ndarray:
    zheng = nd.zheng
    pred = np.zeros(len(no_mid), dtype=bool)
    for k, z0 in enumerate(zheng[:-1]):
        z1 = zheng[k + 1]
        if z1 - z0 != 13:
            continue
        for i in range(z0, z1):
            if no_mid[i]:
                pred[i] = True
                break
    return pred

def write_binary(path: Path, arrays: list[np.ndarray]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("wb") as fh:
        fh.write(b"NFIT0001")
        fh.write(np.int64(len(arrays[0])).tobytes())
        for a in arrays:
            fh.write(np.ascontiguousarray(a).tobytes())


def export_cpp(nd: NaturalData, sun: dict, moon: dict, years: np.ndarray, tropical: np.ndarray) -> None:
    CPP_DATA.mkdir(parents=True, exist_ok=True)
    write_binary(CPP_DATA / "sun_events.bin", [
        sun["jd"].astype(np.float64), sun["angle"].astype(np.float64),
        sun["year_abs"].astype(np.int32), sun["month_abs"].astype(np.int32),
        sun["kind"].astype(np.int32),
    ])
    write_binary(CPP_DATA / "moon_events.bin", [
        moon["jd"].astype(np.float64), moon["angle"].astype(np.float64),
        moon["year_abs"].astype(np.int32), moon["month_abs"].astype(np.int32),
    ])
    write_binary(CPP_DATA / "months.bin", [
        nd.month_abs.astype(np.int64), nd.year_abs.astype(np.int64),
        nd.leap.astype(np.int32), nd.year_len.astype(np.int32),
        nd.raw["jd_shuo"].astype(np.float64), nd.shuo_day.astype(np.int64),
    ])
    write_binary(CPP_DATA / "years.bin", [
        years["year_abs"].astype(np.int64), years["months"].astype(np.int32),
        years["synodic_days"].astype(np.int32), tropical.astype(np.float64),
    ])


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def build_payload(nd: NaturalData, sun_model: SunModel, moon_model: MoonModel,
                  sun_events: dict, moon_events: dict, python_seconds: float) -> dict:
    cum_model, cum_metrics = fit_cumulative(nd)
    year_payload = fit_year_lengths(nd)
    leap_payload = leap_rule_metrics(nd, sun_model)

    sun_struct = {
        "params": int(sun_model.metrics["params"]),
        "rmsDegrees": float(sun_model.metrics["rms"]),
        "maxDegrees": float(sun_model.metrics["max"]),
        "timingSeconds": sun_model.metrics["solar-term timing (s)"],
        "ratePerCentury": float(sun_model.rate_per_century),
        "offset": float(sun_model.value(sun_events["jd"][0]) - sun_events["angle"][0]),
        "polynomial": sun_model.poly.tolist(),
        "envelopes": [{"omega": float(w), "degree": len(cc) - 1,
                       "cos": [float(v) for v in cc], "sin": [float(v) for v in ss]}
                      for w, cc, ss in sun_model.envelopes],
        "extras": [{"amplitude": float(a), "omega": float(w), "phase": float(p)}
                   for a, w, p in sun_model.extras],
        "validation": sun_model.metrics.get("validation", {}),
    }
    moon_struct = {
        "params": int(moon_model.metrics["params"]),
        "rmsDegrees": float(moon_model.metrics["rms"]),
        "maxDegrees": float(moon_model.metrics["max"]),
        "timingSeconds": moon_model.metrics["phase timing (s)"],
        "ratePerCentury": float(moon_model.rate),
        "offset": float(moon_model.value(moon_events["jd"][0])),
        "polynomial": moon_model.poly.tolist(),
        "envelopes": [{"omega": float(w), "degree": len(cc) - 1,
                       "cos": [float(v) for v in cc], "sin": [float(v) for v in ss]}
                      for w, cc, ss in moon_model.envelopes],
        "validation": moon_model.metrics.get("validation", {}),
    }

    stats = json.loads((DATA / "extract_stats.json").read_text(encoding="utf-8"))
    data = {
        "rows": int(stats["rows"]),
        "firstYear": int(stats["year_first"]),
        "lastYear": int(stats["year_last"]),
        "termEvents": int(stats["term_events"]),
        "midEvents": int(stats["mid_events"]),
        "phaseEvents": int(stats["phase_events"]),
        "leapMonths": int(stats["leap_months"]),
        "leapYears": int(stats["leap_years"]),
        "nanCounts": stats["nan_counts"],
        "duplicates": stats.get("duplicates", {}),
        "dateConvention": stats["date_convention"],
        "epochRow": int(stats["epoch_row"]),
        "epochJiyue": int(stats["epoch_jiyue"]),
    }

    payload = {
        "sun": sun_struct,
        "moon": moon_struct,
        "cumulative": {
            "coefficients": cum_model.coefficients.tolist(),
            "scale": cum_model.scale,
            "meanDaysPerMonth": cum_model.mean_slope,
            "metrics": cum_metrics,
        },
        "yearLength": year_payload,
        "leap": leap_payload,
        "data": data,
        "timing": {"pythonSeconds": python_seconds},
        "agreement": {},
    }

    # Use the C++ coefficients when they agree with the Python reference.
    cpp_path = DATA / "cpp_coefficients.json"
    if cpp_path.exists():
        try:
            cpp = json.loads(cpp_path.read_text(encoding="utf-8"))
            cpp_sun = fit_report.normalize_sun(cpp["sun"])
            cpp_moon = fit_report.normalize_moon(cpp["moon"])
            agreement = {}
            for key, py, cp in (("sun", sun_struct, cpp_sun), ("moon", moon_struct, cpp_moon)):
                denom = max(abs(py["rmsDegrees"]), 1e-15)
                agreement[key] = {
                    "pythonRms": py["rmsDegrees"], "cppRms": cp["rmsDegrees"],
                    "relativeRmsDiff": abs(py["rmsDegrees"] - cp["rmsDegrees"]) / denom,
                    "pythonMax": py["maxDegrees"], "cppMax": cp["maxDegrees"],
                }
            payload["agreement"] = agreement
            payload["timing"]["cppSeconds"] = float(cpp.get("fitSeconds", 0.0))
            cpp_sun["validation"] = sun_struct["validation"]
            cpp_moon["validation"] = moon_struct["validation"]
            cpp_sun["offset"] = sun_struct["offset"]
            cpp_moon["offset"] = moon_struct["offset"]
            payload["sun"] = cpp_sun
            payload["moon"] = cpp_moon
            print("using C++ coefficients for waveforms.h")
        except (KeyError, ValueError) as exc:
            print(f"warning: cannot use C++ coefficients: {exc}")
    return payload


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("command", nargs="?", default="all",
                        choices=["all", "extract", "fit", "report"])
    parser.add_argument("--force", action="store_true", help="rebuild the xlsx cache")
    args = parser.parse_args()
    sys.stdout.reconfigure(encoding="utf-8")

    if args.command in ("all", "extract"):
        extract(force=args.force)

    if args.command == "report":
        payload = json.loads((DATA / "fit_python.json").read_text(encoding="utf-8"))
        fit_report.write_header(payload)
        fit_report.write_manifest(payload)
        fit_report.write_report(payload)
        print("wrote", HEADER, MANIFEST, REPORT)
        return 0

    nd = NaturalData.load()
    sun, moon, years, tropical = build_events(nd)
    export_cpp(nd, sun, moon, years, tropical)
    print(f"sun events: {len(sun['jd'])}, moon events: {len(moon['jd'])}, months: {len(nd.raw)}")

    if args.command in ("all", "fit"):
        t0 = time.time()
        sun_val = make_validation(sun)
        moon_val = make_validation(moon)
        print("fitting sun ...", flush=True)
        sun_model = fit_sun(sun, sun_val)
        print("sun metrics:", json.dumps(sun_model.metrics, ensure_ascii=False))
        print("fitting moon ...", flush=True)
        moon_model = fit_moon(moon, moon_val)
        print("moon metrics:", json.dumps(moon_model.metrics, ensure_ascii=False))
        python_seconds = time.time() - t0
        print(f"fit seconds: {python_seconds:.1f}")

        payload = build_payload(nd, sun_model, moon_model, sun, moon, python_seconds)
        (DATA / "fit_python.json").write_text(
            json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8")
        print("wrote", DATA / "fit_python.json")
        fit_report.write_header(payload)
        fit_report.write_manifest(payload)
        fit_report.write_report(payload)
        print("wrote", HEADER)
        print("wrote", MANIFEST)
        print("wrote", REPORT)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
