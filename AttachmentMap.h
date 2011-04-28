#ifndef HG_ATTACHMENT_MAP_H
#define HG_ATTACHMENT_MAP_H
#include <vector>
#include <limits>
namespace hg
{
    struct Attachment
    {
        Attachment() :
        platformIndex(std::numeric_limits<std::size_t>::max()),
        xOffset(0),
        yOffset(0)
        {
        }
        Attachment(std::size_t nPlatformIndex,
                   int nxOffset,
                   int nyOffset) :
        platformIndex(nPlatformIndex),
        xOffset(nxOffset),
        yOffset(nyOffset)
        {
        }
        std::size_t platformIndex;
        int xOffset;
        int yOffset;
    };
    
    class AttachmentMap
    {
    public:
        AttachmentMap(const std::vector<Attachment>& newButtonAttachments,
                      const std::vector<Attachment>& newPortalAttachments) :
        buttonAttachments(newButtonAttachments),
        portalAttachments(newPortalAttachments)
        {
        }

        const std::vector<Attachment>& getButtonAttachmentRef() const 
        {
            return buttonAttachments;
        }
        const std::vector<Attachment>& getPortalAttachmentRef() const
        {
            return portalAttachments;
        }

    private:
        //Lists of attachments:
        //buttonAttachments[n] contains the Attachment information for the button with index n
        //Attachment information is:
        //platformIndex -- index of platform to which button is attached
        //(x/y)Offset --  offset to apply to platform position to get button position
        std::vector<Attachment> buttonAttachments;
        //See above -- portalAttachments are the same.
        std::vector<Attachment> portalAttachments;
    };
}
#endif //HG_ATTACHMENT_MAP_H
