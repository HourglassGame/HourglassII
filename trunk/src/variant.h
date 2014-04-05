#ifndef HG_VARIANT_H
#define HG_VARIANT_H

//TODO:
// Strict aliasing compatibility
// Better compile-time checks (eg, ensure types are unique)
// Recursive variant
// N-ary visitation
// Visitor return value
// Optimisation for no-throw ctors
// Reference members (?)
// Better namespace safety
// Clean up implementation

#include <type_traits>
#include <cassert>
#include <tuple>
#include <utility>
#include <cstring>
#include <limits>
namespace hg {
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

template <typename Visitor, typename... Variants>
typename Visitor::result_type apply_visitor(Visitor&& vis, Variants&&... variants) {
    //TODO
    assert(false);
}

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
struct tag {};

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
    void operator()(Head& h) const {
        h.~Head();
    }
};
template<typename Head, typename... Types>
struct DtorVisitor<Head, Types...> : DtorVisitor<Types...> {
    using DtorVisitor<Types...>::operator();
    void operator()(Head& h) const {
        h.~Head();
    }
};

template<typename... Types>
struct CopyVisitor;

template<typename Variant, typename Head> struct CopyVisitor<Variant, Head> {
    typedef void result_type;
    Variant const* o;
    CopyVisitor(Variant const& o) : o(&o) {}
    void operator()(Head const& h) const {
        new ((void*)&h) Head(o->template get<Head>());
    }
};
template<typename Variant, typename Head, typename... Types>
struct CopyVisitor<Variant, Head, Types...> : CopyVisitor<Variant, Types...> {
    CopyVisitor(Variant const& o) : CopyVisitor<Variant, Types...>(o) {}
    using CopyVisitor<Variant, Types...>::operator();
    using CopyVisitor<Variant, Types...>::o;
    void operator()(Head const& h) const {
        new ((void*)&h) Head(o->template get<Head>());
    }
};

template<typename... Types>
struct MoveVisitor;

