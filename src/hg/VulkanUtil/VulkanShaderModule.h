#ifndef HG_VULKANSHADERMODULE_H
#define HG_VULKANSHADERMODULE_H
#include "VulkanExceptions.h"
#include <vulkan/vulkan.h>
#include <vector>
namespace hg {

    class VulkanShaderModule final {
    public:
        VulkanShaderModule(
            VkDevice const device,
            std::vector<uint32_t> const& code
        ) : device(device)
        {
            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size()*sizeof(decltype(*code.data()));
            createInfo.pCode = code.data();
            {
                auto const res{vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create shader module!"));
                }
            }
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
        VkDevice device;
        VkShaderModule shaderModule;
    };
}
#endif // !HG_VULKANSHADERMODULE_H
