#ifndef HG_VULKANFENCE_H
#define HG_VULKANFENCE_H
#include "VulkanExceptions.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
namespace hg {
    class VulkanFence final {
    public:
        explicit VulkanFence(
            VkDevice const device,
            VkFenceCreateInfo const &fenceInfo
        ) : device(device)
          , fence([&]{
                VkFence f{VK_NULL_HANDLE};
                {
                    auto const res{vkCreateFence(device, &fenceInfo, nullptr, &f)};
                    if (res != VK_SUCCESS) {
                        BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create fence!"));
                    }
                }
                return f;
            }())
        {
        }
        VulkanFence(VulkanFence const&) = delete;
        VulkanFence(VulkanFence &&o) noexcept
            : device(o.device)
            , fence(std::exchange(o.fence, VkFence{VK_NULL_HANDLE}))
        {
        }
        VulkanFence &operator=(VulkanFence const&) = delete;
        VulkanFence &operator=(VulkanFence &&o) noexcept {
            std::swap(device, o.device);
            std::swap(fence, o.fence);
            return *this;
        }
        ~VulkanFence() noexcept {
            vkDestroyFence(device, fence, nullptr);
        }
    private:
        VkDevice device;
    public:
        VkFence fence;
    };
}
#endif // !HG_VULKANFENCE_H