template<typename Variant, typename Head> struct MoveVisitor<Variant, Head> {
    typedef void result_type;
    Variant* o;
    MoveVisitor(Variant&& o) : o(&o) {}
    void operator()(Head& h) const {
        new ((void*)&h) Head(std::move(o->template get<Head>()));
    }
};
template<typename Variant, typename Head, typename... Types>
struct MoveVisitor<Variant, Head, Types...> : MoveVisitor<Variant, Types...> {
    MoveVisitor(Variant&& o) : MoveVisitor<Variant, Types...>(std::move(o)) {}
    using MoveVisitor<Variant, Types...>::operator();
    using MoveVisitor<Variant, Types...>::o;
    void operator()(Head& h) const {
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

template <typename... Types>
class variant {
    //static_assert(all_unique<T...>);
    typename std::aligned_storage<max_size<Types...>::value, max_align<Types...>::value>::type storage;
    typename UnionTagType<sizeof... (Types)>::type currentMember; //Index in T... of current member
    void scrub_storage() {
        std::memset(this, 0xDE, sizeof(variant));
    }
public:
//Mutators:
    //Move+Copy Constructors
    variant(variant const& o) {
        scrub_storage();
        currentMember = o.which();
        //TODO: use of visit here probably
        //violates strict aliasing (we are visiting an object that
        //has not yet been constructed).
        visit(CopyVisitor<variant, Types...>{o});
    }
    variant(variant&& o) {
        scrub_storage();
        currentMember = o.which();
        //TODO: use of visit here probably
        //violates strict aliasing (we are visiting an object that
        //has not yet been constructed).
        visit(MoveVisitor<variant, Types...>{std::move(o)});
    }

    //Move+Copy Assignment
    //(Wipes out existing object, makes new object in its place.
    // For assignment to contained object, use a visitor.)
    variant& operator=(variant const& o) {
        variant temp(o);
        visit(DtorVisitor<Types...>());
        scrub_storage();
        currentMember = o.which();
        //TODO: use of visit here probably
        //violates strict aliasing (we are visiting an object that
        //has not yet been constructed).
        try { visit(MoveVisitor<variant, Types...>{std::move(temp)}); }
        catch (...) { assert(false && "All types in variant must have no-throw move ctor"); }
        return *this;
    }
    variant& operator=(variant&& o) {
        variant temp(std::move(o));
        visit(DtorVisitor<Types...>());
        scrub_storage();
        currentMember = o.which();
        //TODO: use of visit here probably
        //violates strict aliasing (we are visiting an object that
        //has not yet been constructed).
        try { visit(MoveVisitor<variant, Types...>{std::move(temp)});  }
        catch (...) { assert(false && "All types in variant must have no-throw move ctor"); }
        return *this;
    }

    template<
        typename T,
        typename =
            typename std::enable_if<
                !is_tag<typename std::remove_reference<T>::type>::value && !std::is_same<T, variant>::value>::type>
    variant(T&& val) {
        typedef typename std::remove_const<typename std::remove_reference<T>::type>::type ActualT;
        scrub_storage();
        currentMember = IndexOf<ActualT, Types...>::value;
        new (&storage) (typename std::remove_reference<T>::type)(std::forward<T>(val));
    }

    //Construct object of type T, with `values` forwarded to the constructor.
    template<typename T, typename... V>
    explicit variant(tag<T>, V&&... values) {
        scrub_storage();
        currentMember = IndexOf<T, Types...>::value;
        new (&storage) T(std::forward<V>(values)...);
    }
    
    //Wipe out existing object.
    //Construct object of type T, with `values` forwarded to the constructor.
    template<typename T, typename... V>
    void reset(tag<T>, V&&... values) {
        //TODO - temp can be eliminated if construction from `values` is no-throw.
        T temp(std::forward<V>(values)...);

        visit(DtorVisitor<Types...>());
        scrub_storage();
        currentMember = IndexOf<T, Types...>::value;
        try { new (&storage) T(std::move(temp)); }
        catch (...) { assert(false && "All types in variant must have no-throw move ctor"); }
    }
    
    ~variant() {
        visit(DtorVisitor<Types...>());
    }

//Accessors:

    template<typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit(Visitor&& v) & {
        return visit_aux<sizeof... (Types) - 1>(v);
    }
    
    template<typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit(Visitor&& v) const & {
        return visit_aux_const<sizeof... (Types) - 1>(v);
    }
    
    template<typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit(Visitor&& v) && {
        return visit_aux_rvalue<sizeof... (Types) - 1>(v);
    }
    
    template<typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit(Visitor&& v) const && {
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
        bool operator()(typename UnionTagType<sizeof... (Types)>::type currentMember) const {return false;}
    };
    template<typename unused, typename Head, typename... ToLookFor>
    struct active_struct<unused, Head, ToLookFor...> {
        bool operator()(typename UnionTagType<sizeof... (Types)>::type currentMember) const {
            return IndexOf<Head, Types...>::value == currentMember || active_struct<unused, ToLookFor...>{}(currentMember);
        }
    };
public:

    template<typename... ToLookFor>
    bool active() const {
        return active_struct<dummy, ToLookFor...>{}(currentMember);
    }

    template<typename T>
    T& get() {
        assert((IndexOf<T, Types...>::value == currentMember));
        return reinterpret_cast<T&>(storage);
    }
    template<typename T>
    T const& get() const {
        assert((IndexOf<T, Types...>::value == currentMember));
        return reinterpret_cast<T const&>(storage);
    }

private:
    //visit_aux
    template<std::size_t N, typename Visitor> friend struct visit_aux_struct;
    template<std::size_t N, typename Visitor> struct visit_aux_struct;
    template<typename Visitor>
    struct visit_aux_struct<0, Visitor> {
        variant* this_;
        visit_aux_struct(variant* this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            assert(0 == this_->which());
            return v(reinterpret_cast<typename NthType<0, Types...>::type&>(this_->storage));
        }
    };
    template<std::size_t N, typename Visitor>
    struct visit_aux_struct {
        variant* this_;
        visit_aux_struct(variant* this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            return (N == this_->which()) ?
                v(reinterpret_cast<typename NthType<N, Types...>::type&>(this_->storage))
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
        variant const* this_;
        visit_aux_const_struct(variant const* this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            assert(0 == this_->which());
            return v(reinterpret_cast<typename NthType<0, Types...>::type const&>(this_->storage));
        }
    };
    template<std::size_t N, typename Visitor>
    struct visit_aux_const_struct {
        variant const* this_;
        visit_aux_const_struct(variant const* this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            return (N == this_->which()) ?
                v(reinterpret_cast<typename NthType<N, Types...>::type const&>(this_->storage))
              : visit_aux_const_struct<N-1, Visitor>(this_)(v);
        }
    };
    
    template<std::size_t N, typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit_aux_const(Visitor&& v) const {
        return visit_aux_const_struct<N, Visitor>(this)(v);
    }
    
    
    //visit_aux_rvalue
    template<std::size_t N, typename Visitor> friend struct visit_aux_rvalue_struct;
    template<std::size_t N, typename Visitor> struct visit_aux_rvalue_struct;
    template<typename Visitor>
    struct visit_aux_rvalue_struct<0, Visitor> {
        variant* this_;
        visit_aux_rvalue_struct(variant* this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            assert(0 == this_->which());
            return v(reinterpret_cast<typename NthType<0, Types...>::type&&>(this_->storage));
        }
    };
    template<std::size_t N, typename Visitor>
    struct visit_aux_rvalue_struct {
        variant* this_;
        visit_aux_rvalue_struct(variant* this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            return (N == this_->which()) ?
                v(reinterpret_cast<typename NthType<N, Types...>::type&&>(this_->storage))
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
        variant const* this_;
        visit_aux_const_rvalue_struct(variant const* this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            assert(0 == this_->which());
            return v(reinterpret_cast<typename NthType<0, Types...>::type const&&>(this_->storage));
        }
    };
    template<std::size_t N, typename Visitor>
    struct visit_aux_const_rvalue_struct {
        variant const* this_;
        visit_aux_const_rvalue_struct(variant const* this_) : this_(this_) {}
        typename std::remove_reference<Visitor>::type::result_type operator()(Visitor v) {
            return (N == this_->which()) ?
                v(reinterpret_cast<typename NthType<N, Types...>::type const&&>(this_->storage))
              : visit_aux_const_struct<N-1, Visitor>(this_)(v);
        }
    };
    
    template<std::size_t N, typename Visitor>
    typename std::remove_reference<Visitor>::type::result_type visit_aux_const_rvalue(Visitor&& v) const {
        return visit_aux_const_rvalue_struct<N, Visitor>(this)(v);
    }
};

template<typename Visitor, typename U>
struct BinaryBoundVisitor {
    Visitor visitor;
    U boundVar;
    typedef typename std::remove_reference<Visitor>::type::result_type result_type;
    
    template<typename T>
    result_type operator()(T&& t) {
        return std::forward<Visitor>(visitor)(std::forward<U>(boundVar), std::forward<T>(t));
    }
};

template<typename Visitor, typename VariantB>
struct BinaryVisitorFlattener {
    Visitor visitor;
    VariantB variant_b;
    typedef typename std::remove_reference<Visitor>::type::result_type result_type;
    template<typename T>
    result_type operator()(T&& t) {
        return std::forward<VariantB>(variant_b).visit(BinaryBoundVisitor<Visitor, T>{std::forward<Visitor>(visitor), std::forward<T>(t)});
    }
};

template <typename Visitor, typename VariantA, typename VariantB>
typename std::remove_reference<Visitor>::type::result_type apply_visitor(Visitor&& visitor, VariantA&& variant_a, VariantB&& variant_b) {
    return std::forward<VariantA>(variant_a).visit(BinaryVisitorFlattener<Visitor, VariantB>{std::forward<Visitor>(visitor), std::forward<VariantB>(variant_b)});
}
template<typename Visitor, typename A, typename B>
struct TrinaryVisitorFlattener3 {
    typedef typename std::remove_reference<Visitor>::type::result_type result_type;
    Visitor visitor;
    std::tuple<A, B> matchedValues;
    
    template<typename C>
    result_type operator()(C&& c) {
        return std::forward<Visitor>(visitor)(
            std::forward<A>(std::get<0>(matchedValues)),
            std::forward<B>(std::get<1>(matchedValues)),
            std::forward<C>(c));
    }
};

template<typename Visitor, typename VariantC, typename A>
struct TrinaryVisitorFlattener2 {
    typedef typename std::remove_reference<Visitor>::type::result_type result_type;
    Visitor visitor;
    VariantC variant_c;
    A a;
    template<typename B>
    result_type operator()(B&& b) {
        return std::forward<VariantC>(variant_c)
            .visit(TrinaryVisitorFlattener3<Visitor, A, B>{
                std::forward<Visitor>(visitor),
                std::tuple<A, B>{std::forward<A>(a), std::forward<B>(b)}});
    }
};

template<typename Visitor, typename VariantB, typename VariantC>
struct TrinaryVisitorFlattener {
    typedef typename std::remove_reference<Visitor>::type::result_type result_type;
    Visitor visitor;
    VariantB variant_b;
    VariantC variant_c;
    
    template<typename A>
    result_type operator()(A&& a) {
        return std::forward<VariantB>(variant_b)
            .visit(TrinaryVisitorFlattener2<Visitor, VariantC, A>{
                std::forward<Visitor>(visitor),
                std::forward<VariantC>(variant_c),
                std::forward<A>(a)});
    }
};

template <typename Visitor, typename VariantA, typename VariantB, typename VariantC>
typename std::remove_reference<Visitor>::type::result_type
apply_visitor(Visitor&& visitor, VariantA&& variant_a, VariantB&& variant_b, VariantC&& variant_c) {
    return
        std::forward<VariantA>(variant_a)
        .visit(
            TrinaryVisitorFlattener<Visitor, VariantB, VariantC>{
                std::forward<Visitor>(visitor),
                std::forward<VariantB>(variant_b),
                std::forward<VariantC>(variant_c)});
}
}




#endif //HG_VARIANT_H
