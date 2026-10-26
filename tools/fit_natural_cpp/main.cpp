// Independent C++17 fitter for the natural-calendar Waveforms.
//
// Reads the binary event tables written by tools/fit_natural.py and solves the
// same compact least-squares models with a chunked normal-equation + Cholesky
// solver.  It does not depend on Eigen or any other third-party library.
//
// Build (Windows, MSVC):
//   cmake -S tools/fit_natural_cpp -B build-natural-fit -G Ninja
//   cmake --build build-natural-fit
// Run:
//   build-natural-fit/fit_natural_cpp temporal/natural/data temporal/natural/data
//
// Output:
//   <out>/cpp_coefficients.json   coefficients + metrics
//   <out>/cpp_metrics.json        metrics only

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{

constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr double kJ2000 = 2451545.0;
constexpr double kDaysPerCentury = 36525.0;

constexpr double kPAnomalisticYear = 365.259636;
constexpr double kPSynodic = 29.530588;
constexpr double kPAnomalistic = 27.554550;
constexpr double kPDraconitic = 27.212221;
constexpr double kPTropical = 365.242190;

const double kSunExtraPeriods[] = {
    18.61, 11.862, 1.0921, 1.5987, 0.7994, 29.457, 0.6152, 2.135, 0.317,
    8.1, 3.983, 2.47, 1.2028, 1.1408, 1.068, 0.546, 0.533, 0.572, 0.333, 0.444,
};

double omegaDays(double period_days) { return 2.0 * kPi * kDaysPerCentury / period_days; }

struct Vec
{
    std::vector<double> data;
    double operator[](std::size_t i) const { return data[i]; }
    double &operator[](std::size_t i) { return data[i]; }
    std::size_t size() const { return data.size(); }
};

template <typename T>
void readRaw(std::ifstream &in, std::vector<T> &out, std::size_t count)
{
    out.resize(count);
    in.read(reinterpret_cast<char *>(out.data()), static_cast<std::streamsize>(count * sizeof(T)));
    if (!in)
    {
        throw std::runtime_error("short read");
    }
}

struct BinaryTable
{
    std::uint64_t count = 0;
    std::vector<double> a; // first double array
    std::vector<double> b; // second double array
    std::vector<std::int32_t> i0;
    std::vector<std::int32_t> i1;
    std::vector<std::int32_t> i2;
};

BinaryTable readTable(const std::string &path, int double_count, int int_count)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        throw std::runtime_error("cannot open " + path);
    }
    char magic[8] = {};
    in.read(magic, 8);
    std::uint64_t n = 0;
    in.read(reinterpret_cast<char *>(&n), sizeof(n));
    if (!in || std::string(magic, 8) != "NFIT0001")
    {
        throw std::runtime_error("bad magic in " + path);
    }
    BinaryTable table;
    table.count = n;
    if (double_count >= 1)
    {
        readRaw(in, table.a, n);
    }
    if (double_count >= 2)
    {
        readRaw(in, table.b, n);
    }
    if (int_count >= 1)
    {
        readRaw(in, table.i0, n);
    }
    if (int_count >= 2)
    {
        readRaw(in, table.i1, n);
    }
    if (int_count >= 3)
    {
        readRaw(in, table.i2, n);
    }
    return table;
}

// ---------------------------------------------------------------------------
// Dense symmetric solver
// ---------------------------------------------------------------------------

std::vector<double> choleskySolve(std::vector<double> a, std::vector<double> b, std::size_t p)
{
    // a is row-major p x p, positive definite; b is length p.
    for (std::size_t i = 0; i < p; ++i)
    {
        for (std::size_t j = 0; j <= i; ++j)
        {
            double sum = a[i * p + j];
            for (std::size_t k = 0; k < j; ++k)
            {
                sum -= a[i * p + k] * a[j * p + k];
            }
            if (i == j)
            {
                if (!(sum > 0.0))
                {
                    throw std::runtime_error("normal matrix is not positive definite");
                }
                a[i * p + j] = std::sqrt(sum);
            }
            else
            {
                a[i * p + j] = sum / a[j * p + j];
            }
        }
    }
    std::vector<double> y(p);
    for (std::size_t i = 0; i < p; ++i)
    {
        double sum = b[i];
        for (std::size_t k = 0; k < i; ++k)
        {
            sum -= a[i * p + k] * y[k];
        }
        y[i] = sum / a[i * p + i];
    }
    std::vector<double> x(p);
    for (std::size_t i = p; i-- > 0;)
    {
        double sum = y[i];
        for (std::size_t k = i + 1; k < p; ++k)
        {
            sum -= a[k * p + i] * x[k];
        }
        x[i] = sum / a[i * p + i];
    }
    return x;
}

