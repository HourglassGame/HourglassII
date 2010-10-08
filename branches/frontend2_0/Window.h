#ifndef HG_WINDOW_H
#define HG_WINDOW_H
#include <SFML/Graphics/RenderWindow.hpp>
namespace hg {
//Wrapper around a sf::RenderWindow which keeps track of the window style and other useful information which is not kept by SFML
//The public inheritance here is an implementations detail; I couldn't be bothered writing out wrappers for all the functions
    class Window : public ::sf::RenderWindow {
        public:
        ::std::string title;
        unsigned long windowStyle;
        
        Window();
        Window(::sf::VideoMode mode, 
                const ::std::string& ntitle,
                unsigned long nwindowStyle=sf::Style::Resize|::sf::Style::Close, 
                const ::sf::WindowSettings& params=::sf::WindowSettings());
        void Create(::sf::VideoMode mode, 
                const ::std::string& ntitle,
                unsigned long nwindowStyle=sf::Style::Resize|::sf::Style::Close, 
                const ::sf::WindowSettings& params=::sf::WindowSettings());
    };
}
#endif //HG_WINDOW_H
