#ifndef HG_POSITION_H
#define HG_POSITION_H
namespace hg {
    namespace verticalposition {
        struct TOP {
            typedef ::hg::verticalposition::TOP SetCentreTagType;
            static float StartPosition(float /*menuHeight*/, float windowHeight)
            {
                //Start 10% down from the top
                return windowHeight * 0.1f;
            }
        };
        struct MIDDLE {
            typedef ::hg::verticalposition::MIDDLE SetCentreTagType;
            static float StartPosition(float menuHeight, float windowHeight)
            {
                //Start such that the menu ends up centered
                return (windowHeight - menuHeight)/2.f;
            }
        };
        struct BOTTOM {
            typedef ::hg::verticalposition::BOTTOM SetCentreTagType;
            static float StartPosition(float /*menuHeight*/, float windowHeight)
            {
                //Start 10% up from the bottom
                return windowHeight * 0.9f;
            }
        };
    }
    namespace horizontalposition {
        struct LEFT {
            typedef ::hg::horizontalposition::LEFT SetCentreTagType;
            static float StartPosition(float /*menuWidth*/, float windowWidth)
            {
                //Start 10% right from the left
                return windowWidth * 0.1f;
            }
        };
        struct MIDDLE {
            typedef ::hg::horizontalposition::MIDDLE SetCentreTagType;
            static float StartPosition(float menuWidth, float windowWidth)
            {
                //Start such that the menu ends up centered
                return (windowWidth - menuWidth)/2.f;
            }
        };
        struct RIGHT {
            typedef ::hg::horizontalposition::RIGHT SetCentreTagType;
            static float StartPosition(float /*menuWidth*/, float windowWidth)
            {
                //Start 10% up from the left from the right
                return windowWidth * 0.9f;
            }
        };
    }
}
#endif //HG_POSITION_H
