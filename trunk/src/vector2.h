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
}
#endif //HG_VECTOR2_H
