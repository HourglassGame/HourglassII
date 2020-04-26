#ifndef HG_VULKANSHADERMODULEHG_H
#define HG_VULKANSHADERMODULEHG_H
#include "hg/VulkanUtil/VulkanShaderModule.h"
#include <vulkan/vulkan.h>
#include <vector>
namespace hg {

    class VulkanShaderModuleHG final {
    public:
        explicit VulkanShaderModuleHG(
            VkDevice const device,
            std::vector<uint32_t> const &code
        ) : shaderModule (
                device,
                [&]{
                    VkShaderModuleCreateInfo createInfo = {};
                    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                    createInfo.codeSize = code.size()*sizeof(decltype(*code.data()));
                    createInfo.pCode = code.data();
                    return createInfo;
                }()
            )
        {
        }
        VulkanShaderModuleHG(VulkanShaderModuleHG const&) = delete;
        VulkanShaderModuleHG(VulkanShaderModuleHG &&o) noexcept = default;
        VulkanShaderModuleHG &operator=(VulkanShaderModuleHG const&) = delete;
        VulkanShaderModuleHG &operator=(VulkanShaderModuleHG &&o) noexcept = default;

        VkShaderModule h() const {
            return shaderModule.shaderModule;
        }
    private:
        VulkanShaderModule shaderModule;
    };
}
#endif // !HG_VULKANSHADERMODULEHG_H
