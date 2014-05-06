#ifndef HG_VARIANT_H
#define HG_VARIANT_H

//TODO:
// Strict aliasing compatibility
// Better compile-time checks (eg, ensure types are unique)
// Recursive variant
// Visitor return value
// Optimisation for no-throw ctors
// Reference members (?)
// Better namespace safety (mostly done now)
// Clean up implementation

#include <type_traits>
#include <cassert>
#include <tuple>
#include <utility>
#include <cstring>
#include <limits>
namespace hg {

template <typename T>
struct tag {};

namespace variant_detail {
template <typename... T>
struct max_align;

template<typename T>
struct max_align<T>
    : std::integral_constant<std::size_t, alignof (T)>
{};

template <typename T, typename... Tail>
struct max_align<T, Tail...>
    : std::integral_constant<
        std::size_t,
        (max_align<T>::value > max_align<Tail...>::value ? max_align<T>::value : max_align<Tail...>::value)>
{
};

template <typename... T>
struct max_size;

template<typename T>
struct max_size<T>
    : std::integral_constant<std::size_t, sizeof (T)>
{};

template <typename T, typename... Tail>
struct max_size<T, Tail...>
    : std::integral_constant<
        std::size_t,
        (max_size<T>::value > max_size<Tail...>::value ? max_size<T>::value : max_size<Tail...>::value)>
{
};

template<size_t N, typename... Types>
struct NthType {
    typedef typename std::tuple_element<N, std::tuple<Types...>>::type type;
};

template<std::size_t Index, typename T, typename... Types>
struct IndexOfAux;

template<bool check>
struct IndexOfCheck {
    static_assert(check, "T not found in Types...");
};

template<std::size_t Index, typename T, typename Type>
struct IndexOfAux<Index, T, Type>
    : IndexOfCheck<std::is_same<T, Type>::value>,
      std::integral_constant<std::size_t, Index> {};

template<std::size_t Index, typename T, typename Head, typename... Types>
struct IndexOfAux<Index, T, Head, Types...> :
    std::conditional<
        std::is_same<T, Head>::value,
        std::integral_constant<std::size_t, Index>,
        IndexOfAux<Index+1, T, Types...>>::type {};

template<typename T, typename... Types>
struct IndexOf : IndexOfAux<0, T, Types...> {};

template <typename T>
struct is_tag;

template <typename T>
struct is_tag<tag<T>> : std::true_type {};
template <typename T>
struct is_tag : std::false_type {};

template<typename... Types>
struct DtorVisitor;
template<typename Head> struct DtorVisitor<Head>{
    typedef void result_type;
    void operator()(Head &h) const {
        static_assert(noexcept(h.~Head()), "Types used in Variant must have no-throw dtor");
        h.~Head();
    }
};
template<typename Head, typename... Types>
struct DtorVisitor<Head, Types...> : DtorVisitor<Types...> {
    using DtorVisitor<Types...>::operator();
    void operator()(Head &h) const {
        static_assert(noexcept(h.~Head()), "Types used in Variant must have no-throw dtor");
        h.~Head();
    }
};

template<typename... Types>
struct CopyVisitor;

template<typename Variant, typename Head> struct CopyVisitor<Variant, Head> {
    typedef void result_type;
    Variant const *o;
    CopyVisitor(Variant const &o) : o(&o) {}
    void operator()(Head const &h) const {
        new ((void*)&h) Head(o->template get<Head>());
    }
};
template<typename Variant, typename Head, typename... Types>
struct CopyVisitor<Variant, Head, Types...> : CopyVisitor<Variant, Types...> {
    CopyVisitor(Variant const &o) : CopyVisitor<Variant, Types...>(o) {}
    using CopyVisitor<Variant, Types...>::operator();
    using CopyVisitor<Variant, Types...>::o;
    void operator()(Head const &h) const {
        new ((void*)&h) Head(o->template get<Head>());
    }
};

template<typename... Types>
struct MoveVisitor;

template<typename Variant, typename Head> struct MoveVisitor<Variant, Head> {
    typedef void result_type;
    Variant *o;
    MoveVisitor(Variant &&o) : o(&o) {}
    void operator()(Head &h) const {
        static_assert(noexcept(new ((void*)&h) Head(std::move(o->template get<Head>()))),
                      "Types used in Variant must have no-throw move constructor");
        new ((void*)&h) Head(std::move(o->template get<Head>()));
    }
};
template<typename Variant, typename Head, typename... Types>
struct MoveVisitor<Variant, Head, Types...> : MoveVisitor<Variant, Types...> {
    MoveVisitor(Variant&& o) : MoveVisitor<Variant, Types...>(std::move(o)) {}
    using MoveVisitor<Variant, Types...>::operator();
    using MoveVisitor<Variant, Types...>::o;
    void operator()(Head& h) const {
        static_assert(noexcept(new ((void*)&h) Head(std::move(o->template get<Head>()))),
                      "Types used in Variant must have no-throw move constructor");
        new ((void*)&h) Head(std::move(o->template get<Head>()));
    }
};

template<std::size_t N, typename... Types>
struct UnionTagType_aux;

template<std::size_t N, typename Head>
struct UnionTagType_aux<N, Head> {
    static_assert((std::numeric_limits<Head>::max() >= N), "Head must be able to fit N");
    typedef Head type;
};

template<std::size_t N, typename Head, typename... Types>
struct UnionTagType_aux<N, Head, Types...> {
    typedef typename std::conditional<(std::numeric_limits<Head>::max() >= N), Head, typename UnionTagType_aux<N, Types...>::type>::type type;
};

template<std::size_t N>
struct UnionTagType {
    typedef typename UnionTagType_aux<N, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>::type type;
};
}//namespace variant_detail

template <typename... Types>
class variant final {
    //static_assert(all_unique<T...>);
    typename std::aligned_storage<variant_detail::max_size<Types...>::value, variant_detail::max_align<Types...>::value>::type storage;
    typename variant_detail::UnionTagType<sizeof... (Types)>::type currentMember; //Index in T... of current member
#ifndef NDEBUG
    void scrub_storage() {
        std::memset(this, 0xDE, sizeof (variant));
    }
#endif
public:
//Mutators:
    //Move+Copy Constructors
    variant(variant const &o) {
#ifndef NDEBUG
        scrub_storage();
#endif
        currentMember = o.which();
        //TODO: use of visit here probably
        //violates strict aliasing (we are visiting an object that
        //has not yet been constructed).
        visit(variant_detail::CopyVisitor<variant, Types...>{o});
    }
    variant(variant &&o) noexcept {
#ifndef NDEBUG
        scrub_storage();
#endif
        currentMember = o.which();
        //TODO: use of visit here probably
        //violates strict aliasing (we are visiting an object that
        //has not yet been constructed).
        visit(variant_detail::MoveVisitor<variant, Types...>{std::move(o)});
    }

