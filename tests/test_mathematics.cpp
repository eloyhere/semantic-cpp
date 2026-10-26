#include "test_framework.h"

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "mathematics/mathematics.h"

namespace
{

constexpr double Tolerance = 1e-9;

bool close(double left, double right)
{
    return std::abs(left - right) <= Tolerance;
}

} // namespace

TEST_CASE("mathematics: constants")
{
    CHECK(close(mathematics::Pi<double>, 3.141592653589793));
    CHECK(close(mathematics::PiOverTwo<double>, 1.5707963267948966));
    CHECK(close(mathematics::DoublePi<double>, 6.283185307179586));
    CHECK(close(mathematics::Euler<double>, 2.718281828459045));
    CHECK_EQ(mathematics::Zero<double>, 0.0);
    CHECK_EQ(mathematics::One<double>, 1.0);

    CHECK(close(mathematics::normalizeArgument(mathematics::Pi<double> * 3.0), mathematics::Pi<double>));
    CHECK(close(mathematics::normalizeArgument(-mathematics::Pi<double>), mathematics::Pi<double>));
    CHECK(close(mathematics::normalizeArgument(mathematics::Pi<double> * 0.5), mathematics::Pi<double> * 0.5));
}

TEST_CASE("mathematics: complex construction and accessors")
{
    mathematics::Complex<double> zero;
    CHECK_EQ(zero.getReal(), 0.0);
    CHECK_EQ(zero.getImaginary(), 0.0);
    CHECK(zero.isZero());
    CHECK_EQ(zero.argument(), 0.0);
    CHECK_EQ(zero.magnitude(), 0.0);

    mathematics::Complex<double> value(3.0, 4.0);
    CHECK_EQ(value.getReal(), 3.0);
    CHECK_EQ(value.getImaginary(), 4.0);
    CHECK(close(value.magnitude(), 5.0));
    CHECK(close(value.squaredMagnitude(), 25.0));
    CHECK(close(value.argument(), std::atan2(4.0, 3.0)));

    mathematics::Complex<double> realOnly(2.5);
    CHECK_EQ(realOnly.getReal(), 2.5);
    CHECK_EQ(realOnly.getImaginary(), 0.0);

    value.setReal(1.0);
    value.setImaginary(-1.0);
    CHECK_EQ(value.getReal(), 1.0);
    CHECK_EQ(value.getImaginary(), -1.0);

    mathematics::Complex<double> copied(value);
    CHECK(copied == value);

    mathematics::Complex<double> moved(std::move(copied));
    CHECK(moved == value);
    CHECK(copied.isZero());

    mathematics::Complex<double> assigned;
    assigned = moved;
    CHECK(assigned == moved);

    mathematics::Complex<double> moveAssigned;
    moveAssigned = std::move(assigned);
    CHECK(moveAssigned == moved);
    CHECK(assigned.isZero());
}

TEST_CASE("mathematics: complex polar form")
{
    const mathematics::Complex<double> value =
        mathematics::Complex<double>::fromPolar(2.0, mathematics::Pi<double> * 0.5);
    CHECK(close(value.getReal(), 0.0));
    CHECK(close(value.getImaginary(), 2.0));
    CHECK(close(value.magnitude(), 2.0));
    CHECK(close(value.argument(), mathematics::Pi<double> * 0.5));

    const mathematics::Complex<double> rotated =
        mathematics::Complex<double>::fromPolar(-2.0, mathematics::Pi<double> * 0.5);
    CHECK(close(rotated.getReal(), 0.0));
    CHECK(close(rotated.getImaginary(), -2.0));
}

