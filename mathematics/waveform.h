#pragma once

#include <cstddef>
#include <deque>
#include <initializer_list>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "epicycle.h"

namespace mathematics
{

// A sum of epicycles: z(t) = sum over all terms of
// amplitude * e^(i * (omega * t + phase)). Terms are stored in insertion
// order; evaluation is order independent. An empty waveform evaluates to 0.
template <typename Number>
class Waveform
{
    static_assert(std::is_floating_point<Number>::value,
                  "mathematics::Waveform requires a floating-point Number type");

  public:
    using container_type = std::deque<Epicycle<Number>>;
    using value_type = Epicycle<Number>;
    using size_type = std::size_t;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    Waveform() = default;

    Waveform(std::initializer_list<Epicycle<Number>> terms) : epicycles(terms) {}

    template <typename Iterator>
    Waveform(Iterator first, Iterator last) : epicycles(first, last)
    {
    }

    explicit Waveform(const container_type &terms) : epicycles(terms) {}

    explicit Waveform(container_type &&terms) : epicycles(std::move(terms)) {}

    Waveform(const Waveform &other) = default;

    Waveform(Waveform &&other) = default;

    Waveform &operator=(const Waveform &other) = default;

    Waveform &operator=(Waveform &&other) = default;

    ~Waveform() = default;

    size_type size() const { return epicycles.size(); }

    bool empty() const { return epicycles.empty(); }

    const Epicycle<Number> &operator[](size_type index) const { return epicycles[index]; }
    Epicycle<Number> &operator[](size_type index) { return epicycles[index]; }

    const Epicycle<Number> &at(size_type index) const { return epicycles.at(index); }
    Epicycle<Number> &at(size_type index) { return epicycles.at(index); }

    iterator begin() { return epicycles.begin(); }
    iterator end() { return epicycles.end(); }
    const_iterator begin() const { return epicycles.begin(); }
    const_iterator end() const { return epicycles.end(); }
    const_iterator cbegin() const { return epicycles.cbegin(); }
    const_iterator cend() const { return epicycles.cend(); }

    const container_type &terms() const { return epicycles; }
    container_type &terms() { return epicycles; }

    void add(const Epicycle<Number> &term) { epicycles.push_back(term); }
    void add(Epicycle<Number> &&term) { epicycles.push_back(std::move(term)); }
    void clear() { epicycles.clear(); }

    // Sum of every term's phasor at time t.
    Complex<Number> value(Number time) const
    {
        Complex<Number> result;
        for (const Epicycle<Number> &epicycle : epicycles)
        {
            result += epicycle.value(time);
        }
        return result;
    }

    Number magnitude(Number time) const { return value(time).magnitude(); }

    Number argument(Number time) const { return value(time).argument(); }

    // Unit phasor of the phase difference b - a between the summed waveforms:
    // [cos(b - a), sin(b - a)].
    Complex<Number> similarity(const Waveform &other, Number time) const
    {
        const Number difference = other.argument(time) - argument(time);
        return Complex<Number>(std::cos(difference), std::sin(difference));
    }

    // similarity scaled by both magnitudes:
    // [ma * mb * cos(b - a), ma * mb * sin(b - a)].
    Complex<Number> tendency(const Waveform &other, Number time) const
    {
        const Number difference = other.argument(time) - argument(time);
        const Number factor = magnitude(time) * other.magnitude(time);
        return Complex<Number>(factor * std::cos(difference), factor * std::sin(difference));
    }

    Waveform conjugate() const
    {
        Waveform result;
        for (const Epicycle<Number> &epicycle : epicycles)
        {
            result.add(epicycle.conjugate());
        }
        return result;
    }

    Waveform derivate() const
    {
        Waveform result;
        for (const Epicycle<Number> &epicycle : epicycles)
        {
            result.add(epicycle.derivate());
        }
        return result;
    }

    Complex<Number> derivate(Number time) const
    {
        Complex<Number> result;
        for (const Epicycle<Number> &epicycle : epicycles)
        {
            result += epicycle.derivate(time);
        }
        return result;
    }

    // Term-wise antiderivative; throws std::domain_error if any term has a zero
    // omega, whose antiderivative is not an epicycle.
    Waveform integral() const
    {
        Waveform result;
        for (const Epicycle<Number> &epicycle : epicycles)
        {
            result.add(epicycle.integral());
        }
        return result;
    }

    Complex<Number> integral(Number start, Number end) const
    {
        Complex<Number> result;
        for (const Epicycle<Number> &epicycle : epicycles)
        {
            result += epicycle.integral(start, end);
        }
        return result;
    }

    bool isZero() const
    {
        for (const Epicycle<Number> &epicycle : epicycles)
        {
            if (!epicycle.isZero())
            {
                return false;
            }
        }
        return true;
    }

