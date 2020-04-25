#ifndef HG_VULKAN_BUFFER_H
#define HG_VULKAN_BUFFER_H
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
#include <utility>
namespace hg {
    class VulkanBuffer final {
    public:
        explicit VulkanBuffer(VkDevice const device)
            : device(device), buffer(VK_NULL_HANDLE)
        {}
        explicit VulkanBuffer(
            VkDevice const device,
            VkBufferCreateInfo const &bufferInfo
        ) : device(device)
          , buffer([&]{
                VkBuffer b{VK_NULL_HANDLE};
                auto const res{vkCreateBuffer(device, &bufferInfo, nullptr, &b)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create vulkan buffer!"));
                }
                return b;
            }())
        {
        }
        VulkanBuffer(VulkanBuffer const&) = delete;
        VulkanBuffer(VulkanBuffer &&o) noexcept
            : device(o.device)
            , buffer(std::exchange(o.buffer, VkBuffer{VK_NULL_HANDLE}))
        {
        }
        VulkanBuffer &operator=(VulkanBuffer const&) = delete;
        VulkanBuffer &operator=(VulkanBuffer &&o) noexcept {
            std::swap(device, o.device);
            std::swap(buffer, o.buffer);
            return *this;
        }
        ~VulkanBuffer() noexcept {
            vkDestroyBuffer(device, buffer, nullptr);
        }
    private:
        VkDevice device;
    public:
        VkBuffer buffer;
    };
}
#endif // !HG_VULKAN_BUFFER_H