    //Move+Copy Assignment
    //(Wipes out existing object, makes new object in its place.
    // For assignment to contained object, use a visitor.)
    variant &operator=(variant const &o) {
        variant temp(o);
        visit(variant_detail::DtorVisitor<Types...>());
#ifndef NDEBUG
        scrub_storage();
#endif
        currentMember = o.which();
        //TODO: use of visit here probably
        //violates strict aliasing (we are visiting an object that
        //has not yet been constructed).
        try { visit(variant_detail::MoveVisitor<variant, Types...>{std::move(temp)}); }
        catch (...) { assert(false && "All types in variant must have no-throw move ctor"); }
        return *this;
    }
    variant &operator=(variant &&o) noexcept {
        variant temp(std::move(o));
        visit(variant_detail::DtorVisitor<Types...>());
#ifndef NDEBUG
        scrub_storage();
#endif
        currentMember = o.which();
        //TODO: use of visit here probably
        //violates strict aliasing (we are visiting an object that
        //has not yet been constructed).
        try { visit(variant_detail::MoveVisitor<variant, Types...>{std::move(temp)});  }
        catch (...) { assert(false && "All types in variant must have no-throw move ctor"); }
        return *this;
    }

    template<
        typename T,
        typename =
            typename std::enable_if<
                !variant_detail::is_tag<typename std::remove_reference<T>::type>::value && !std::is_same<T, variant>::value>::type>
    variant(T &&val) {
        typedef typename std::remove_const<typename std::remove_reference<T>::type>::type ActualT;
#ifndef NDEBUG
        scrub_storage();
#endif
        currentMember = variant_detail::IndexOf<ActualT, Types...>::value;
        new (&storage) (typename std::remove_reference<T>::type)(std::forward<T>(val));
    }

