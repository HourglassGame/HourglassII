#ifndef HG_VULKANSURFACE_H
#define HG_VULKANSURFACE_H
#include "VulkanExceptions.h"
#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>
namespace hg {

    class VulkanSurface final {

    public:
        VulkanSurface(
            VkInstance const i,
            GLFWwindow * const window
        )
            : i(i)
        {
            auto const res{glfwCreateWindowSurface(i, window, nullptr, &surface)};
            if (res != VK_SUCCESS) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create window surface!"));
            }
        }
        VulkanSurface(VulkanSurface const&) = delete;
        VulkanSurface(VulkanSurface &&) = delete;
        VulkanSurface &operator=(VulkanSurface const&) = delete;
        VulkanSurface &operator=(VulkanSurface &&) = delete;
        ~VulkanSurface() noexcept {
            vkDestroySurfaceKHR(i, surface, nullptr);
        }
        VkInstance i;
        VkSurfaceKHR surface;
    };
}
#endif // !HG_VULKANSURFACE_H
