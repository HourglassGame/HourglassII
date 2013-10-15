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
    mt::std::vector<Glitz>::type const &forwardsGlitz,
    mt::std::vector<Glitz>::type const &reverseGlitz,
    mt::std::vector<GuyOutputInfo>::type const &guyInformation) :
        forwardsGlitz_(forwardsGlitz),
        reverseGlitz_(reverseGlitz),
        guyInformation_(guyInformation)
{
}

void FrameView::swap(FrameView &o)
{
    boost::swap(forwardsGlitz_, o.forwardsGlitz_);
    boost::swap(reverseGlitz_, o.reverseGlitz_);
    boost::swap(guyInformation_, o.guyInformation_);
}
}//namespace hg
