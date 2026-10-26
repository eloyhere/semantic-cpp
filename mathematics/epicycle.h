#pragma once

#include <cmath>
#include <ostream>
#include <stdexcept>
#include <type_traits>

#include "numerical.h"

namespace mathematics
{

template <typename Number>
class Epicycle;

template <typename Number>
class Waveform;

// Adding two epicycles superposes them, which is only representable as another
// epicycle when the omegas agree, so the general result is a Waveform.
template <typename Number>
Waveform<Number> operator+(const Epicycle<Number> &left, const Epicycle<Number> &right);

template <typename Number>
Waveform<Number> operator+(const Waveform<Number> &left, const Epicycle<Number> &right);

template <typename Number>
Waveform<Number> operator+(const Epicycle<Number> &left, const Waveform<Number> &right);

// One rotating term of a Waveform: z(t) = amplitude * e^(i * (omega * t + phase)).
// amplitude is kept non-negative: a negative constructor argument is folded
// into phase by adding Pi, which leaves value(t) unchanged.
template <typename Number>
class Epicycle
{
    static_assert(std::is_floating_point<Number>::value,
                  "mathematics::Epicycle requires a floating-point Number type");

  public:
    Epicycle() : amplitude(Zero<Number>), omega(Zero<Number>), phase(Zero<Number>) {}

    Epicycle(Number amplitudeValue, Number omegaValue, Number phaseValue)
        : amplitude(amplitudeValue), omega(omegaValue), phase(normalizeArgument(phaseValue))
    {
        normalizeAmplitude();
    }

    Epicycle(const Epicycle &other) : amplitude(other.amplitude), omega(other.omega), phase(other.phase) {}

    Epicycle(Epicycle &&other) noexcept : amplitude(other.amplitude), omega(other.omega), phase(other.phase)
    {
        other.amplitude = Zero<Number>;
        other.omega = Zero<Number>;
        other.phase = Zero<Number>;
    }

    Epicycle &operator=(const Epicycle &other)
    {
        if (this != &other)
        {
            amplitude = other.amplitude;
            omega = other.omega;
            phase = other.phase;
        }
        return *this;
    }

    Epicycle &operator=(Epicycle &&other) noexcept
    {
        if (this != &other)
        {
            amplitude = other.amplitude;
            omega = other.omega;
            phase = other.phase;
            other.amplitude = Zero<Number>;
            other.omega = Zero<Number>;
            other.phase = Zero<Number>;
        }
        return *this;
    }

    ~Epicycle() = default;

    Number getAmplitude() const { return amplitude; }
    Number getOmega() const { return omega; }
    Number getPhase() const { return phase; }

    void setAmplitude(Number value)
    {
        amplitude = value;
        normalizeAmplitude();
    }

    void setOmega(Number value) { omega = value; }

    void setPhase(Number value) { phase = normalizeArgument(value); }

    // Phasor at time t: amplitude * e^(i * (omega * t + phase)).
    Complex<Number> value(Number time) const
    {
        return Complex<Number>::fromPolar(amplitude, omega * time + phase);
    }

    // |z(t)|: constant for an epicycle and non-negative by construction.
    Number magnitude(Number time) const
    {
        static_cast<void>(time);
        return amplitude;
    }

    // Principal argument of z(t); zero when the epicycle has zero amplitude,
    // matching Complex::argument for the additive identity.
    Number argument(Number time) const
    {
        if (amplitude == Zero<Number>)
        {
            return Zero<Number>;
        }
        return normalizeArgument(omega * time + phase);
    }

    // Unit phasor of the phase difference b - a, where a and b are the
    // arguments of this epicycle and other at the same time:
    // [cos(b - a), sin(b - a)].
    Complex<Number> similarity(const Epicycle &other, Number time) const
    {
        const Number difference = other.argument(time) - argument(time);
        return Complex<Number>(std::cos(difference), std::sin(difference));
    }

    // similarity scaled by both magnitudes:
    // [ma * mb * cos(b - a), ma * mb * sin(b - a)].
    Complex<Number> tendency(const Epicycle &other, Number time) const
    {
        const Number difference = other.argument(time) - argument(time);
        const Number factor = magnitude(time) * other.magnitude(time);
        return Complex<Number>(factor * std::cos(difference), factor * std::sin(difference));
    }

    Epicycle conjugate() const { return Epicycle(amplitude, -omega, -phase); }

    // d/dt value(t) = amplitude * omega * e^(i * (omega * t + phase + Pi/2)).
    Epicycle derivate() const { return Epicycle(amplitude * omega, omega, phase + PiOverTwo<Number>); }

    Complex<Number> derivate(Number time) const
    {
        return Complex<Number>::fromPolar(amplitude * omega, omega * time + phase + PiOverTwo<Number>);
    }

    // Antiderivative as another epicycle; undefined for omega == 0 because the
    // antiderivative of a constant phasor is linear in t, not an epicycle.
    Epicycle integral() const
    {
        if (omega == Zero<Number>)
        {
            throw std::domain_error("mathematics::Epicycle::integral: omega must be non-zero");
        }
        return Epicycle(amplitude / omega, omega, phase - PiOverTwo<Number>);
    }

    // Definite integral from start to end. Unlike integral(), a zero omega is
    // well defined here: the result is the constant phasor times the width.
    Complex<Number> integral(Number start, Number end) const
    {
        if (omega == Zero<Number>)
        {
            return value(start) * (end - start);
        }
        return (value(end) - value(start)) / Complex<Number>(Zero<Number>, omega);
    }

