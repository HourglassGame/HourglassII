#ifndef HG_INTEGER_SEQUENCE_H
#define HG_INTEGER_SEQUENCE_H
#include <cstddef>
namespace hg {
    template<class T, T... Ints>
    struct integer_sequence {
        typedef T value_type;
        static_assert( std::is_integral<T>::value,
                      "hg::integer_sequence can only be instantiated with an integral type" );
        static constexpr std::size_t size() noexcept { return sizeof... (Ints); }
    };

    namespace integer_sequence_detail {
        template<std::size_t... Ints> struct seq {
            template<typename T>
            struct apply {
                typedef ::hg::integer_sequence<T, Ints...> type;
            };
        };

        template<class S1, class S2> struct concat;

        template<std::size_t... I1, std::size_t... I2>
        struct concat<seq<I1...>, seq<I2...>>
          : seq<I1..., (sizeof...(I1)+I2)...>{};

        template<std::size_t N> struct gen_seq;

        template<std::size_t N>
        struct gen_seq : concat<gen_seq<N/2>, gen_seq<N - N/2>>{};

        template<> struct gen_seq<0> : seq<>{};
        template<> struct gen_seq<1> : seq<0>{};
        
        template<typename T, T N>
        struct make_integer_sequence {
            static_assert(std::is_integral<T>::value,
                  "hg::make_integer_sequence can only be instantiated with an integral type");
            static_assert(0 <= N, "hg::make_integer_sequence input shall not be negative");
            typedef typename gen_seq<N>::template apply<T>::type type;
        };
    }
    
    
    template<std::size_t... Ints>
    using index_sequence = hg::integer_sequence<std::size_t, Ints...>;

    template<class T, T N>
    using make_integer_sequence = typename integer_sequence_detail::make_integer_sequence<T, N>::type;

    template<std::size_t N>
    using make_index_sequence = make_integer_sequence<std::size_t, N>;
    
    template<class... T>
    using index_sequence_for = make_index_sequence<sizeof...(T)>;
}
#endif //HG_INTEGER_SEQUENCE_H
