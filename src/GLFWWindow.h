#ifndef HG_GLFWWINDOW_H
#define HG_GLFWWINDOW_H

#include <GLFW/glfw3.h>
#include <iostream>

namespace hg {
    class GLFWWindow final {
    public:
        GLFWWindow(
            int const width,
            int const height,
            const char * const title,
            GLFWmonitor * const monitor,
            GLFWwindow * const share
            ) :
            w(glfwCreateWindow(width, height, title, monitor, share))
        {
            if(!w) throw std::exception("glfwCreateWindow failed");
        }
        GLFWWindow(GLFWWindow const&) = delete;
        GLFWWindow(GLFWWindow &&) = delete;
        GLFWWindow &operator=(GLFWWindow const&) = delete;
        GLFWWindow &operator=(GLFWWindow &&) = delete;
        ~GLFWWindow() noexcept {
            glfwDestroyWindow(w);
        }
        GLFWwindow *w;

        int lastKey;
        int prevLastKey;

        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            std::cout << "Key: " << key << "\n" << std::flush;
            GLFWWindow *datas;
            datas = (GLFWWindow *)glfwGetWindowUserPointer(window);
            if (key == GLFW_RELEASE) {
                if (key == datas->lastKey) {
                    datas->lastKey = 0;
                }
                return;
            }
            datas->lastKey = key;
        }

        int HasLastKey()
        {
            return lastKey;
        }

        int UseLastKey()
        {
            prevLastKey = lastKey;
            lastKey = 0;
            return prevLastKey;
        }
    };
}
#endif // !HG_GLFWWINDOW_H