    bool isZero() const { return amplitude == Zero<Number>; }

    // Field-wise comparison with an absolute tolerance. Phase is compared by
    // angular distance, so phases that differ by a full turn are close.
    bool closeTo(const Epicycle &other, Number tolerance = DefaultTolerance<Number>) const
    {
        return std::abs(amplitude - other.amplitude) <= tolerance && std::abs(omega - other.omega) <= tolerance &&
               std::abs(normalizeArgument(phase - other.phase)) <= tolerance;
    }

    bool operator==(const Epicycle &other) const
    {
        return amplitude == other.amplitude && omega == other.omega && phase == other.phase;
    }

    bool operator!=(const Epicycle &other) const { return !(*this == other); }

    // -z(t): same frequency, phase shifted by half a turn.
    Epicycle operator-() const { return Epicycle(amplitude, omega, phase + Pi<Number>); }

    Epicycle &operator*=(Number scalar)
    {
        amplitude *= scalar;
        normalizeAmplitude();
        return *this;
    }

    Epicycle &operator/=(Number scalar)
    {
        if (scalar == Zero<Number>)
        {
            throw std::domain_error("mathematics::Epicycle::operator/=: division by zero");
        }
        amplitude /= scalar;
        normalizeAmplitude();
        return *this;
    }

    // Multiply by a phasor: scale the amplitude and rotate the phase.
    Epicycle &operator*=(const Complex<Number> &factor)
    {
        amplitude *= factor.magnitude();
        phase = normalizeArgument(phase + factor.argument());
        return *this;
    }

    Epicycle &operator/=(const Complex<Number> &divisor)
    {
        const Number divisorMagnitude = divisor.magnitude();
        if (divisorMagnitude == Zero<Number>)
        {
            throw std::domain_error("mathematics::Epicycle::operator/=: division by zero");
        }
        amplitude /= divisorMagnitude;
        phase = normalizeArgument(phase - divisor.argument());
        return *this;
    }

  private:
    // Restore the amplitude >= 0 invariant, folding the sign into the phase.
    void normalizeAmplitude()
    {
        if (amplitude < Zero<Number>)
        {
            amplitude = -amplitude;
            phase = normalizeArgument(phase + Pi<Number>);
        }
    }

    Number amplitude;
    Number omega;
    Number phase;
};

// Multiplying two epicycles stays closed: amplitudes multiply, omegas multiply
// and phases add, matching phasor multiplication of the two terms.
template <typename Number>
Epicycle<Number> operator*(const Epicycle<Number> &left, const Epicycle<Number> &right)
{
    return Epicycle<Number>(left.getAmplitude() * right.getAmplitude(), left.getOmega() * right.getOmega(),
                            left.getPhase() + right.getPhase());
}

// Scale and rotate by a phasor.
template <typename Number>
Epicycle<Number> operator*(const Epicycle<Number> &value, const Complex<Number> &factor)
{
    return Epicycle<Number>(value.getAmplitude() * factor.magnitude(), value.getOmega(),
                            value.getPhase() + factor.argument());
}

template <typename Number>
Epicycle<Number> operator*(const Complex<Number> &factor, const Epicycle<Number> &value)
{
    return value * factor;
}

template <typename Number>
Epicycle<Number> operator/(const Epicycle<Number> &value, const Complex<Number> &divisor)
{
    const Number divisorMagnitude = divisor.magnitude();
    if (divisorMagnitude == Zero<Number>)
    {
        throw std::domain_error("mathematics::operator/: division by zero");
    }
    return Epicycle<Number>(value.getAmplitude() / divisorMagnitude, value.getOmega(),
                            value.getPhase() - divisor.argument());
}

template <typename Number, typename Scalar>
std::enable_if_t<std::is_arithmetic<Scalar>::value, Epicycle<Number>> operator*(const Epicycle<Number> &value,
                                                                               Scalar scalar)
{
    return Epicycle<Number>(value.getAmplitude() * static_cast<Number>(scalar), value.getOmega(), value.getPhase());
}

template <typename Number, typename Scalar>
std::enable_if_t<std::is_arithmetic<Scalar>::value, Epicycle<Number>> operator*(Scalar scalar,
                                                                               const Epicycle<Number> &value)
{
    return value * scalar;
}

template <typename Number, typename Scalar>
std::enable_if_t<std::is_arithmetic<Scalar>::value, Epicycle<Number>> operator/(const Epicycle<Number> &value,
                                                                               Scalar scalar)
{
    const Number divisor = static_cast<Number>(scalar);
    if (divisor == Zero<Number>)
    {
        throw std::domain_error("mathematics::operator/: division by zero");
    }
    return Epicycle<Number>(value.getAmplitude() / divisor, value.getOmega(), value.getPhase());
}

template <typename Number>
std::ostream &operator<<(std::ostream &stream, const Epicycle<Number> &value)
{
    stream << "Epicycle(amplitude=" << value.getAmplitude() << ", omega=" << value.getOmega()
           << ", phase=" << value.getPhase() << ')';
    return stream;
}

} // namespace mathematics

// Epicycle::operator+ returns a Waveform, so its definition lives with
// Waveform. Including it here keeps `first + second` working for translation
// units that only need epicycles; #pragma once makes the reverse include safe.
#include "waveform.h"
