#ifndef HG_GLFWAPP_H
#define HG_GLFWAPP_H

#include <GLFW/glfw3.h>
namespace hg{
    class GLFWApp final {
    public:
        GLFWApp() {
            if (!glfwInit()) throw std::exception("glfwInit failed");
        }
        GLFWApp(GLFWApp const&) = delete;
        GLFWApp(GLFWApp &&) = delete;
        GLFWApp &operator=(GLFWApp const&) = delete;
        GLFWApp &operator=(GLFWApp &&) = delete;
        ~GLFWApp() noexcept {
            glfwTerminate();
        }
    };
}
#endif // !HG_GLFWAPP_H
