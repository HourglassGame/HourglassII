#ifndef HG_MULTI_ARRAY_H
#define HG_MULTI_ARRAY_H
#include <array>
namespace hg {
    namespace multi_array_detail {
        //template<typename T>
        //struct multi_array_def;
        template<typename T, std::size_t ...Sizes>
        struct multi_array_def;
        
        template<typename T, std::size_t N, std::size_t ...Sizes>
        struct multi_array_def<T, N, Sizes...> final {
            using type = std::array<typename multi_array_def<T, Sizes...>::type, N>;
        };
        
        template<typename T>
        struct multi_array_def<T> final {
            using type = T;
        };
    };

    //multi_array<T, 3, 5, 2> == std::array<std::array<std::array<T, 2>, 5>, 3>
    //multi_array<T> == T
    //Intent is to be similar to declaration syntax for builtin arrays: e.g. T[3][5][2].
    template<typename T, std::size_t ...Sizes>
    using multi_array = typename multi_array_detail::multi_array_def<T, Sizes...>::type;
}
#endif //HG_MULTI_ARRAY_H
