#ifndef HG_VULKAN_EXCEPTIONS
#define HG_VULKAN_EXCEPTIONS
#include <vulkan/vulkan.h>
#include <system_error>
#include <type_traits>
namespace std {
    template<>
    struct is_error_code_enum<VkResult> : std::true_type {};
}

std::error_code make_error_code(VkResult const res);

#endif //HG_VULKAN_EXCEPTIONS
