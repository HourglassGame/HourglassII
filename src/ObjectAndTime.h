#ifndef HG_OBJECT_AND_TIME
#define HG_OBJECT_AND_TIME
#include <tuple>
namespace hg {
    template<typename Object, typename FrameT>
    struct ObjectAndTime final
    {
    private:
        auto comparison_tuple() const -> decltype(auto)
        {
            return std::tie(
                object,
                frame
            );
        }
    public:
        ObjectAndTime(
            Object const &nobject,
            FrameT nframe) :
                object(nobject),
                frame(nframe) {}
        Object object;
        FrameT frame;
        bool operator==(ObjectAndTime const &o) const {
            return comparison_tuple() == o.comparison_tuple();
        }
    };
}
#endif //HG_OBJECT_AND_TIME
