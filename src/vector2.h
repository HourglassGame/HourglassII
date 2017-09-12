#ifndef HG_VECTOR2_H
#define HG_VECTOR2_H
namespace hg {
template<typename T>
struct vector2
{
    vector2(T nx, T ny) : x(nx), y(ny) {}
    T x;
    T y;
};
template<typename T>
bool operator==(vector2<T> const &l, vector2<T> const &r) {
    return l.x == r.x && r.y == r.y;
}
}
#endif //HG_VECTOR2_H
