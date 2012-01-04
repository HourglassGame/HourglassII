#ifndef HG_GEOMETRY_H
#define HG_GEOMETRY_H

namespace hg {
template<typename T>
struct Vector2{
    Vector2() :
        x(), y() {}
    
    Vector2(T nx, T ny) :
        x(nx), y(ny) {}
    T x;
    T y;
};

template<typename T>
struct Rect{
    
    Rect() :
        x(), y(), w(), h() {}
    
    Rect(T nx, T ny, T nw, T nh) :
        x(nx), y(ny), w(nw), h(nh) {}
    T x;
    T y;
    T w;
    T h;
};
}

#endif // HG_GEOMETRY_H
