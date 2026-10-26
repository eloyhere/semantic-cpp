#pragma once

#include <cmath>
#include <ostream>
#include <stdexcept>
#include <type_traits>

namespace mathematics
{

template <typename Number>
constexpr Number Pi = static_cast<Number>(3.141592653589793238462643383279502884L);

template <typename Number>
constexpr Number PiOverTwo = static_cast<Number>(1.570796326794896619231321691639751442L);

template <typename Number>
constexpr Number DoublePi = static_cast<Number>(6.283185307179586476925286766559005768L);

template <typename Number>
constexpr Number Zero = static_cast<Number>(0);

template <typename Number>
constexpr Number One = static_cast<Number>(1);

template <typename Number>
constexpr Number Euler = static_cast<Number>(2.718281828459045235360287471352662498L);

// Absolute tolerance used by the closeTo family of comparisons.
template <typename Number>
constexpr Number DefaultTolerance = static_cast<Number>(1e-9);

// Fold an angle into the half-open interval (-Pi, Pi] so that phases produced
// by different code paths compare equal.
template <typename Number>
Number normalizeArgument(Number angle)
{
    static_assert(std::is_floating_point<Number>::value,
                  "mathematics: Number must be a floating-point type");
    Number remainder = std::fmod(angle, DoublePi<Number>);
    if (remainder <= -Pi<Number>)
    {
        remainder += DoublePi<Number>;
    }
    else if (remainder > Pi<Number>)
    {
        remainder -= DoublePi<Number>;
    }
    return remainder;
}

// A complex number in rectangular form. The derived phasor operations
// (argument, magnitude, similarity, tendency) are expressed in polar form.
template <typename Number>
class Complex
{
    static_assert(std::is_floating_point<Number>::value,
                  "mathematics::Complex requires a floating-point Number type");

  public:
    Complex() : real(Zero<Number>), imaginary(Zero<Number>) {}

    explicit Complex(Number realValue) : real(realValue), imaginary(Zero<Number>) {}

    Complex(Number realValue, Number imaginaryValue) : real(realValue), imaginary(imaginaryValue) {}

    Complex(const Complex &other) : real(other.real), imaginary(other.imaginary) {}

    Complex(Complex &&other) noexcept : real(other.real), imaginary(other.imaginary)
    {
        other.real = Zero<Number>;
        other.imaginary = Zero<Number>;
    }

    Complex &operator=(const Complex &other)
    {
        if (this != &other)
        {
            real = other.real;
            imaginary = other.imaginary;
        }
        return *this;
    }

    Complex &operator=(Complex &&other) noexcept
    {
        if (this != &other)
        {
            real = other.real;
            imaginary = other.imaginary;
            other.real = Zero<Number>;
            other.imaginary = Zero<Number>;
        }
        return *this;
    }

    ~Complex() = default;

    // Build a complex number from its polar form. A negative magnitude rotates
    // the result by Pi, exactly like multiplying by a negative real scalar.
    static Complex fromPolar(Number magnitudeValue, Number argumentValue)
    {
        return Complex(magnitudeValue * std::cos(argumentValue), magnitudeValue * std::sin(argumentValue));
    }

    Number getReal() const { return real; }
    Number getImaginary() const { return imaginary; }
    void setReal(Number value) { real = value; }
    void setImaginary(Number value) { imaginary = value; }

    Number squaredMagnitude() const { return real * real + imaginary * imaginary; }

    Number magnitude() const { return std::sqrt(squaredMagnitude()); }

    // The principal argument in (-Pi, Pi]. The additive identity has no
    // argument; it is reported as 0 by convention instead of throwing.
    Number argument() const
    {
        if (isZero())
        {
            return Zero<Number>;
        }
        return std::atan2(imaginary, real);
    }

    Complex conjugate() const { return Complex(real, -imaginary); }

    bool isZero() const { return real == Zero<Number> && imaginary == Zero<Number>; }

    Complex reciprocal() const
    {
        const Number squared = squaredMagnitude();
        if (squared == Zero<Number>)
        {
            throw std::domain_error("mathematics::Complex::reciprocal: division by zero");
        }
        return Complex(real / squared, -imaginary / squared);
    }

    // Unit phasor of the phase difference b - a, where a is this argument and
    // b is other's argument: [cos(b - a), sin(b - a)].
    Complex similarity(const Complex &other) const
    {
        const Number difference = other.argument() - argument();
        return Complex(std::cos(difference), std::sin(difference));
    }

    // similarity scaled by both magnitudes:
    // [ma * mb * cos(b - a), ma * mb * sin(b - a)].
    Complex tendency(const Complex &other) const
    {
        const Number difference = other.argument() - argument();
        const Number factor = magnitude() * other.magnitude();
        return Complex(factor * std::cos(difference), factor * std::sin(difference));
    }

    bool closeTo(const Complex &other, Number tolerance = DefaultTolerance<Number>) const
    {
        return std::abs(real - other.real) <= tolerance && std::abs(imaginary - other.imaginary) <= tolerance;
    }

    Complex &operator+=(const Complex &other)
    {
        real += other.real;
        imaginary += other.imaginary;
        return *this;
    }

