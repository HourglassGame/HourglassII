#ifndef HG_VULKANSHADERMODULE_H
#define HG_VULKANSHADERMODULE_H
#include "VulkanExceptions.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
#include <utility>
namespace hg {

    class VulkanShaderModule final {
    public:
        explicit VulkanShaderModule(
            VkDevice const device,
            VkShaderModuleCreateInfo const &createInfo
        ) : device(device)
          , shaderModule([&]{
                VkShaderModule sm{VK_NULL_HANDLE};
                {
                    auto const res{vkCreateShaderModule(device, &createInfo, nullptr, &sm)};
                    if (res != VK_SUCCESS) {
                        BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create shader module!"));
                    }
                }
                return sm;
            }())
        {
        }
        VulkanShaderModule(VulkanShaderModule const&) = delete;
        VulkanShaderModule(VulkanShaderModule &&o) noexcept
            : device(o.device)
            , shaderModule(std::exchange(o.shaderModule, VkShaderModule{VK_NULL_HANDLE}))
        {
        }
        VulkanShaderModule &operator=(VulkanShaderModule const&) = delete;
        VulkanShaderModule &operator=(VulkanShaderModule &&o) noexcept {
            std::swap(device, o.device);
            std::swap(shaderModule, o.shaderModule);
            return *this;
        }
        ~VulkanShaderModule() noexcept {
            vkDestroyShaderModule(device, shaderModule, nullptr);
        }
    private:
        VkDevice device;
    public:
        VkShaderModule shaderModule;
    };
}
#endif // !HG_VULKANSHADERMODULE_H
