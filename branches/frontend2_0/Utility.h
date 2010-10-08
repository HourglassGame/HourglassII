#ifndef HG_UTILITY_H
#define HG_UTILITY_H
#include <SFML/Graphics/RenderTarget.hpp>

/** a whole bunch of useful non-member functions
    sitting here until I think of a better place to put each of them
*/
namespace sf {
    class Sprite;
    class String;
}
namespace hg {
    class Options;
    class Window;
    namespace verticalposition {
        struct TOP;
        struct MIDDLE;
        struct BOTTOM;
    }
    namespace horizontalposition {
        struct LEFT;
        struct MIDDLE;
        struct RIGHT;
    }

    void initialiseNormalWindow(Window& window, const Options& options);

    float centrePosition(const ::sf::String& toSetCentre, horizontalposition::LEFT);
    float centrePosition(const ::sf::Sprite& toSetCentre, horizontalposition::LEFT);
    float centrePosition(const ::sf::String& toSetCentre, horizontalposition::MIDDLE);
    float centrePosition(const ::sf::Sprite& toSetCentre, horizontalposition::MIDDLE);
    float centrePosition(const ::sf::String& toSetCentre, horizontalposition::RIGHT);
    float centrePosition(const ::sf::Sprite& toSetCentre, horizontalposition::RIGHT);
    
    float centrePosition(const ::sf::String& toSetCentre, verticalposition::TOP);
    float centrePosition(const ::sf::Sprite& toSetCentre, verticalposition::TOP);
    float centrePosition(const ::sf::String& toSetCentre, verticalposition::MIDDLE);
    float centrePosition(const ::sf::Sprite& toSetCentre, verticalposition::MIDDLE);
    float centrePosition(const ::sf::String& toSetCentre, verticalposition::BOTTOM);
    float centrePosition(const ::sf::Sprite& toSetCentre, verticalposition::BOTTOM);
    
    
/** have to write explicit overloads for many of these because SFML does not have any sort
 of consistent interface between its classes */
//Not really much point making them templates at this stage, but it makes me feel happy -_-  
//-- and allows the design to change easily
    template <typename ToSetCentre, typename HorizontalPositionTag, typename VerticalPositionTag>
    void setCentre (ToSetCentre& toSetCentre, HorizontalPositionTag ht, VerticalPositionTag vt) {
        toSetCentre.SetCenter(centrePosition(toSetCentre, ht),centrePosition(toSetCentre, vt));
    }
    
    void setCentre(::sf::String& toSetCentre, float x, float y);
    void setCentre(::sf::Sprite& toSetCentre, float x, float y);

    float getXSize(const ::sf::String& toGetXSize);
    float getYSize(const ::sf::String& toGetYSize);
    float getXSize(const ::sf::Sprite& toGetXSize);
    float getYSize(const ::sf::Sprite& toGetYSize);
    
    template <class Drawable>
    void setXPosition(Drawable& toSetXPosition, int newXPosition)
    {
        toSetXPosition.SetX(newXPosition);
    }
    
    template <class Drawable>
    void setYPosition(Drawable& toSetYPosition, int newYPosition)
    {
        toSetYPosition.SetY(newYPosition);
    }
    
    template <class Drawable>
    void scale(Drawable& toScale, float scaleFactor)
    {
        toScale.Scale(scaleFactor, scaleFactor);
    }
    
    template <class Drawable>
    void draw(::sf::RenderTarget& target, const Drawable& toDraw)
    {
       target.Draw(toDraw);
    }
}
#endif //HG_UTILITY_H
