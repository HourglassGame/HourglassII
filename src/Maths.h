#ifndef HG_MATHS_H
#define HG_MATHS_H
#include <algorithm>
namespace hg {

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

}//namespace hg
#endif //HG_MATHS_H

