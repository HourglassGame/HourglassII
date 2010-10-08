#include "Utility.h"
#include "Position.h"
#include "Options.h"
#include "Window.h"
#include <SFML/Graphics/String.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace hg {

    void initialiseNormalWindow(Window& window, const Options& options)
    {
        //set icon? Want to do/test that on windows, because SFML does not actually set the icon on OSX
        if (window.GetWidth() != options.width || 
            window.GetHeight() != options.height || 
            window.windowStyle != options.windowStyle) {
            if (options.windowStyle & sf::Style::Fullscreen) {
                window.Create(::sf::VideoMode::GetDesktopMode(),"Hourglass II", options.windowStyle);
            }
            else {
                window.Create(::sf::VideoMode(options.width, options.height),"Hourglass II", options.windowStyle);
            }
        }
    }

//perhaps make these template specialisations of a static member function (is that possible) 
//to improve inlining and help the compiler/code eliminate the tag objects
    float centrePosition(const ::sf::String& /*toSetCentre*/, horizontalposition::LEFT)
    {
        return 0.f;
    }
    float centrePosition(const ::sf::Sprite& /*toSetCentre*/, horizontalposition::LEFT)
    {
        return 0.f;
    }
    float centrePosition(const ::sf::String& toSetCentre, horizontalposition::MIDDLE)
    {
        return toSetCentre.GetRect().GetWidth()/2.f;
    }
    float centrePosition(const ::sf::Sprite& toSetCentre, horizontalposition::MIDDLE)
    {
        return toSetCentre.GetSubRect().GetWidth()/2.f;
    }
    float centrePosition(const ::sf::String& toSetCentre, horizontalposition::RIGHT)
    {
        return toSetCentre.GetRect().GetWidth();
    }
    float centrePosition(const ::sf::Sprite& toSetCentre, horizontalposition::RIGHT)
    {
        return toSetCentre.GetSubRect().GetWidth();
    }
    float centrePosition(const ::sf::String& /*toSetCentre*/, verticalposition::TOP)
    {
        return 0.f;
    }
    float centrePosition(const ::sf::Sprite& /*toSetCentre*/, verticalposition::TOP)
    {
        return 0.f;
    }
    float centrePosition(const ::sf::String& toSetCentre, verticalposition::MIDDLE)
    {
        return toSetCentre.GetRect().GetHeight()/2.f;
    }
    float centrePosition(const ::sf::Sprite& toSetCentre, verticalposition::MIDDLE)
    {
        return toSetCentre.GetSubRect().GetHeight()/2.f;
    }
    float centrePosition(const ::sf::String& toSetCentre, verticalposition::BOTTOM)
    {
        return toSetCentre.GetRect().GetHeight();
    }
    float centrePosition(const ::sf::Sprite& toSetCentre, verticalposition::BOTTOM)
    {
        return toSetCentre.GetSubRect().GetHeight();
    }
    
    void setCentre(::sf::String& toSetCentre, float x, float y)
    {
        toSetCentre.SetCenter(toSetCentre.GetRect().GetWidth()*x, toSetCentre.GetRect().GetHeight()*y);
    }
    void setCentre(::sf::Sprite& toSetCentre, float x, float y)
    {
         toSetCentre.SetCenter(toSetCentre.GetSubRect().GetWidth()*x,toSetCentre.GetSubRect().GetHeight()*y);
    }

    float getXSize(const ::sf::String& toGetXSize)
    {
        return toGetXSize.GetRect().GetWidth();
    }
    float getYSize(const ::sf::String& toGetYSize)
    {
        return toGetYSize.GetRect().GetHeight();
    }
    float getXSize(const ::sf::Sprite& toGetXSize)
    {
        return toGetXSize.GetSubRect().GetWidth() * toGetXSize.GetScale().x;
    }
    float getYSize(const ::sf::Sprite& toGetYSize)
    {
        return toGetYSize.GetSubRect().GetHeight() * toGetYSize.GetScale().y;
    }
}

