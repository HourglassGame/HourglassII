#ifndef HG_RENDER_WINDOW_H
#define HG_RENDER_WINDOW_H
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>
#include <boost/container/vector.hpp>
#include <cassert>
namespace hg {
//Wraps sf::RenderWindow and adds some additional features.
struct RenderWindow {
    struct InputState {
        InputState(
            boost::container::vector<bool> const &pressedKeys,
            boost::container::vector<bool> const &pressedMouseButtons,
            sf::Vector2i mousePosition):
                pressedKeys(pressedKeys),
                pressedMouseButtons(pressedMouseButtons),
                mousePosition(mousePosition)
        {
            assert(this->pressedKeys.size() == sf::Keyboard::KeyCount);
            assert(this->pressedMouseButtons.size() == sf::Mouse::ButtonCount);
        }
        bool isKeyPressed(sf::Keyboard::Key keyCode) const {
            return pressedKeys[keyCode];
        }
        bool isMouseButtonPressed(sf::Mouse::Button button) const {
            return pressedMouseButtons[button];
        }
        sf::Vector2i getMousePosition() const {
            return mousePosition;
        }
    private:
        //Perhaps use a bit-vector
        //Purposefully avoiding std::vector<bool> as it is confusing, compared to something more explicit
        boost::container::vector<bool> pressedKeys;
        boost::container::vector<bool> pressedMouseButtons;
        sf::Vector2i mousePosition;
    };
    struct InputStateTracker {
        InputStateTracker(sf::RenderWindow const &window) :
            pressedKeys(sf::Keyboard::KeyCount),
            pressedMouseButtons(sf::Mouse::ButtonCount),
            mousePos(sf::Mouse::getPosition(window))
        {
        }
        void processEvent(sf::Event const &event) {
            switch (event.type) {
                case sf::Event::Closed:                 ///< The window requested to be closed (no data)
                case sf::Event::Resized:                ///< The window was resized (data in event.size)
                case sf::Event::LostFocus:              ///< The window lost the focus (no data)
                case sf::Event::GainedFocus:            ///< The window gained the focus (no data)
                case sf::Event::TextEntered:            ///< A character was entered (data in event.text)
                    //Nothing to do.
                break;
                case sf::Event::KeyPressed:             ///< A key was pressed (data in event.key)
                    if (0 <= event.key.code && (std::size_t)event.key.code < pressedKeys.size()) {
                        pressedKeys[event.key.code] = true;
                    }
                break;
                case sf::Event::KeyReleased:            ///< A key was released (data in event.key)
                    if (0 <= event.key.code && (std::size_t)event.key.code < pressedKeys.size()) {
                        pressedKeys[event.key.code] = false;
                    }
                break;
                case sf::Event::MouseWheelMoved:        ///< The mouse wheel was scrolled (data in event.mouseWheel)
                    //Nothing to do.
                break;
                case sf::Event::MouseButtonPressed:     ///< A mouse button was pressed (data in event.mouseButton)
                    if (0 <= event.mouseButton.button && (std::size_t)event.mouseButton.button < pressedMouseButtons.size()) {
                        pressedMouseButtons[event.mouseButton.button] = true;
                    }
                break;
                case sf::Event::MouseButtonReleased:    ///< A mouse button was released (data in event.mouseButton)
                    if (0 <= event.mouseButton.button && (std::size_t)event.mouseButton.button < pressedMouseButtons.size()) {
                        pressedMouseButtons[event.mouseButton.button] = false;
                    }
                break;
                case sf::Event::MouseMoved:             ///< The mouse cursor moved (data in event.mouseMove)
                    mousePos.x = event.mouseMove.x;
                    mousePos.y = event.mouseMove.y;
                break;
                case sf::Event::MouseEntered:           ///< The mouse cursor entered the area of the window (no data)
                case sf::Event::MouseLeft:              ///< The mouse cursor left the area of the window (no data)
                    //Nothing to do.
                break;
                case sf::Event::JoystickButtonPressed:  ///< A joystick button was pressed (data in event.joystickButton)
                case sf::Event::JoystickButtonReleased: ///< A joystick button was released (data in event.joystickButton)
                case sf::Event::JoystickMoved:          ///< The joystick moved along an axis (data in event.joystickMove)
                case sf::Event::JoystickConnected:      ///< A joystick was connected (data in event.joystickConnect)
                case sf::Event::JoystickDisconnected:   ///< A joystick was disconnected (data in event.joystickConnect)
                    //TODO implement joystick support
                break;
                case sf::Event::Count:                   ///< Keep last -- the total number of event types
                    assert(false && "sf::Event::Count is not a real event");
                break;
            }
        }
        boost::container::vector<bool> pressedKeys;
        boost::container::vector<bool> pressedMouseButtons;
        sf::Vector2i mousePos;
        InputState getInputState() const {
            return InputState(pressedKeys, pressedMouseButtons, mousePos);
        }
    };
    //Additional Features
    
    //Gets the state of the input at the time of call.
    InputState getInputState() const {
        return inputStateTracker.getInputState();
    }
    
    //sf::RenderWindow Interface:
    sf::RenderTarget &getRenderTarget() {
        return renderWindow;
    }
    