TEST_CASE("mathematics: complex arithmetic")
{
    const mathematics::Complex<double> left(1.0, 2.0);
    const mathematics::Complex<double> right(3.0, -4.0);

    CHECK(left + right == mathematics::Complex<double>(4.0, -2.0));
    CHECK(left - right == mathematics::Complex<double>(-2.0, 6.0));
    CHECK(left * right == mathematics::Complex<double>(11.0, 2.0));
    CHECK((left / right).closeTo(mathematics::Complex<double>(-0.2, 0.4)));

    CHECK(-left == mathematics::Complex<double>(-1.0, -2.0));

    CHECK(left * 2.0 == mathematics::Complex<double>(2.0, 4.0));
    CHECK(2.0 * left == mathematics::Complex<double>(2.0, 4.0));
    CHECK(left * 2 == mathematics::Complex<double>(2.0, 4.0));
    CHECK(left + 1 == mathematics::Complex<double>(2.0, 2.0));
    CHECK(1 + left == mathematics::Complex<double>(2.0, 2.0));
    CHECK(left - 1 == mathematics::Complex<double>(0.0, 2.0));
    CHECK(1 - left == mathematics::Complex<double>(0.0, -2.0));
    CHECK(left / 2.0 == mathematics::Complex<double>(0.5, 1.0));

    mathematics::Complex<double> compound(left);
    compound += right;
    CHECK(compound == mathematics::Complex<double>(4.0, -2.0));
    compound -= right;
    CHECK(compound == left);
    compound *= right;
    CHECK(compound == mathematics::Complex<double>(11.0, 2.0));
    compound /= right;
    CHECK(compound.closeTo(left));

    const mathematics::Complex<double> reciprocal = left.reciprocal();
    CHECK((left * reciprocal).closeTo(mathematics::Complex<double>(1.0, 0.0)));
}

TEST_CASE("mathematics: complex division by zero")
{
    const mathematics::Complex<double> value(1.0, 2.0);
    CHECK_THROWS_AS(value / mathematics::Complex<double>(), std::domain_error);
    CHECK_THROWS_AS(value / 0.0, std::domain_error);
    CHECK_THROWS_AS(mathematics::Complex<double>().reciprocal(), std::domain_error);

    mathematics::Complex<double> compound(value);
    CHECK_THROWS_AS(compound /= mathematics::Complex<double>(), std::domain_error);
}

TEST_CASE("mathematics: complex conjugate, similarity and tendency")
{
    const mathematics::Complex<double> value(3.0, 4.0);
    const mathematics::Complex<double> conjugate = value.conjugate();
    CHECK_EQ(conjugate.getReal(), 3.0);
    CHECK_EQ(conjugate.getImaginary(), -4.0);
    CHECK(close(conjugate.magnitude(), value.magnitude()));

    const mathematics::Complex<double> unitReal(1.0, 0.0);
    const mathematics::Complex<double> unitImaginary(0.0, 2.0);

    const mathematics::Complex<double> similarity = unitReal.similarity(unitImaginary);
    CHECK(close(similarity.getReal(), 0.0));
    CHECK(close(similarity.getImaginary(), 1.0));
    CHECK(close(similarity.magnitude(), 1.0));

    const mathematics::Complex<double> tendency = unitReal.tendency(unitImaginary);
    CHECK(close(tendency.getReal(), 0.0));
    CHECK(close(tendency.getImaginary(), 2.0));

    const mathematics::Complex<double> zero;
    const mathematics::Complex<double> zeroSimilarity = zero.similarity(zero);
    CHECK_EQ(zeroSimilarity.getReal(), 1.0);
    CHECK_EQ(zeroSimilarity.getImaginary(), 0.0);
    CHECK(zero.tendency(unitImaginary).isZero());
}

TEST_CASE("mathematics: epicycle construction and normalization")
{
    mathematics::Epicycle<double> zero;
    CHECK_EQ(zero.getAmplitude(), 0.0);
    CHECK_EQ(zero.getOmega(), 0.0);
    CHECK_EQ(zero.getPhase(), 0.0);
    CHECK(zero.isZero());

    mathematics::Epicycle<double> negative(-2.0, 1.0, 0.0);
    CHECK(close(negative.getAmplitude(), 2.0));
    CHECK(close(negative.getPhase(), mathematics::Pi<double>));
    CHECK(negative.value(0.0).closeTo(mathematics::Complex<double>(-2.0, 0.0)));

    mathematics::Epicycle<double> wrapped(1.0, 0.0, mathematics::Pi<double> * 1.5);
    CHECK(close(wrapped.getPhase(), -mathematics::Pi<double> * 0.5));

    mathematics::Epicycle<double> copied(wrapped);
    CHECK(copied == wrapped);

    mathematics::Epicycle<double> moved(std::move(copied));
    CHECK(moved == wrapped);
    CHECK(copied.isZero());
}