struct Metrics
{
    double rms = 0.0;
    double max = 0.0;
    double mean = 0.0;
    std::size_t n = 0;
};

Metrics residualMetrics(const std::vector<double> &residual)
{
    Metrics m;
    m.n = residual.size();
    if (m.n == 0)
    {
        return m;
    }
    double sse = 0.0;
    double sum = 0.0;
    for (double r : residual)
    {
        sse += r * r;
        sum += std::abs(r);
        m.max = std::max(m.max, std::abs(r));
    }
    m.rms = std::sqrt(sse / static_cast<double>(m.n));
    m.mean = sum / static_cast<double>(m.n);
    return m;
}

// ---------------------------------------------------------------------------
// Sun model
// ---------------------------------------------------------------------------

constexpr int kSunPoly = 6;
constexpr int kSunEnvCount = 4;
constexpr int kSunEnvDegrees[kSunEnvCount] = {4, 2, 1, 1};
constexpr int kSunExtraCount = static_cast<int>(sizeof(kSunExtraPeriods) / sizeof(double));

int sunParamCount()
{
    int p = kSunPoly;
    for (int k = 0; k < kSunEnvCount; ++k)
    {
        p += 2 * (kSunEnvDegrees[k] + 1);
    }
    p += 2 * kSunExtraCount;
    return p;
}

struct SunModel
{
    std::vector<double> coef;
    std::vector<double> envOmega;
    std::vector<int> envDegree;
    std::vector<double> extraOmega;
    std::vector<double> extraAmp;
    std::vector<double> extraPhase;

    double value(double jd) const
    {
        const double x = (jd - kJ2000) / kDaysPerCentury;
        const double xs = x / 100.0;
        double v = 0.0;
        double power = 1.0;
        for (int p = 0; p < kSunPoly; ++p)
        {
            v += coef[p] * power;
            power *= xs;
        }
        int pos = kSunPoly;
        for (int k = 0; k < kSunEnvCount; ++k)
        {
            const int degree = kSunEnvDegrees[k];
            const double w = envOmega[k];
            const double c = std::cos(w * x);
            const double s = std::sin(w * x);
            double pow = 1.0;
            for (int m = 0; m <= degree; ++m)
            {
                v += (coef[pos] * c + coef[pos + 1] * s) * pow;
                pos += 2;
                pow *= xs;
            }
        }
        for (int k = 0; k < kSunExtraCount; ++k)
        {
            v += coef[pos] * std::cos(extraOmega[k] * x) + coef[pos + 1] * std::sin(extraOmega[k] * x);
            pos += 2;
        }
        return v;
    }
};

std::vector<double> sunRow(double jd)
{
    const double x = (jd - kJ2000) / kDaysPerCentury;
    const double xs = x / 100.0;
    std::vector<double> row;
    row.reserve(sunParamCount());
    double power = 1.0;
    for (int p = 0; p < kSunPoly; ++p)
    {
        row.push_back(power);
        power *= xs;
    }
    for (int k = 0; k < kSunEnvCount; ++k)
    {
        const double w = (k + 1) * omegaDays(kPAnomalisticYear);
        const double c = std::cos(w * x);
        const double s = std::sin(w * x);
        double pow = 1.0;
        for (int m = 0; m <= kSunEnvDegrees[k]; ++m)
        {
            row.push_back(c * pow);
            row.push_back(s * pow);
            pow *= xs;
        }
    }
    for (int k = 0; k < kSunExtraCount; ++k)
    {
        const double w = omegaDays(kSunExtraPeriods[k] * 365.25);
        row.push_back(std::cos(w * x));
        row.push_back(std::sin(w * x));
    }
    return row;
}

std::vector<double> unwrapAngles(const std::vector<double> &jd, const std::vector<double> &angle)
{
    std::vector<std::size_t> order(jd.size());
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        order[i] = i;
    }
    std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) { return jd[a] < jd[b]; });
    std::vector<double> out(jd.size());
    double current = 0.0;
    for (std::size_t k = 0; k < order.size(); ++k)
    {
        const std::size_t i = order[k];
        if (k == 0)
        {
            current = angle[i];
        }
        else
        {
            double delta = std::fmod(angle[i] - out[order[k - 1]], 360.0);
            if (delta < 0.0)
            {
                delta += 360.0;
            }
            if (delta == 0.0)
            {
                delta = 360.0;
            }
            current = out[order[k - 1]] + delta;
        }
        out[i] = current;
    }
    return out;
}

