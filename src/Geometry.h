#ifndef HG_GEOMETRY_H
#define HG_GEOMETRY_H
#include <utility>
namespace hg {
template<typename T>
struct Vector2 final {
    Vector2() :
        x(), y() {}
    
    Vector2(T nx, T ny) :
        x(std::move(nx)), y(std::move(ny)) {}
    T x;
    T y;
};

template<typename T>
struct Rect final {
    Rect() :
        x(), y(), w(), h() {}
    
    Rect(T nx, T ny, T nw, T nh) :
        x(std::move(nx)), y(std::move(ny)), w(std::move(nw)), h(std::move(nh)) {}
    T x;
    T y;
    T w;
    T h;
};
}

#endif // HG_GEOMETRY_H