TEST_CASE("mathematics: epicycle value, magnitude and argument")
{
    const mathematics::Epicycle<double> value(2.0, 3.0, mathematics::Pi<double> * 0.5);

    CHECK(close(value.magnitude(0.0), 2.0));
    CHECK(close(value.magnitude(7.5), 2.0));
    CHECK(close(value.argument(0.0), mathematics::Pi<double> * 0.5));
    CHECK(close(value.argument(1.0), mathematics::normalizeArgument(3.0 + mathematics::Pi<double> * 0.5)));

    CHECK(value.value(0.0).closeTo(mathematics::Complex<double>(0.0, 2.0)));
    CHECK(value.value(1.0).closeTo(
        mathematics::Complex<double>::fromPolar(2.0, 3.0 + mathematics::Pi<double> * 0.5)));

    mathematics::Epicycle<double> silent(0.0, 5.0, 1.0);
    CHECK_EQ(silent.argument(2.0), 0.0);
    CHECK_EQ(silent.value(2.0).magnitude(), 0.0);
}

TEST_CASE("mathematics: epicycle derivate and integral")
{
    const mathematics::Epicycle<double> value(2.0, 3.0, mathematics::Pi<double> * 0.25);

    const mathematics::Epicycle<double> derivative = value.derivate();
    CHECK(close(derivative.getAmplitude(), 6.0));
    CHECK(close(derivative.getOmega(), 3.0));
    CHECK(close(derivative.getPhase(), mathematics::Pi<double> * 0.75));
    CHECK(value.derivate(1.0).closeTo(
        mathematics::Complex<double>::fromPolar(6.0, 3.0 + mathematics::Pi<double> * 0.75)));

    const mathematics::Epicycle<double> antiderivative = value.integral();
    CHECK(close(antiderivative.getAmplitude(), 2.0 / 3.0));
    CHECK(close(antiderivative.getOmega(), 3.0));
    CHECK(close(antiderivative.getPhase(), -mathematics::Pi<double> * 0.25));

    const mathematics::Complex<double> definite = value.integral(0.0, 1.0);
    CHECK(definite.closeTo(antiderivative.value(1.0) - antiderivative.value(0.0)));

    const mathematics::Epicycle<double> constant(2.0, 0.0, 0.0);
    CHECK(constant.derivate().isZero());
    CHECK(constant.derivate(5.0).isZero());
    CHECK_THROWS_AS(constant.integral(), std::domain_error);
    CHECK(constant.integral(0.0, 3.0).closeTo(mathematics::Complex<double>(6.0, 0.0)));
}

TEST_CASE("mathematics: epicycle conjugate, similarity and tendency")
{
    const mathematics::Epicycle<double> left(1.0, 2.0, 0.0);
    const mathematics::Epicycle<double> right(3.0, 2.0, mathematics::Pi<double> * 0.5);

    const mathematics::Epicycle<double> conjugate = left.conjugate();
    CHECK(close(conjugate.getOmega(), -2.0));
    CHECK(close(conjugate.getPhase(), 0.0));
    CHECK(conjugate.value(0.5).closeTo(left.value(0.5).conjugate()));

    const mathematics::Complex<double> similarity = left.similarity(right, 0.0);
    CHECK(similarity.closeTo(mathematics::Complex<double>(0.0, 1.0)));

    const mathematics::Complex<double> tendency = left.tendency(right, 0.0);
    CHECK(tendency.closeTo(mathematics::Complex<double>(0.0, 3.0)));
}

TEST_CASE("mathematics: epicycle arithmetic")
{
    const mathematics::Epicycle<double> left(2.0, 3.0, mathematics::Pi<double> * 0.25);
    const mathematics::Epicycle<double> right(4.0, 5.0, mathematics::Pi<double> * 0.25);

    const mathematics::Epicycle<double> product = left * right;
    CHECK(close(product.getAmplitude(), 8.0));
    CHECK(close(product.getOmega(), 15.0));
    CHECK(close(product.getPhase(), mathematics::Pi<double> * 0.5));

    CHECK(close((left * 2.0).getAmplitude(), 4.0));
    CHECK(close((2.0 * left).getAmplitude(), 4.0));
    CHECK(close((left / 2.0).getAmplitude(), 1.0));

    const mathematics::Epicycle<double> rotated =
        left * mathematics::Complex<double>::fromPolar(2.0, mathematics::Pi<double> * 0.5);
    CHECK(close(rotated.getAmplitude(), 4.0));
    CHECK(close(rotated.getPhase(), mathematics::Pi<double> * 0.75));

    const mathematics::Epicycle<double> negated = -left;
    CHECK(negated.value(0.0).closeTo(-left.value(0.0)));

    CHECK_THROWS_AS(left / 0.0, std::domain_error);
    CHECK_THROWS_AS(left / mathematics::Complex<double>(), std::domain_error);

    const mathematics::Waveform<double> sum = left + right;
    CHECK_EQ(sum.size(), 2u);
    CHECK(sum[0] == left);
    CHECK(sum[1] == right);
}