    sf::Window const &getWindow() const {
        return renderWindow;
    }
    
    
    RenderWindow() : renderWindow(), inputStateTracker(renderWindow) {}
    RenderWindow(
        sf::VideoMode mode,
        const sf::String &title,
        sf::Uint32 style = sf::Style::Default,
        const sf::ContextSettings &settings = sf::ContextSettings()) :
            renderWindow(mode, title, style, settings), inputStateTracker(renderWindow)
    {}

    explicit RenderWindow(sf::WindowHandle handle, const sf::ContextSettings &settings = sf::ContextSettings()) :
        renderWindow(handle, settings), inputStateTracker(renderWindow)
    {}

    //virtual ~Window();
    //Window Functions:
    void create(
        sf::VideoMode mode,
        const sf::String &title,
        sf::Uint32 style = sf::Style::Default,
        const sf::ContextSettings &settings = sf::ContextSettings())
    {
        renderWindow.create(mode, title, style, settings);
    }

    void create(sf::WindowHandle handle, const sf::ContextSettings &settings = sf::ContextSettings())
    {
        renderWindow.create(handle, settings);
    }

    void close() {
        renderWindow.close();
    }

    bool isOpen() const {
        return renderWindow.isOpen();
    }

    const sf::ContextSettings &getSettings() const {
        return renderWindow.getSettings();
    }

    bool pollEvent(sf::Event &event) {
        bool eventCaptured(renderWindow.pollEvent(event));
        if (eventCaptured) inputStateTracker.processEvent(event);
        return eventCaptured;
    }

    bool waitEvent(sf::Event &event) {
        bool eventCaptured(renderWindow.waitEvent(event));
        if (eventCaptured) inputStateTracker.processEvent(event);
        return eventCaptured;
    }

    sf::Vector2i getPosition() const {
        return renderWindow.getPosition();
    }

    void setPosition(const sf::Vector2i &position) {
        renderWindow.setPosition(position);
    }

    sf::Vector2u getSize() const {
        return renderWindow.getSize();
    }

    void setSize(const sf::Vector2u size) {
        renderWindow.setSize(size);
    }
    
    void setTitle(const sf::String &title) {
        renderWindow.setTitle(title);
    }

    void setIcon(unsigned int width, unsigned int height, sf::Uint8 const *pixels) {
        renderWindow.setIcon(width, height, pixels);
    }

    void setVisible(bool visible) {
        renderWindow.setVisible(visible);
    }

    void setVerticalSyncEnabled(bool enabled) {
        renderWindow.setVerticalSyncEnabled(enabled);
    }

    void setMouseCursorVisible(bool visible) {
        renderWindow.setMouseCursorVisible(visible);
    }

    void setKeyRepeatEnabled(bool enabled) {
        renderWindow.setKeyRepeatEnabled(enabled);
    }

    void setFramerateLimit(unsigned int limit) {
        renderWindow.setFramerateLimit(limit);
    }

    void setJoystickThreshold(float threshold) {
        renderWindow.setJoystickThreshold(threshold);
    }

   
    bool setActive(bool active = true) const {
        return renderWindow.setActive(active);
    }

    void display() {
        renderWindow.display();
    }


    sf::WindowHandle getSystemHandle() const {
        return renderWindow.getSystemHandle();
    }

    //sf::RenderWindow functions:
    //virtual sf::Vector2u getSize() const {
    //    return renderWindow.getSize();
    //}

    sf::Image capture() const {
        return renderWindow.capture();
    }
    
    //====== sf::RenderTarget functions ======
    void clear(const sf::Color &color = sf::Color(0, 0, 0, 255)) {
        return renderWindow.clear(color);
    }

    void setView(const sf::View &view) {
        renderWindow.setView(view);
    }

    const sf::View &getView() const {
        return renderWindow.getView();
    }

    const sf::View &getDefaultView() const {
        return renderWindow.getDefaultView();
    }

    sf::IntRect getViewport(const sf::View &view) const {
        return renderWindow.getViewport(view);
    }

    sf::Vector2f mapPixelToCoords(const sf::Vector2i &point) const {
        return renderWindow.mapPixelToCoords(point);
    }

    sf::Vector2f mapPixelToCoords(const sf::Vector2i &point, const sf::View &view) const {
        return renderWindow.mapPixelToCoords(point, view);
    }

    sf::Vector2i mapCoordsToPixel(const sf::Vector2f &point) const {
        return renderWindow.mapCoordsToPixel(point);
    }

    sf::Vector2i mapCoordsToPixel(const sf::Vector2f &point, const sf::View &view) const {
        return renderWindow.mapCoordsToPixel(point, view);
    }

    void draw(const sf::Drawable &drawable, const sf::RenderStates &states = sf::RenderStates::Default) {
        return renderWindow.draw(drawable, states);
    }

    void draw(sf::Vertex const*vertices, unsigned int vertexCount,
              sf::PrimitiveType type, const sf::RenderStates &states = sf::RenderStates::Default)
    {
        renderWindow.draw(vertices, vertexCount, type, states);
    }

    //virtual Vector2u getSize() const = 0;

    void pushGLStates() {
        renderWindow.pushGLStates();
    }

    void popGLStates() {
        renderWindow.popGLStates();
    }

    void resetGLStates() {
        renderWindow.resetGLStates();
    }
    
private:
    sf::RenderWindow renderWindow;
    InputStateTracker inputStateTracker;
};
}
#endif //HG_RENDER_WINDOW_H
