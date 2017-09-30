#ifndef HG_OBJECT_AND_TIME
#define HG_OBJECT_AND_TIME
namespace hg {
    template<typename Object, typename FrameT>
    struct ObjectAndTime final
    {
        ObjectAndTime(
            Object const &nobject,
            FrameT nframe) :
                object(nobject),
                frame(nframe) {}
        Object object;
        FrameT frame;
    };
}
#endif //HG_OBJECT_AND_TIME