// Solve y ~ row(jd) * coef by chunked normal equations.
template <typename Row>
std::vector<double> fitLeastSquares(const std::vector<double> &jd, const std::vector<double> &y,
                                    std::size_t p, Row row, std::vector<double> &residual)
{
    std::vector<double> ata(p * p, 0.0);
    std::vector<double> aty(p, 0.0);
    std::vector<double> r(p);
    const std::size_t chunk = 50000;
    for (std::size_t start = 0; start < jd.size(); start += chunk)
    {
        const std::size_t end = std::min(start + chunk, jd.size());
        for (std::size_t i = start; i < end; ++i)
        {
            r = row(jd[i]);
            for (std::size_t a = 0; a < p; ++a)
            {
                aty[a] += r[a] * y[i];
                for (std::size_t b = 0; b <= a; ++b)
                {
                    ata[a * p + b] += r[a] * r[b];
                }
            }
        }
    }
    for (std::size_t a = 0; a < p; ++a)
    {
        for (std::size_t b = 0; b < a; ++b)
        {
            ata[b * p + a] = ata[a * p + b];
        }
    }
    double trace = 0.0;
    for (std::size_t a = 0; a < p; ++a)
    {
        trace += ata[a * p + a];
    }
    const double ridge = 1e-12 * std::max(1.0, trace / static_cast<double>(p));
    for (std::size_t a = 0; a < p; ++a)
    {
        ata[a * p + a] += ridge;
    }
    const std::vector<double> coef = choleskySolve(ata, aty, p);

    residual.assign(jd.size(), 0.0);
    for (std::size_t start = 0; start < jd.size(); start += chunk)
    {
        const std::size_t end = std::min(start + chunk, jd.size());
        for (std::size_t i = start; i < end; ++i)
        {
            r = row(jd[i]);
            double pred = 0.0;
            for (std::size_t a = 0; a < p; ++a)
            {
                pred += r[a] * coef[a];
            }
            residual[i] = y[i] - pred;
        }
    }
    return coef;
}

std::vector<double> invertModel(const std::vector<double> &jd, const std::vector<double> &angle,
                                const std::function<double(double)> &value)
{
    const std::vector<double> target = unwrapAngles(jd, angle);
    std::vector<double> errors(jd.size());
    for (std::size_t i = 0; i < jd.size(); ++i)
    {
        double t = jd[i];
        for (int it = 0; it < 8; ++it)
        {
            const double v = value(t);
            const double h = 0.5e-3;
            const double d = (value(t + h) - v) / h;
            t -= (v - target[i]) / d;
        }
        errors[i] = (t - jd[i]) * 86400.0;
    }
    return errors;
}

// ---------------------------------------------------------------------------
// Moon model
// ---------------------------------------------------------------------------

struct MoonSpec
{
    double omega;
    int degree;
};

std::vector<MoonSpec> moonSpecs()
{
    const double wsyn = omegaDays(kPSynodic);
    const double wano = omegaDays(kPAnomalistic);
    const double wdra = omegaDays(kPDraconitic);
    const double wann = omegaDays(kPAnomalisticYear);
    const double weve = 2.0 * wsyn - wano;
    const double wvar = 2.0 * wsyn;
    return {
        {wano, 4}, {2.0 * wano, 3}, {3.0 * wano, 2}, {weve, 3}, {wvar, 2},
        {wann, 2}, {wdra, 2}, {3.0 * wsyn, 1}, {2.0 * wsyn - 2.0 * wano, 2},
        {2.0 * wsyn - wann, 2}, {wsyn - 2.0 * wano, 2}, {wsyn - wann, 2},
        {wsyn - wdra, 2},
    };
}

int moonParamCount()
{
    int p = 5; // xs^0..xs^3 + x
    for (const MoonSpec &s : moonSpecs())
    {
        p += 2 * (s.degree + 1);
    }
    return p;
}

std::vector<double> moonRow(double jd)
{
    const double x = (jd - kJ2000) / kDaysPerCentury;
    const double xs = x / 100.0;
    std::vector<double> row;
    row.reserve(moonParamCount());
    double power = 1.0;
    for (int p = 0; p < 4; ++p)
    {
        row.push_back(power);
        power *= xs;
    }
    row.push_back(x);
    for (const MoonSpec &s : moonSpecs())
    {
        const double c = std::cos(s.omega * x);
        const double sn = std::sin(s.omega * x);
        double pow = 1.0;
        for (int m = 0; m <= s.degree; ++m)
        {
            row.push_back(c * pow);
            row.push_back(sn * pow);
            pow *= xs;
        }
    }
    return row;
}