TEST_CASE("mathematics: waveform construction and container interface")
{
    mathematics::Waveform<double> empty;
    CHECK(empty.empty());
    CHECK_EQ(empty.size(), 0u);
    CHECK(empty.value(1.0).isZero());
    CHECK_EQ(empty.argument(1.0), 0.0);
    CHECK_EQ(empty.magnitude(1.0), 0.0);
    CHECK(empty.isZero());

    const mathematics::Epicycle<double> first(1.0, 1.0, 0.0);
    const mathematics::Epicycle<double> second(2.0, 2.0, 0.0);
    mathematics::Waveform<double> waveform{first, second};

    CHECK_EQ(waveform.size(), 2u);
    CHECK(waveform[0] == first);
    CHECK(waveform.at(1) == second);

    std::size_t count = 0;
    for (const mathematics::Epicycle<double> &epicycle : waveform)
    {
        static_cast<void>(epicycle);
        ++count;
    }
    CHECK_EQ(count, 2u);

    waveform.add(mathematics::Epicycle<double>(3.0, 3.0, 0.0));
    CHECK_EQ(waveform.size(), 3u);
    CHECK_EQ(waveform.terms().size(), 3u);

    mathematics::Waveform<double> copied(waveform);
    CHECK(copied == waveform);

    copied.clear();
    CHECK(copied.empty());
}

TEST_CASE("mathematics: waveform value, magnitude and argument")
{
    const mathematics::Epicycle<double> first(1.0, 1.0, 0.0);
    const mathematics::Epicycle<double> second(2.0, 2.0, 0.0);
    const mathematics::Waveform<double> waveform{first, second};

    CHECK(waveform.value(0.0).closeTo(mathematics::Complex<double>(3.0, 0.0)));
    CHECK(close(waveform.magnitude(0.0), 3.0));
    CHECK(close(waveform.argument(0.0), 0.0));

    const mathematics::Waveform<double> empty;
    CHECK(empty.value(2.5).isZero());
}

TEST_CASE("mathematics: waveform arithmetic and comparison")
{
    const mathematics::Waveform<double> left{mathematics::Epicycle<double>(1.0, 1.0, 0.0)};
    const mathematics::Waveform<double> right{mathematics::Epicycle<double>(2.0, 2.0, 0.0)};

    const mathematics::Waveform<double> combined = left + right;
    CHECK_EQ(combined.size(), 2u);
    CHECK(combined == combined);
    CHECK(combined != left);
    CHECK(combined.closeTo(combined));

    const mathematics::Waveform<double> scaled = combined * 2.0;
    CHECK(close(scaled[0].getAmplitude(), 2.0));
    CHECK(close(scaled[1].getAmplitude(), 4.0));
    CHECK(close((2.0 * combined)[0].getAmplitude(), 2.0));
    CHECK(close((combined / 2.0)[0].getAmplitude(), 0.5));

    const mathematics::Waveform<double> negated = -combined;
    CHECK(negated.value(0.0).closeTo(-combined.value(0.0)));

    const mathematics::Waveform<double> subtracted = left - right;
    CHECK_EQ(subtracted.size(), 2u);
    CHECK(subtracted.value(0.0).closeTo(left.value(0.0) - right.value(0.0)));

    const mathematics::Waveform<double> rotated =
        combined * mathematics::Complex<double>::fromPolar(2.0, mathematics::Pi<double> * 0.5);
    CHECK(close(rotated[0].getAmplitude(), 2.0));
    CHECK(close(rotated[0].getPhase(), mathematics::Pi<double> * 0.5));

    mathematics::Waveform<double> appended = left;
    appended += right;
    CHECK_EQ(appended.size(), 2u);
    appended += mathematics::Epicycle<double>(4.0, 4.0, 0.0);
    CHECK_EQ(appended.size(), 3u);
    appended -= right;
    CHECK_EQ(appended.size(), 4u);

    CHECK_THROWS_AS(combined / 0.0, std::domain_error);
}