    Complex &operator-=(const Complex &other)
    {
        real -= other.real;
        imaginary -= other.imaginary;
        return *this;
    }

    Complex &operator*=(const Complex &other)
    {
        const Number realValue = real * other.real - imaginary * other.imaginary;
        const Number imaginaryValue = real * other.imaginary + imaginary * other.real;
        real = realValue;
        imaginary = imaginaryValue;
        return *this;
    }

    Complex &operator/=(const Complex &other)
    {
        const Number squared = other.squaredMagnitude();
        if (squared == Zero<Number>)
        {
            throw std::domain_error("mathematics::Complex::operator/=: division by zero");
        }
        const Number realValue = (real * other.real + imaginary * other.imaginary) / squared;
        const Number imaginaryValue = (imaginary * other.real - real * other.imaginary) / squared;
        real = realValue;
        imaginary = imaginaryValue;
        return *this;
    }

    bool operator==(const Complex &other) const { return real == other.real && imaginary == other.imaginary; }

    bool operator!=(const Complex &other) const { return !(*this == other); }

    Complex operator-() const { return Complex(-real, -imaginary); }

  private:
    Number real;
    Number imaginary;
};

template <typename Number>
Complex<Number> operator+(const Complex<Number> &left, const Complex<Number> &right)
{
    return Complex<Number>(left.getReal() + right.getReal(), left.getImaginary() + right.getImaginary());
}

template <typename Number>
Complex<Number> operator-(const Complex<Number> &left, const Complex<Number> &right)
{
    return Complex<Number>(left.getReal() - right.getReal(), left.getImaginary() - right.getImaginary());
}

template <typename Number>
Complex<Number> operator*(const Complex<Number> &left, const Complex<Number> &right)
{
    return Complex<Number>(left.getReal() * right.getReal() - left.getImaginary() * right.getImaginary(),
                           left.getReal() * right.getImaginary() + left.getImaginary() * right.getReal());
}

template <typename Number>
Complex<Number> operator/(const Complex<Number> &left, const Complex<Number> &right)
{
    const Number squared = right.squaredMagnitude();
    if (squared == Zero<Number>)
    {
        throw std::domain_error("mathematics::operator/: division by zero");
    }
    return Complex<Number>((left.getReal() * right.getReal() + left.getImaginary() * right.getImaginary()) / squared,
                           (left.getImaginary() * right.getReal() - left.getReal() * right.getImaginary()) / squared);
}

// Scalar overloads accept any arithmetic type, so Complex<double>() * 2 works
// without an implicit Complex(Number) conversion.
template <typename Number, typename Scalar>
std::enable_if_t<std::is_arithmetic<Scalar>::value, Complex<Number>> operator+(const Complex<Number> &value,
                                                                               Scalar scalar)
{
    const Number addend = static_cast<Number>(scalar);
    return Complex<Number>(value.getReal() + addend, value.getImaginary());
}

template <typename Number, typename Scalar>
std::enable_if_t<std::is_arithmetic<Scalar>::value, Complex<Number>> operator+(Scalar scalar,
                                                                               const Complex<Number> &value)
{
    return value + scalar;
}

template <typename Number, typename Scalar>
std::enable_if_t<std::is_arithmetic<Scalar>::value, Complex<Number>> operator-(const Complex<Number> &value,
                                                                               Scalar scalar)
{
    const Number subtrahend = static_cast<Number>(scalar);
    return Complex<Number>(value.getReal() - subtrahend, value.getImaginary());
}

template <typename Number, typename Scalar>
std::enable_if_t<std::is_arithmetic<Scalar>::value, Complex<Number>> operator-(Scalar scalar,
                                                                               const Complex<Number> &value)
{
    return Complex<Number>(static_cast<Number>(scalar) - value.getReal(), -value.getImaginary());
}

template <typename Number, typename Scalar>
std::enable_if_t<std::is_arithmetic<Scalar>::value, Complex<Number>> operator*(const Complex<Number> &value,
                                                                               Scalar scalar)
{
    const Number factor = static_cast<Number>(scalar);
    return Complex<Number>(value.getReal() * factor, value.getImaginary() * factor);
}

template <typename Number, typename Scalar>
std::enable_if_t<std::is_arithmetic<Scalar>::value, Complex<Number>> operator*(Scalar scalar,
                                                                               const Complex<Number> &value)
{
    return value * scalar;
}

template <typename Number, typename Scalar>
std::enable_if_t<std::is_arithmetic<Scalar>::value, Complex<Number>> operator/(const Complex<Number> &value,
                                                                               Scalar scalar)
{
    const Number divisor = static_cast<Number>(scalar);
    if (divisor == Zero<Number>)
    {
        throw std::domain_error("mathematics::operator/: division by zero");
    }
    return Complex<Number>(value.getReal() / divisor, value.getImaginary() / divisor);
}

template <typename Number>
std::ostream &operator<<(std::ostream &stream, const Complex<Number> &value)
{
    stream << '(' << value.getReal() << ", " << value.getImaginary() << ')';
    return stream;
}

} // namespace mathematics