    //Construct object of type T, with `values` forwarded to the constructor.
    template<typename T, typename... V>
    explicit variant(tag<T>, V &&...values) {
#ifndef NDEBUG
        scrub_storage();
#endif
        currentMember = variant_detail::IndexOf<T, Types...>::value;
        new (&storage) T(std::forward<V>(values)...);
    }
    
    //Wipe out existing object.
    //Construct object of type T, with `values` forwarded to the constructor.
    template<typename T, typename... V>
    void reset(tag<T>, V &&...values) {
        //TODO - temp can be eliminated if construction from `values` is no-throw.
        T temp(std::forward<V>(values)...);

        visit(variant_detail::DtorVisitor<Types...>());
#ifndef NDEBUG
        scrub_storage();
#endif
        currentMember = variant_detail::IndexOf<T, Types...>::value;
        try { new (&storage) T(std::move(temp)); }
        catch (...) { assert(false && "All types in variant must have no-throw move ctor"); }
    }
    
    ~variant() noexcept {
        visit(variant_detail::DtorVisitor<Types...>());
    }

//Accessors:

    template<typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit(Visitor &&v) & {
        return visit_aux<sizeof... (Types) - 1>(v);
    }
    
    template<typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit(Visitor &&v) const & {
        return visit_aux_const<sizeof... (Types) - 1>(v);
    }
    
    template<typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit(Visitor &&v) && {
        return visit_aux_rvalue<sizeof... (Types) - 1>(v);
    }
    
    template<typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit(Visitor &&v) const && {
        return visit_aux_const_rvalue<sizeof... (Types) - 1>(v);
    }

    std::size_t which() const {
        return currentMember;
    }

private:
    struct dummy{};
    //Dummy parameter since templates cannot be fully specialised in class scope.
    template<typename, typename... ToLookFor> struct active_struct;
    template<typename unused> struct active_struct<unused> {
        bool operator()(typename variant_detail::UnionTagType<sizeof... (Types)>::type currentMember) const {return false;}
    };
    template<typename unused, typename Head, typename... ToLookFor>
    struct active_struct<unused, Head, ToLookFor...> {
        bool operator()(typename variant_detail::UnionTagType<sizeof... (Types)>::type currentMember) const {
            return variant_detail::IndexOf<Head, Types...>::value == currentMember
                || active_struct<unused, ToLookFor...>{}(currentMember);
        }
    };
public:

    template<typename... ToLookFor>
    bool active() const noexcept {
        return active_struct<dummy, ToLookFor...>{}(currentMember);
    }

    template<typename T>
    T &get() & noexcept {
        assert((variant_detail::IndexOf<T, Types...>::value == currentMember));
        return reinterpret_cast<T &>(storage);
    }
    template<typename T>
    T const &get() const & noexcept {
        assert((variant_detail::IndexOf<T, Types...>::value == currentMember));
        return reinterpret_cast<T const &>(storage);
    }

