#ifndef HG_OBJECT_AND_TIME
#define HG_OBJECT_AND_TIME
#include <tuple>
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
		bool operator==(ObjectAndTime const& o) const = default;
	};
}
#endif //HG_OBJECT_AND_TIME
