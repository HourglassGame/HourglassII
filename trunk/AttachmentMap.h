#ifndef HG_ATTACHMENT_MAP_H
#define HG_ATTACHMENT_MAP_H
#include <vector>
#include <boost/tuple/tuple.hpp>
namespace hg
{
    class AttachmentMap
    {
    public:
        AttachmentMap(::std::vector< ::boost::tuple<int, int, int> > newButtonAttachments,
                      ::std::vector< ::boost::tuple<int, int, int> > newPortalAttachments) :
        buttonAttachments(newButtonAttachments),
        portalAttachments(newPortalAttachments)
        {
        }

        const ::std::vector< ::boost::tuple<int, int, int> >& getButtonAttachmentRef() const {return buttonAttachments;}
        const ::std::vector< ::boost::tuple<int, int, int> >& getPortalAttachmentRef() const {return portalAttachments;}

    private:
        ::std::vector< ::boost::tuple<int, int, int> > buttonAttachments;
        ::std::vector< ::boost::tuple<int, int, int> > portalAttachments;
    };
}
#endif //HG_ATTACHMENT_MAP_H
