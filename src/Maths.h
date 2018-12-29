#ifndef HG_MATHS_H
#define HG_MATHS_H
#include <algorithm>
#include <SFML/System/Vector2.hpp>
namespace hg {

template<typename T> struct vec2{
    T a;
    T b;
};
//Should be very standard layout, for interop with Vulkan etc.
static_assert(sizeof(vec2<float>) == 2 * sizeof(float));
static_assert(offsetof(vec2<float>, a) == 0);
static_assert(offsetof(vec2<float>, b) == 0 + sizeof(float));
static_assert(sizeof(vec2<double>) == 2 * sizeof(double));
static_assert(offsetof(vec2<double>, a) == 0);
static_assert(offsetof(vec2<double>, b) == 0 + sizeof(double));
static_assert(sizeof(vec2<short>) == 2 * sizeof(short));
static_assert(offsetof(vec2<short>, a) == 0);
static_assert(offsetof(vec2<short>, b) == 0 + sizeof(short));
static_assert(sizeof(vec2<int>) == 2 * sizeof(int));
static_assert(offsetof(vec2<int>, a) == 0);
static_assert(offsetof(vec2<int>, b) == 0 + sizeof(int));


template<typename T> struct vec3 {
    T a;
    T b;
    T c;
};
//Should be very standard layout, for interop with Vulkan etc.
static_assert(sizeof(vec3<float>) == 3 * sizeof(float));
static_assert(offsetof(vec3<float>, a) == 0);
static_assert(offsetof(vec3<float>, b) == 0 + sizeof(float));
static_assert(offsetof(vec3<float>, c) == 0 +  2*sizeof(float));
static_assert(sizeof(vec3<double>) == 3 * sizeof(double));
static_assert(offsetof(vec3<double>, a) == 0);
static_assert(offsetof(vec3<double>, b) == 0 + sizeof(double));
static_assert(offsetof(vec3<double>, c) == 0 + 2*sizeof(double));
static_assert(sizeof(vec3<short>) == 3 * sizeof(short));
static_assert(offsetof(vec3<short>, a) == 0);
static_assert(offsetof(vec3<short>, b) == 0 + sizeof(short));
static_assert(offsetof(vec3<short>, c) == 0 + 2*sizeof(short));
static_assert(sizeof(vec3<int>) == 3 * sizeof(int));
static_assert(offsetof(vec3<int>, a) == 0);
static_assert(offsetof(vec3<int>, b) == 0 + sizeof(int));
static_assert(offsetof(vec3<int>, c) == 0 + 2*sizeof(int));

inline float magnitude(sf::Vector2f const vec) {
    return std::sqrt(vec.x*vec.x + vec.y*vec.y);
}

inline sf::Vector2f normal(sf::Vector2f const vec) {
    sf::Vector2f direction(vec.y, -vec.x);
    assert(magnitude(direction));
    return direction / magnitude(direction);
}


template<typename Numeric>
Numeric sign(Numeric a) {
    return a == 0 ? 0 : a < 0 ? -1 : 1;
}

template<typename Integral>
Integral iabs(Integral a) {
    return a < 0 ? -a : a;
}

//Let ÷ be mathematical division, and / be C++ division.
//Returns floor(a÷b).
//We know
//    a÷b = a/b + f (f is the remainder, not all divisions have exact Integral results)
//and
//    (a/b)*b + a%b == a (from the standard).
//Together, these imply (through algebraic manipulation)
//    sign(f) == sign(a%b)*sign(b)
//We want the remainder (f) to always be >=0 (by definition of flooredDivision),
//so when sign(f) < 0, we subtract 1 from a/n to make f > 0.
template<typename Integral>
Integral flooredDivision(Integral a, Integral b) {
    Integral q(a/b);
    if ((a%b < 0 && b > 0) || (a%b > 0 && b < 0)) --q;
    return q;
}

//flooredModulo: Modulo function for use in the construction
//looping topologies. The result will always be between 0 and the
//denominator, and will loop in a natural fashion (rather than swapping
//the looping direction over the zero point (as in C++11),
//or being unspecified (as in earlier C++)).
//Returns x such that:
//
//Real a = Real(numerator)
//Real n = Real(denominator)
//Real r = a - n*floor(n/d)
//x = Integral(r)
template<typename Integral>
Integral flooredModulo(Integral a, Integral n) {
    return a - n * flooredDivision(a, n);
}


template<typename T>
T const& clamp(T const& lower, T const& val, T const& upper) {
    return std::max(lower, std::min(val, upper));
}

template<typename Float>
constexpr bool essentiallyEqual(Float const a, Float const b, Float const epsilon) noexcept
{
    return std::abs(a - b) <= std::min(std::abs(a), std::abs(b)) * epsilon;
}
}//namespace hg
#endif //HG_MATHS_H

