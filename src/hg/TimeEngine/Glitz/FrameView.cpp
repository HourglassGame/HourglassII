#include "FrameView.h"
#include <boost/swap.hpp>
namespace hg {
FrameView::FrameView() :
	forwardsGlitz_(),
	reverseGlitz_(),
	guyInformation_()
{
}
FrameView::FrameView(
	mt::std::vector<Glitz> &&forwardsGlitz,
	mt::std::vector<Glitz> &&reverseGlitz,
	mt::std::vector<GuyOutputInfo> &&guyInformation) :
		forwardsGlitz_(std::move(forwardsGlitz)),
		reverseGlitz_(std::move(reverseGlitz)),
		guyInformation_(std::move(guyInformation))
{
}

void FrameView::swap(FrameView &o)
{
	boost::swap(forwardsGlitz_, o.forwardsGlitz_);
	boost::swap(reverseGlitz_, o.reverseGlitz_);
	boost::swap(guyInformation_, o.guyInformation_);
}
}//namespace hg
