#ifndef HG_GLFWWINDOW_H
#define HG_GLFWWINDOW_H

#include <GLFW/glfw3.h>
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
    };
}
#endif // !HG_GLFWWINDOW_H