struct MoonModel
{
    std::vector<double> coef;
    std::vector<MoonSpec> specs;

    double value(double jd) const
    {
        const double x = (jd - kJ2000) / kDaysPerCentury;
        const double xs = x / 100.0;
        double v = coef[4] * x;
        double power = 1.0;
        for (int p = 0; p < 4; ++p)
        {
            v += coef[p] * power;
            power *= xs;
        }
        int pos = 5;
        for (const MoonSpec &s : specs)
        {
            const double c = std::cos(s.omega * x);
            const double sn = std::sin(s.omega * x);
            double pow = 1.0;
            for (int m = 0; m <= s.degree; ++m)
            {
                v += (coef[pos] * c + coef[pos + 1] * sn) * pow;
                pos += 2;
                pow *= xs;
            }
        }
        return v;
    }
};

// ---------------------------------------------------------------------------
// JSON output
// ---------------------------------------------------------------------------

void writeMetricsJson(std::ostream &out, const Metrics &m)
{
    out << "{\"n\":" << m.n << ",\"rms\":" << m.rms << ",\"max\":" << m.max << ",\"mean\":" << m.mean << "}";
}

void writeArray(std::ostream &out, const std::vector<double> &v)
{
    out << '[';
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        if (i)
        {
            out << ',';
        }
        out << v[i];
    }
    out << ']';
}

} // namespace