TEST_CASE("mathematics: waveform calculus")
{
    const mathematics::Waveform<double> waveform{mathematics::Epicycle<double>(2.0, 3.0, mathematics::Pi<double> * 0.25),
                                                 mathematics::Epicycle<double>(1.0, 5.0, 0.0)};

    const mathematics::Waveform<double> derivative = waveform.derivate();
    CHECK_EQ(derivative.size(), 2u);
    CHECK(derivative.value(0.7).closeTo(waveform.derivate(0.7)));

    const mathematics::Waveform<double> antiderivative = waveform.integral();
    CHECK_EQ(antiderivative.size(), 2u);
    CHECK(waveform.integral(0.0, 1.0).closeTo(antiderivative.value(1.0) - antiderivative.value(0.0)));

    const mathematics::Waveform<double> conjugate = waveform.conjugate();
    CHECK(conjugate.value(0.4).closeTo(waveform.value(0.4).conjugate()));

    const mathematics::Waveform<double> constant{mathematics::Epicycle<double>(1.0, 0.0, 0.0)};
    CHECK_THROWS_AS(constant.integral(), std::domain_error);
    CHECK(constant.integral(0.0, 2.0).closeTo(mathematics::Complex<double>(2.0, 0.0)));
}

TEST_CASE("mathematics: waveform similarity and tendency")
{
    const mathematics::Waveform<double> left{mathematics::Epicycle<double>(1.0, 0.0, 0.0)};
    const mathematics::Waveform<double> right{mathematics::Epicycle<double>(2.0, 0.0, mathematics::Pi<double> * 0.5)};

    const mathematics::Complex<double> similarity = left.similarity(right, 0.0);
    CHECK(similarity.closeTo(mathematics::Complex<double>(0.0, 1.0)));

    const mathematics::Complex<double> tendency = left.tendency(right, 0.0);
    CHECK(tendency.closeTo(mathematics::Complex<double>(0.0, 2.0)));

    const mathematics::Waveform<double> empty;
    CHECK(empty.similarity(empty, 0.0).closeTo(mathematics::Complex<double>(1.0, 0.0)));
    CHECK(empty.tendency(left, 0.0).isZero());
}

TEST_CASE("mathematics: closeness with tolerance")
{
    const mathematics::Complex<double> value(3.0, 4.0);
    CHECK(value.closeTo(mathematics::Complex<double>(3.0, 4.0)));
    CHECK(!value.closeTo(mathematics::Complex<double>(3.0, 4.1)));
    CHECK(value.closeTo(mathematics::Complex<double>(3.0, 4.1), 0.2));

    // Phases straddling the -Pi/Pi seam are one angular step apart.
    const mathematics::Epicycle<double> nearPositivePi(1.0, 0.0, mathematics::Pi<double> - 0.0005);
    const mathematics::Epicycle<double> nearNegativePi(1.0, 0.0, -mathematics::Pi<double> + 0.0005);
    CHECK(nearPositivePi.closeTo(nearNegativePi, 0.002));
    CHECK(!nearPositivePi.closeTo(nearNegativePi, 0.0001));

    const mathematics::Waveform<double> first{mathematics::Epicycle<double>(1.0, 1.0, 0.0)};
    const mathematics::Waveform<double> second{mathematics::Epicycle<double>(1.0, 1.0, 0.0)};
    CHECK(first.closeTo(second));
    CHECK(!first.closeTo(mathematics::Waveform<double>{}));
}

TEST_CASE("mathematics: stream output")
{
    std::ostringstream complexStream;
    complexStream << mathematics::Complex<double>(1.5, -2.5);
    CHECK_EQ(complexStream.str(), std::string("(1.5, -2.5)"));

    std::ostringstream epicycleStream;
    epicycleStream << mathematics::Epicycle<double>(2.0, 3.0, 0.0);
    CHECK(epicycleStream.str().find("Epicycle") != std::string::npos);

    std::ostringstream waveformStream;
    waveformStream << mathematics::Waveform<double>{};
    CHECK_EQ(waveformStream.str(), std::string("Waveform[0]: {}"));
}
