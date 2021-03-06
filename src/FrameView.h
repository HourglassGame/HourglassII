#ifndef HG_FRAME_VIEW_H
#define HG_FRAME_VIEW_H
#include "mt/std/vector"
#include "Glitz.h"
#include "GuyOutputInfo.h"
namespace hg {
//TODO -- improve description.
//A InterfaceInput holds data that is passed from the physics engine to the frontend.
//Each "executeFrame()" returns one of these. The frontend can ask to be given the one
//returned by any frame that it wants to know about.
//This can hold essentially anything, but is expected to hold such things as
//positions and time-directions of objects, references to sound and graphical samples,
//player inventory contents (including the contents of the inventories of the other
//guys in the frame), and so on.
class FrameView final {
public:
    FrameView();
    FrameView(
        mt::std::vector<Glitz> &&forwardsGlitz,
        mt::std::vector<Glitz> &&reverseGlitz,
        mt::std::vector<GuyOutputInfo> &&guyInformation);
    
    void swap(FrameView &o);
    
    mt::std::vector<Glitz> const &getForwardsGlitz() const { return forwardsGlitz_; }
    mt::std::vector<Glitz> const &getReverseGlitz() const { return reverseGlitz_; }
    mt::std::vector<GuyOutputInfo> const &getGuyInformation() const { return guyInformation_; }
private:
    //The draw order (where relevant) is determined by the order in the list
    //(note-- on a LayeredCanvas, the actual order is also determined
    //by the layer of the Glitz).
    mt::std::vector<Glitz> forwardsGlitz_;
    mt::std::vector<Glitz> reverseGlitz_;
    
    //Filled with player positions, inventories, time-directions, indices etc
    //Could also have information about "pseudo-guys" who are
    //dead but still interesting to follow.
    //An example of a situation where this might be useful is:
    //A guy travels through a chrono/teleporter
    //On arrival he is killed by spikes
    //It is desireable for the camera to follow the guy through both space and time.
    //This can store the relevant space part.
    mt::std::vector<GuyOutputInfo> guyInformation_;
};
}//namespace hg
#endif //HG_FRAME_VIEW_H