    template<typename T>
    T &&get() && noexcept {
        assert((variant_detail::IndexOf<T, Types...>::value == currentMember));
        return reinterpret_cast<T &&>(storage);
    }
    template<typename T>
    T const &&get() const && noexcept {
        assert((variant_detail::IndexOf<T, Types...>::value == currentMember));
        return reinterpret_cast<T const &&>(storage);
    }

private:
    //visit_aux
    template<std::size_t N, typename Visitor> friend struct visit_aux_struct;
    template<std::size_t N, typename Visitor> struct visit_aux_struct;
    template<typename Visitor>
    struct visit_aux_struct<0, Visitor> {
        variant *this_;
        visit_aux_struct(variant *this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            assert(0 == this_->which());
            return v(reinterpret_cast<typename variant_detail::NthType<0, Types...>::type &>(this_->storage));
        }
    };
    template<std::size_t N, typename Visitor>
    struct visit_aux_struct {
        variant *this_;
        visit_aux_struct(variant *this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            return (N == this_->which()) ?
                v(reinterpret_cast<typename variant_detail::NthType<N, Types...>::type &>(this_->storage))
              : visit_aux_struct<N-1, Visitor>(this_)(v);
        }
    };
    
    template<std::size_t N, typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit_aux(Visitor&& v) {
        return visit_aux_struct<N, Visitor>(this)(v);
    }
    
    //visit_aux_const
    template<std::size_t N, typename Visitor> friend struct visit_aux_const_struct;
    template<std::size_t N, typename Visitor> struct visit_aux_const_struct;
    template<typename Visitor>
    struct visit_aux_const_struct<0, Visitor> {
        variant const *this_;
        visit_aux_const_struct(variant const *this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            assert(0 == this_->which());
            return v(reinterpret_cast<typename variant_detail::NthType<0, Types...>::type const&>(this_->storage));
        }
    };
    template<std::size_t N, typename Visitor>
    struct visit_aux_const_struct {
        variant const *this_;
        visit_aux_const_struct(variant const *this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            return (N == this_->which()) ?
                v(reinterpret_cast<typename variant_detail::NthType<N, Types...>::type const &>(this_->storage))
              : visit_aux_const_struct<N-1, Visitor>(this_)(v);
        }
    };
    
    template<std::size_t N, typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit_aux_const(Visitor &&v) const {
        return visit_aux_const_struct<N, Visitor>(this)(v);
    }
    
    
    //visit_aux_rvalue
    template<std::size_t N, typename Visitor> friend struct visit_aux_rvalue_struct;
    template<std::size_t N, typename Visitor> struct visit_aux_rvalue_struct;
    template<typename Visitor>
    struct visit_aux_rvalue_struct<0, Visitor> {
        variant *this_;
        visit_aux_rvalue_struct(variant *this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            assert(0 == this_->which());
            return v(reinterpret_cast<typename variant_detail::NthType<0, Types...>::type &&>(this_->storage));
        }
    };
    template<std::size_t N, typename Visitor>
    struct visit_aux_rvalue_struct {
        variant *this_;
        visit_aux_rvalue_struct(variant *this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            return (N == this_->which()) ?
                v(reinterpret_cast<typename variant_detail::NthType<N, Types...>::type &&>(this_->storage))
              : visit_aux_struct<N-1, Visitor>(this_)(v);
        }
    };
    
    template<std::size_t N, typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit_aux_rvalue(Visitor&& v) {
        return visit_aux_rvalue_struct<N, Visitor>(this)(v);
    }
    
