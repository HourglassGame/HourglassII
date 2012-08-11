#ifndef HG_MATHS_H
#define HG_MATHS_H
namespace hg {

template<typename Integral>
Integral iabs(Integral a) {
    return a < 0 ? -a : a;
}

//Returns floor(a/n) (with the division done exactly).
//Assumes that the operator/ of Integral performs truncated
//division.
template<typename Integral>
Integral flooredDivision(Integral a, Integral n) {
    //Assumes Integral division truncates.
    Integral q(a/n);
    if ((a < 0 && !(n < 0)) || (n < 0 && !(a < 0))) {
        //Take absolute value to avoid unspecified behaviour in C++03
        if (iabs(a) % iabs(n) != 0) {
            --q;
        }
    }
    return q;
}

//flooredModulo: Modulo function for use in the construction
//looping topologies. The result will always be between 0 and the
//denominator, and will loop in a natural fashion (rather than swapping
//the looping direction over the zero point (as in C++11),
//or being unspecified (as in earlier C++).
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
}//namespace hg
#endif //HG_MATHS_H

