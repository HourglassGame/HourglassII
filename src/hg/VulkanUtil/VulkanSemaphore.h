#ifndef HG_VULKANSEMAPHORE_H
#define HG_VULKANSEMAPHORE_H
#include "VulkanExceptions.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
#include <utility>
namespace hg {
    class VulkanSemaphore final {
    public:
        explicit VulkanSemaphore(
            VkDevice const device,
            VkSemaphoreCreateInfo const &semaphoreInfo
        ) : device(device)
          , semaphore(
                [&]{
                    VkSemaphore s{VK_NULL_HANDLE};
                    {
                        auto const res{vkCreateSemaphore(device, &semaphoreInfo, nullptr, &s)};
                        if (res != VK_SUCCESS) {
                            BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create semaphore!"));
                        }
                    }
                    return s;
                }()
            )
        {
        }
        VulkanSemaphore(VulkanSemaphore const&) = delete;
        VulkanSemaphore(VulkanSemaphore &&o) noexcept
            : device(o.device)
            , semaphore(std::exchange(o.semaphore, VkSemaphore{ VK_NULL_HANDLE }))
        {
        }
        VulkanSemaphore &operator=(VulkanSemaphore const&) = delete;
        VulkanSemaphore &operator=(VulkanSemaphore &&o) noexcept {
            std::swap(device, o.device);
            std::swap(semaphore, o.semaphore);
            return *this;
        }
        ~VulkanSemaphore() noexcept {
            vkDestroySemaphore(device, semaphore, nullptr);
        }
    private:
        VkDevice device;
    public:
        VkSemaphore semaphore;
    };
}
#endif // !HG_VULKANSEMAPHORE_H
