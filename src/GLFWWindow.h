#ifndef HG_GLFWWINDOW_H
#define HG_GLFWWINDOW_H

#include <GLFW/glfw3.h>
#include <iostream>

int GLFWWindow_Global_lastKey;

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

        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            // glfwGetWindowUserPointer would allow lastKey to be accessed as a member of GLFWWindow,
            //  however, Vulkan uses the Window User Pointer for its own purposes.
            //GLFWWindow *datas;
            //datas = (GLFWWindow *)glfwGetWindowUserPointer(window);
            if (action == GLFW_RELEASE) {
                if (key == GLFWWindow_Global_lastKey) {
                    GLFWWindow_Global_lastKey = 0;
                }
                return;
            }
            GLFWWindow_Global_lastKey = key;
        }

        bool hasLastKey()
        {
            return (GLFWWindow_Global_lastKey > 0);
        }

        int useLastKey()
        {
            int prevLastKey = GLFWWindow_Global_lastKey;
            GLFWWindow_Global_lastKey = 0;
            return prevLastKey;
        }
    };
}
#endif // !HG_GLFWWINDOW_H
