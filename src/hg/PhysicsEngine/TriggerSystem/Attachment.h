#ifndef HG_ATTACHMENT_H
#define HG_ATTACHMENT_H
#include <vector>
#include <limits>
namespace hg
{
struct Attachment final
{
    explicit Attachment() :
            platformIndex(std::numeric_limits<std::size_t>::max()),
            xOffset(0),
            yOffset(0)
    {
    }
    explicit Attachment(std::size_t nPlatformIndex,
               int nxOffset,
               int nyOffset) :
            platformIndex(nPlatformIndex),
            xOffset(nxOffset),
            yOffset(nyOffset)
    {
    }
    //index of platform to which thing is attached
    std::size_t platformIndex;
    //offset to apply to platform position to get x position of attached thing
    int xOffset;
    //offset to apply to platform position to get y position of attached thing
    int yOffset;
};
}
#endif //HG_ATTACHMENT_H
