#include "Window.h"
namespace hg {
    Window::Window() :
            title(),
            windowStyle()
    {
    }
    Window::Window(::sf::VideoMode mode, 
            const ::std::string& ntitle,
            unsigned long nwindowStyle, 
            const ::sf::WindowSettings& params) :
             ::sf::RenderWindow(mode, ntitle, nwindowStyle, params),
            title(ntitle),
            windowStyle(nwindowStyle)
    {
    }
    void Window::Create(::sf::VideoMode mode, 
                const ::std::string& ntitle,
                unsigned long nwindowStyle,
                const ::sf::WindowSettings& params)
    {
        ::sf::RenderWindow::Create(mode, ntitle, nwindowStyle, params);
    }
}
