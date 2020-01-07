#ifndef HG_INPUT_HANDLER_H
#define HG_INPUT_HANDLER_H

#include "GLFWwindow.h"
#include <iostream>

namespace hg {
class InputHandler {

public:
    static int lastKey;
    static int prevLastKey;

    static int GetLastKey()
    {
        return lastKey;
    }

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        std::cout << "Key: " << key << "\n" << std::flush;
    }

    static void SetWindow(GLFWwindow *windowglfww)
    {
        glfwSetKeyCallback(windowglfww, key_callback);
    }

    static int UseLastKey()
    {
        prevLastKey = lastKey;
        lastKey = 0;
        return prevLastKey;
    }
};

}
#endif //HG_INPUT_HANDLER_H