#ifndef HG_VULKAN_UPDATABLE_TEXTURE_SIMPLE_H
#define HG_VULKAN_UPDATABLE_TEXTURE_SIMPLE_H

#include "VulkanExceptions.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>

namespace hg{

    inline VkDescriptorSet createDescriptorSet2(
        VkDevice const device,
        VkDescriptorPool const samplerDescriptorPool,
        VkDescriptorSetLayout const textureDescriptorSetLayout,
        VkImageView const imageView,
        VkSampler const sampler)
    {
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = samplerDescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &textureDescriptorSetLayout;

        VkDescriptorSet descriptorSet;
        if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        //vkUpdateDescriptorSets
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageView;
        imageInfo.sampler = sampler;

        VkWriteDescriptorSet descriptorWrite = {};

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        return descriptorSet;
    }

    struct VulkanUpdatableTextureSimple final {
        explicit VulkanUpdatableTextureSimple(
            VkPhysicalDevice const physicalDevice,
            VkDevice const device,
            std::size_t const width_,
            std::size_t const height_,
            VkDescriptorPool const descriptorPool,
            VkDescriptorSetLayout const textureDescriptorSetLayout
        )
            : width(std::max<std::size_t>(width_, 1)), height(std::max<std::size_t>(height_, 1)), device(device), stagingMemory(device), stagingBuffer(device), transferSrcMappedRegion(device, stagingMemory.memory), imageMemory(device), image(device), imageView(device), sampler(device)
        {
            std::size_t imageSizeBytes{ width * height * 4 };
            //Allocate Staging Buffer/Memory; Host Visible
            VkBufferCreateInfo stagingBufferInfo = {};
            stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingBufferInfo.size = imageSizeBytes;
            stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            stagingBuffer = VulkanBuffer(device, stagingBufferInfo);

            VkMemoryRequirements stagingMemoryRequirements;
            vkGetBufferMemoryRequirements(device, stagingBuffer.buffer, &stagingMemoryRequirements);

            VkMemoryAllocateInfo stagingMemoryAllocInfo = {};
            stagingMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            stagingMemoryAllocInfo.allocationSize = stagingMemoryRequirements.size;
            stagingMemoryAllocInfo.memoryTypeIndex = findMemoryType(physicalDevice, stagingMemoryRequirements.size, stagingMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

            stagingMemory = VulkanMemory{ device, stagingMemoryAllocInfo };

            vkBindBufferMemory(device, stagingBuffer.buffer, stagingMemory.memory, 0);


            //Map Staging Buffer Memory
            transferSrcMappedRegion = VulkanMappedMemory{
                device,
                stagingMemory.memory,
                0,//offset
                stagingBufferInfo.size,
                0//flags
            };

            //Allocate Image/Memory; device local
            VkImageCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = gsl::narrow<uint32_t>(width);
            imageInfo.extent.height = gsl::narrow<uint32_t>(height);
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            image = VulkanImage(device, imageInfo);

            VkMemoryRequirements imageMemRequirements;
            vkGetImageMemoryRequirements(device, image.image, &imageMemRequirements);

            VkMemoryAllocateInfo imageMemoryAllocInfo = {};
            imageMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            imageMemoryAllocInfo.allocationSize = imageMemRequirements.size;
            imageMemoryAllocInfo.memoryTypeIndex = findMemoryType(physicalDevice, imageMemRequirements.size, imageMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


            imageMemory = VulkanMemory(device, imageMemoryAllocInfo);
            {
                auto const res{ vkBindImageMemory(device, image.image, imageMemory.memory, 0) };
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "vkBindImageMemory failed"));
                }
            }

            //Make ImageView
            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image.image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            imageView = VulkanImageView{ device, viewInfo };
            //MakeSampler
            VkSamplerCreateInfo samplerInfo = {};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.anisotropyEnable = VK_FALSE;
            samplerInfo.maxAnisotropy = 0;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

            sampler = VulkanSampler{ device, samplerInfo };
            //MakeDescriptorSet
            descriptorSet = createDescriptorSet2(device, descriptorPool, textureDescriptorSetLayout, imageView.imageView, sampler.sampler);
        }

        void flushTextureChanges(VkCommandBuffer const preDrawCommandBuffer) {
            //Flush mapped memory
            std::array<VkMappedMemoryRange, 1> const memoryRanges{
                VkMappedMemoryRange{
                VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                nullptr, //pNext
                transferSrcMappedRegion.memory,
                0,//offset
                VK_WHOLE_SIZE//size
            }
            };
            {
                auto const res{ vkFlushMappedMemoryRanges(device, gsl::narrow<uint32_t>(memoryRanges.size()), memoryRanges.data()) };
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "vkFlushMappedMemoryRanges failed"));
                }
            }
            //vkCmdPipelineBarrier to transition to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                //With the exception of the first time that this is called,
                //we know that the old layout is
                //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
                //TODO: Take advantage of this!
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = image.image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;

                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                vkCmdPipelineBarrier(
                    preDrawCommandBuffer,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );
            }
            //vkCmdCopyBufferToImage
            {
                VkBufferImageCopy region = {};
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = 0;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = 1;
                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height),
                    1
                };

                vkCmdCopyBufferToImage(preDrawCommandBuffer, stagingBuffer.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            }
            //vkCmdPipelineBarrier to transition to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = image.image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(
                    preDrawCommandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );
            }
        }
        std::size_t width;
        std::size_t height;
        VkDevice device;
        VulkanMemory stagingMemory;
        VulkanBuffer stagingBuffer;
        VulkanMappedMemory transferSrcMappedRegion;
        VulkanMemory imageMemory;
        VulkanImage image;
        VulkanImageView imageView;
        VulkanSampler sampler;
        VkDescriptorSet descriptorSet;//?
    };

}
#endif //!HG_VULKAN_UPDATABLE_TEXTURE_SIMPLE_H