    bool closeTo(const Waveform &other, Number tolerance = DefaultTolerance<Number>) const
    {
        if (size() != other.size())
        {
            return false;
        }
        auto left = epicycles.begin();
        auto right = other.epicycles.begin();
        for (; left != epicycles.end(); ++left, ++right)
        {
            if (!left->closeTo(*right, tolerance))
            {
                return false;
            }
        }
        return true;
    }

    bool operator==(const Waveform &other) const { return epicycles == other.epicycles; }

    bool operator!=(const Waveform &other) const { return !(*this == other); }

    Waveform &operator+=(const Waveform &other)
    {
        epicycles.insert(epicycles.end(), other.epicycles.begin(), other.epicycles.end());
        return *this;
    }

    Waveform &operator+=(const Epicycle<Number> &term)
    {
        epicycles.push_back(term);
        return *this;
    }

    Waveform &operator-=(const Waveform &other)
    {
        for (const Epicycle<Number> &epicycle : other.epicycles)
        {
            epicycles.push_back(-epicycle);
        }
        return *this;
    }

    Waveform &operator-=(const Epicycle<Number> &term)
    {
        epicycles.push_back(-term);
        return *this;
    }

    Waveform &operator*=(Number scalar)
    {
        for (Epicycle<Number> &epicycle : epicycles)
        {
            epicycle *= scalar;
        }
        return *this;
    }

    Waveform &operator/=(Number scalar)
    {
        if (scalar == Zero<Number>)
        {
            throw std::domain_error("mathematics::Waveform::operator/=: division by zero");
        }
        for (Epicycle<Number> &epicycle : epicycles)
        {
            epicycle /= scalar;
        }
        return *this;
    }

    Waveform &operator*=(const Complex<Number> &factor)
    {
        for (Epicycle<Number> &epicycle : epicycles)
        {
            epicycle *= factor;
        }
        return *this;
    }

    Waveform operator-() const
    {
        Waveform result;
        for (const Epicycle<Number> &epicycle : epicycles)
        {
            result.add(-epicycle);
        }
        return result;
    }

  private:
    container_type epicycles;
};

// Superposition is concatenation: the terms of both waveforms are all kept.
template <typename Number>
Waveform<Number> operator+(const Epicycle<Number> &left, const Epicycle<Number> &right)
{
    Waveform<Number> result;
    result.add(left);
    result.add(right);
    return result;
}

template <typename Number>
Waveform<Number> operator+(const Waveform<Number> &left, const Epicycle<Number> &right)
{
    Waveform<Number> result(left);
    result.add(right);
    return result;
}

template <typename Number>
Waveform<Number> operator+(const Epicycle<Number> &left, const Waveform<Number> &right)
{
    Waveform<Number> result;
    result.add(left);
    result += right;
    return result;
}

template <typename Number>
Waveform<Number> operator+(const Waveform<Number> &left, const Waveform<Number> &right)
{
    Waveform<Number> result(left);
    result += right;
    return result;
}

template <typename Number>
Waveform<Number> operator-(const Waveform<Number> &left, const Waveform<Number> &right)
{
    Waveform<Number> result(left);
    result -= right;
    return result;
}

template <typename Number>
Waveform<Number> operator*(const Waveform<Number> &value, const Complex<Number> &factor)
{
    Waveform<Number> result(value);
    result *= factor;
    return result;
}

template <typename Number>
Waveform<Number> operator*(const Complex<Number> &factor, const Waveform<Number> &value)
{
    return value * factor;
}

template <typename Number, typename Scalar>
std::enable_if_t<std::is_arithmetic<Scalar>::value, Waveform<Number>> operator*(const Waveform<Number> &value,
                                                                                Scalar scalar)
{
    Waveform<Number> result(value);
    result *= static_cast<Number>(scalar);
    return result;
}

template <typename Number, typename Scalar>
std::enable_if_t<std::is_arithmetic<Scalar>::value, Waveform<Number>> operator*(Scalar scalar,
                                                                                const Waveform<Number> &value)
{
    return value * scalar;
}

template <typename Number, typename Scalar>
std::enable_if_t<std::is_arithmetic<Scalar>::value, Waveform<Number>> operator/(const Waveform<Number> &value,
                                                                                Scalar scalar)
{
    Waveform<Number> result(value);
    result /= static_cast<Number>(scalar);
    return result;
}

template <typename Number>
std::ostream &operator<<(std::ostream &stream, const Waveform<Number> &value)
{
    stream << "Waveform[" << value.size() << "]: {";
    for (std::size_t index = 0; index < value.size(); ++index)
    {
        if (index != 0)
        {
            stream << ", ";
        }
        stream << value[index];
    }
    stream << '}';
    return stream;
}

} // namespace mathematics
