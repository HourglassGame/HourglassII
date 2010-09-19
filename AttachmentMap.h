#ifndef HG_ATTACHMENT_MAP_H
#define HG_ATTACHMENT_MAP_H
#include <vector>
#include <boost/tuple/tuple.hpp>
namespace hg
{
    class AttachmentMap
    {
    public:
        AttachmentMap(::std::vector< ::boost::tuple<int, int, int> > newButtonAttachments) :
        buttonAttachments(newButtonAttachments)
        {
        }

        const ::std::vector< ::boost::tuple<int, int, int> >& getButtonAttachmentRef() const {return buttonAttachments;}

    private:
        ::std::vector< ::boost::tuple<int, int, int> > buttonAttachments;
    };
}
#endif //HG_ATTACHMENT_MAP_H