    //visit_aux_const_rvalue
    template<std::size_t N, typename Visitor> friend struct visit_aux_const_rvalue_struct;
    template<std::size_t N, typename Visitor> struct visit_aux_const_rvalue_struct;
    template<typename Visitor>
    struct visit_aux_const_rvalue_struct<0, Visitor> {
        variant const *this_;
        visit_aux_const_rvalue_struct(variant const *this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            assert(0 == this_->which());
            return v(reinterpret_cast<typename variant_detail::NthType<0, Types...>::type const&&>(this_->storage));
        }
    };
    template<std::size_t N, typename Visitor>
    struct visit_aux_const_rvalue_struct {
        variant const *this_;
        visit_aux_const_rvalue_struct(variant const *this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            return (N == this_->which()) ?
                v(reinterpret_cast<typename variant_detail::NthType<N, Types...>::type const &&>(this_->storage))
              : visit_aux_const_struct<N-1, Visitor>(this_)(v);
        }
    };
    
    template<std::size_t N, typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit_aux_const_rvalue(Visitor &&v) const {
        return visit_aux_const_rvalue_struct<N, Visitor>(this)(v);
    }
};

namespace variant_detail {

template<int ...>
struct seq { };

template<int N, int ...S>
struct gens : gens<N-1, N-1, S...> { };

template<int ...S>
struct gens<0, S...> {
  typedef seq<S...> type;
};

template<int ...S, typename Head, typename ...Tail>
std::tuple<Tail...> tuple_tail_impl(seq<S...>, std::tuple<Head, Tail...> const&in_tuple) {
    return std::tuple<Tail...>(std::get<S+1>(in_tuple)...);
}

template<typename Head, typename ...Tail>
std::tuple<Tail...> tuple_tail(std::tuple<Head, Tail...> const& in_tuple) {
    return tuple_tail_impl(typename gens<std::tuple_size<std::tuple<Tail...>>::value>::type(), in_tuple);
}

template<typename Visitor, typename MatchedValueTuple, typename... TailVariants>
struct NAryVisitorFlattener;

template<typename Visitor, typename MatchedValueTuple, typename CurrentVariant, typename... TailVariants>
struct NAryVisitorFlattener<Visitor, MatchedValueTuple, CurrentVariant, TailVariants...> {
    typedef typename std::remove_reference<Visitor>::type::result_type result_type;

    Visitor visitor;
    MatchedValueTuple matchedValues;
    std::tuple<CurrentVariant &&, TailVariants &&...> tailVariants;

    template<typename A>
    result_type operator()(A &&a) {
        return std::get<0>(tailVariants)
            .visit(
                NAryVisitorFlattener<
                    Visitor, decltype(std::tuple_cat(matchedValues, std::forward_as_tuple(std::forward<A>(a)))), TailVariants...
                >
                {
                    std::forward<Visitor>(visitor),
                    std::tuple_cat(matchedValues, std::forward_as_tuple(std::forward<A>(a))),
                    tuple_tail(tailVariants)
                });
    }
};


template<typename Visitor, typename MatchedValueTuple>
struct NAryVisitorFlattener<Visitor, MatchedValueTuple> {
    typedef typename std::remove_reference<Visitor>::type::result_type result_type;

    Visitor visitor;
    MatchedValueTuple matchedValues;
    std::tuple<> tailVariants;
    
    template<typename A>
    result_type operator()(A &&a) {
        return callFunc(typename gens<std::tuple_size<MatchedValueTuple>::value>::type(), std::forward<A>(a));
    }
    
    template<int ...S, typename A>
    result_type callFunc(seq<S...>, A &&a) {
        return std::forward<Visitor>(visitor)(std::get<S>(matchedValues)..., std::forward<A>(a));
    }
};
}//namespace variant_detail

template<typename Visitor, typename VariantA, typename... Variants>
typename std::remove_reference<Visitor>::type::result_type
apply_visitor(Visitor &&visitor, VariantA &&variantA, Variants &&...variants) {
    return
        std::forward<VariantA>(variantA)
        .visit(
            variant_detail::NAryVisitorFlattener<Visitor, std::tuple<>, Variants&&...>{
                std::forward<Visitor>(visitor),
                std::tuple<>{},
                std::forward_as_tuple(std::forward<Variants>(variants)...)});
}

}//namespace hg

#endif //HG_VARIANT_H