int main(int argc, char **argv)
{
    try
    {
        const auto started = std::chrono::steady_clock::now();
        std::string dataDir = "temporal/natural/data";
        std::string outDir = dataDir;
        if (argc > 1)
        {
            dataDir = argv[1];
        }
        if (argc > 2)
        {
            outDir = argv[2];
        }
        std::cout << "data: " << dataDir << "\nout: " << outDir << std::endl;

        const BinaryTable sun = readTable(dataDir + "/cpp/sun_events.bin", 2, 3);
        const BinaryTable moon = readTable(dataDir + "/cpp/moon_events.bin", 2, 2);
        std::cout << "sun events: " << sun.count << ", moon events: " << moon.count << std::endl;

        const std::vector<double> sunTarget = unwrapAngles(sun.a, sun.b);
        const std::size_t sunP = static_cast<std::size_t>(sunParamCount());
        std::vector<double> sunResidual;
        const std::vector<double> sunCoef = fitLeastSquares(sun.a, sunTarget, sunP, sunRow, sunResidual);
        SunModel sunModel;
        sunModel.coef = sunCoef;
        for (int k = 0; k < kSunEnvCount; ++k)
        {
            sunModel.envOmega.push_back((k + 1) * omegaDays(kPAnomalisticYear));
            sunModel.envDegree.push_back(kSunEnvDegrees[k]);
        }
        for (int k = 0; k < kSunExtraCount; ++k)
        {
            sunModel.extraOmega.push_back(omegaDays(kSunExtraPeriods[k] * 365.25));
        }
        {
            int pos = kSunPoly;
            for (int k = 0; k < kSunEnvCount; ++k)
            {
                pos += 2 * (kSunEnvDegrees[k] + 1);
            }
            for (int k = 0; k < kSunExtraCount; ++k)
            {
                const double c = sunCoef[pos];
                const double s = sunCoef[pos + 1];
                sunModel.extraAmp.push_back(std::hypot(c, s));
                sunModel.extraPhase.push_back(std::atan2(-s, c));
                pos += 2;
            }
        }
        const Metrics sunFit = residualMetrics(sunResidual);
        const std::vector<double> sunTiming = invertModel(
            sun.a, sun.b, [&](double t) { return sunModel.value(t); });
        const Metrics sunTimingMetrics = residualMetrics(sunTiming);
        // Reuse residualMetrics for absolute values: it is symmetric in sign.
        std::cout << "sun rms(deg)=" << sunFit.rms << " max(deg)=" << sunFit.max
                  << " timing rms(s)=" << sunTimingMetrics.rms << " max(s)=" << sunTimingMetrics.max << std::endl;

        const std::vector<double> moonTarget = unwrapAngles(moon.a, moon.b);
        const std::size_t moonP = static_cast<std::size_t>(moonParamCount());
        std::vector<double> moonResidual;
        const std::vector<double> moonCoef = fitLeastSquares(moon.a, moonTarget, moonP, moonRow, moonResidual);
        MoonModel moonModel;
        moonModel.coef = moonCoef;
        moonModel.specs = moonSpecs();
        const Metrics moonFit = residualMetrics(moonResidual);
        const std::vector<double> moonTiming = invertModel(
            moon.a, moon.b, [&](double t) { return moonModel.value(t); });
        const Metrics moonTimingMetrics = residualMetrics(moonTiming);
        std::cout << "moon rms(deg)=" << moonFit.rms << " max(deg)=" << moonFit.max
                  << " timing rms(s)=" << moonTimingMetrics.rms << " max(s)=" << moonTimingMetrics.max << std::endl;

        std::ostringstream json;
        json << std::setprecision(17);
        const double fitSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
        json << "{\n  \"source\": \"寿星数据整录——原版.xlsx\",\n";
        json << "  \"fitSeconds\": " << fitSeconds << ",\n";
        json << "  \"timeScale\": \"UTC+8; Julian before 1582-10-15, Gregorian after\",\n";
        json << "  \"normalization\": \"x=(JD-2451545)/36525\",\n";
        json << "  \"sun\": {\n";
        json << "    \"params\": " << sunP << ",\n";
        json << "    \"rmsDegrees\": " << sunFit.rms << ",\n";
        json << "    \"maxDegrees\": " << sunFit.max << ",\n";
        json << "    \"timingSeconds\": ";
        writeMetricsJson(json, sunTimingMetrics);
        json << ",\n    \"polynomial\": ";
        writeArray(json, std::vector<double>(sunCoef.begin(), sunCoef.begin() + kSunPoly));
        json << ",\n    \"envelopes\": [";
        {
            int pos = kSunPoly;
            for (int k = 0; k < kSunEnvCount; ++k)
            {
                if (k)
                {
                    json << ',';
                }
                json << "{\"omega\":" << sunModel.envOmega[k] << ",\"degree\":" << kSunEnvDegrees[k]
                     << ",\"cos\":[";
                for (int m = 0; m <= kSunEnvDegrees[k]; ++m)
                {
                    if (m)
                    {
                        json << ',';
                    }
                    json << sunCoef[pos + 2 * m];
                }
                json << "],\"sin\":[";
                for (int m = 0; m <= kSunEnvDegrees[k]; ++m)
                {
                    if (m)
                    {
                        json << ',';
                    }
                    json << sunCoef[pos + 2 * m + 1];
                }
                json << "]}";
                pos += 2 * (kSunEnvDegrees[k] + 1);
            }
        }
        json << "],\n    \"extras\": [";
        for (int k = 0; k < kSunExtraCount; ++k)
        {
            if (k)
            {
                json << ',';
            }
            json << "{\"amplitude\":" << sunModel.extraAmp[k] << ",\"omega\":" << sunModel.extraOmega[k]
                 << ",\"phase\":" << sunModel.extraPhase[k] << "}";
        }
        json << "]\n  },\n";

        json << "  \"moon\": {\n";
        json << "    \"params\": " << moonP << ",\n";
        json << "    \"rmsDegrees\": " << moonFit.rms << ",\n";
        json << "    \"maxDegrees\": " << moonFit.max << ",\n";
        json << "    \"timingSeconds\": ";
        writeMetricsJson(json, moonTimingMetrics);
        json << ",\n    \"ratePerCentury\": " << moonCoef[4] << ",\n";
        json << "    \"polynomial\": ";
        writeArray(json, std::vector<double>(moonCoef.begin(), moonCoef.begin() + 4));
        json << ",\n    \"envelopes\": [";
        {
            int pos = 5;
            const std::vector<MoonSpec> specs = moonSpecs();
            for (std::size_t k = 0; k < specs.size(); ++k)
            {
                if (k)
                {
                    json << ',';
                }
                json << "{\"omega\":" << specs[k].omega << ",\"degree\":" << specs[k].degree << ",\"cos\":[";
                for (int m = 0; m <= specs[k].degree; ++m)
                {
                    if (m)
                    {
                        json << ',';
                    }
                    json << moonCoef[pos + 2 * m];
                }
                json << "],\"sin\":[";
                for (int m = 0; m <= specs[k].degree; ++m)
                {
                    if (m)
                    {
                        json << ',';
                    }
                    json << moonCoef[pos + 2 * m + 1];
                }
                json << "]}";
                pos += 2 * (specs[k].degree + 1);
            }
        }
        json << "]\n  }\n}\n";

        std::ofstream out(outDir + "/cpp_coefficients.json", std::ios::binary);
        if (!out)
        {
            throw std::runtime_error("cannot write " + outDir + "/cpp_coefficients.json");
        }
        out << json.str();
        std::ofstream metricsOut(outDir + "/cpp_metrics.json", std::ios::binary);
        metricsOut << json.str();
        std::cout << "wrote " << outDir << "/cpp_coefficients.json" << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
}
