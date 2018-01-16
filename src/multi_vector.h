#ifndef HG_MULTI_VECTOR_H
#define HG_MULTI_VECTOR_H
#include <memory>
#include <array>
#include <boost/range/numeric.hpp>
#include <functional>
#include <boost/container/vector.hpp>
#include <boost/operators.hpp>
#include <boost/range/algorithm/equal.hpp>

//TODO: add 'memory region iterators' for use in the implementation
//(Rather than getPosi etc)

namespace hg {
template<std::size_t N_dims>
std::size_t total_by_dim(size_t const dim, std::array<std::size_t, N_dims> const &extents)
{
    return std::accumulate(
        extents.begin() + dim,
        extents.end(),
        std::size_t(1),
        std::multiplies<>());
}
//TODO: Everywhere these functions are used, some sort of iterator could probably be used instead,
//      to avoid the recalculation every time.
template<std::size_t N_dims>
std::array<std::size_t, N_dims> getIndex(
    std::size_t pos, std::array<std::size_t, N_dims> const &size)
{
    std::array<std::size_t, N_dims> index;
    for (std::size_t dim{0}; dim != N_dims - 1; ++dim)
    {
        auto const current_block_logical_length(total_by_dim(dim + 1, size));

        //Deliberate integer truncation.
        std::size_t const block_num = pos / current_block_logical_length;
        pos -= block_num * current_block_logical_length;
        index[dim] = block_num;
    }
    index[N_dims - 1] = pos;
    return index;
}
template<std::size_t N_dims, typename T>
T *getPosByIndex(
    std::array<std::size_t, N_dims> const &index, T * const data, std::array<std::size_t, N_dims> const &capacity)
{
    auto elem{data};
    for (std::size_t dim{0}; dim != N_dims; ++dim)
    {
        auto const current_block_physical_length(total_by_dim(dim + 1, capacity));

        std::size_t const block_num = index[dim];
        elem += block_num * current_block_physical_length;
    }
    //assert(pos < total_by_dim(0, capacity));
    return elem;
}
template<std::size_t N_dims>
bool isIndexInRegion(std::array<std::size_t, N_dims> const &idx, std::array<std::size_t, N_dims> const &region) {
    //TODO: use std algo?
    for (std::size_t i{0}; i != N_dims; ++i) {
        if (idx[i] >= region[i]) return false;
    }
    return true;
}

template<std::size_t N_dims>
bool regionFitsInRegion(std::array<std::size_t, N_dims> const &innerRegion, std::array<std::size_t, N_dims> const &region) {
    //TODO: use std algo?
    for (std::size_t i{ 0 }; i != N_dims; ++i) {
        if (innerRegion[i] > region[i]) return false;
    }
    return true;
}

template<std::size_t N_dims, typename T>
T *getPosi(std::size_t const pos, T * const data, std::array<std::size_t, N_dims> const &size, std::array<std::size_t, N_dims> const &capacity) {
    return getPosByIndex(getIndex(pos, size), data, capacity);
    //DON'T Delete this for now, as it is probably a more efficient implementation of the above!:
#if 0
    T *elem(data);
    for (std::size_t dim{0}; dim != N_dims - 1; ++dim)
    {
        auto const current_block_physical_length(total_by_dim(dim + 1, capacity));
        auto const current_block_logical_length(total_by_dim(dim + 1, size));

        //Deliberate integer truncation.
        std::size_t block_num = pos / current_block_logical_length;
        pos -= block_num * current_block_logical_length;
        elem += block_num * current_block_physical_length;
    }
    assert(pos < total_by_dim(0, capacity));
    return elem + pos;
#endif
}

template<typename T, std::size_t N_dims, typename Allocator = std::allocator<T>>
class multi_vector final : private Allocator {
public:
    typedef Allocator allocator_type;
    Allocator &get_allocator() {
        return *this;
    }
    Allocator const &get_allocator() const {
        return *this;
    }
private:
    //TODO Eliminate duplication!!
    T *getPos(std::size_t const pos) {
        return getPosi(pos, data_, size_, capacity_);
    }
    T const *getPos(std::size_t const pos) const {
        return getPosi(pos, data_, size_, capacity_);
    }

    T &getAtPos(std::size_t const pos)
    {
        return *getPos(pos);
    }

    T const &getAtPos(std::size_t const pos) const
    {
        return *getPos(pos);
    }

public:
    std::size_t total_capacity_by_dim(size_t const dim) const
    {
        return std::accumulate(
                std::begin(capacity_)+dim,
                std::end(capacity_),
                std::size_t(1),
                std::multiplies<>());
    }

    std::size_t total_size_by_dim(size_t const dim) const
    {
        return std::accumulate(
            std::begin(size_)+dim,
            std::end(size_),
            std::size_t(1),
            std::multiplies<>());
    }
private:
    //something storage_order; //maybe?

    std::array<std::size_t, N_dims> size_;
    std::array<std::size_t, N_dims> capacity_; //[2,3,4] stored as {{{a,a,a,a},{a,a,a,a},{a,a,a,a}}, {{a,a,a,a},{a,a,a,a},{a,a,a,a}}}
     //boost::container::vector<T, Allocator> data;
    T *data_;
public:
    class simple_const_iterator_proxy final : boost::totally_ordered<simple_const_iterator_proxy>
    {
    private:
        multi_vector const *const this_;
        std::size_t pos;

    public:
        simple_const_iterator_proxy(multi_vector const * const this_, std::size_t const pos) : this_(this_), pos(pos) {}

        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef T const* pointer;
        typedef T const& reference;
        typedef std::random_access_iterator_tag iterator_category;
        reference operator*() const
        {
            return this_->getAtPos(pos);
        }
        simple_const_iterator_proxy &operator++()
        {
            ++pos;
            return *this;
        }
        simple_const_iterator_proxy &operator++(int)
        {
            ++pos;
            return *this;
        }
        bool operator==(simple_const_iterator_proxy const &o) const
        {
            return pos == o.pos;
        }
        bool operator<(simple_const_iterator_proxy const &o) const
        {
            return pos < o.pos;
        }
        
        simple_const_iterator_proxy &operator+=(difference_type const dif)
        {
            pos += dif;
            return *this;
        }

        simple_const_iterator_proxy &operator-=(difference_type const dif)
        {
            pos -= dif;
            return *this;
        }

        simple_const_iterator_proxy operator+(difference_type const dif) const
        {
            return simple_const_iterator_proxy(*this) += dif;
        }

        simple_const_iterator_proxy operator-(difference_type const dif) const
        {
            return simple_const_iterator_proxy(*this) -= dif;
        }

        difference_type operator-(simple_const_iterator_proxy const &r) const
        {
            return pos - r.pos;
        }
        
        reference operator[](difference_type const dif)
        {
            return *(*this + dif);
        }
    };
    typedef simple_const_iterator_proxy const_iterator;

    typedef std::size_t index;
    template<std::size_t remaining_dims, typename U>
    friend struct op_index_proxy;
    template<std::size_t remaining_dims, typename U>
    struct op_index_proxy final {
    //private:
        U *const array_start;
        multi_vector const *const this_;
        static std::size_t const current_axis = N_dims - remaining_dims;
    //public:
        op_index_proxy<remaining_dims-1,U> operator[](std::size_t const i) const {
            assert(current_axis < this_->size_.size());
            assert(i < this_->size_[current_axis]);
            auto const current_block_length{
                std::accumulate(
                    std::begin(this_->capacity_)+current_axis+1,
                    std::end(this_->capacity_),
                    std::size_t(1),
                    std::multiplies<>())};

            return {array_start + current_block_length*i, this_};
        }
    };
    template<typename U>
    struct op_index_proxy<1,U> final {
    //private:
        U *const array_start;
        multi_vector const *const this_;
        static std::size_t const current_axis = N_dims - 1;
    //public:
        U &operator[](std::size_t const i) const {
            //These two assertions could be placed in a constructor.
            //They do not depend on i.
            //TODO: All these asserts make less sense now that data_ isn't a vector
            assert(this_->total_capacity_by_dim(0) >= this_->size_[current_axis]);
            assert(std::less_equal<>()(array_start, this_->data_ + this_->total_capacity_by_dim(0)));
            assert(std::less_equal<>()(array_start, this_->data_ + this_->total_capacity_by_dim(0) - this_->size_[current_axis]));

            assert(current_axis < this_->size_.size());
            assert(i < this_->size_[current_axis]);
            //TODO: This assert invokes undefined behaviour if violated! (Due to inter-array comparisons/creation of invalid pointers)
            //Not sure how to avoid this. I believe the earlier assertions in combination are equivalent to this assertion; so maybe this
            //assertion can just be removed.
            assert(array_start+i >= this_->data_ && array_start+i < this_->data_ + this_->total_capacity_by_dim(0));
            return array_start[i];
        }
    };

    explicit multi_vector() noexcept(noexcept(Allocator())) :
        multi_vector(Allocator())
    {
    }

    explicit multi_vector(Allocator alloc) noexcept :
        Allocator(std::move(alloc)), size_(), capacity_(), data_()
    {
    }
    ~multi_vector() noexcept
    {
        for (auto &&a: *this) {
            std::allocator_traits<Allocator>::destroy(get_allocator(), &a);
        }
        if (data_) {
            get_allocator().deallocate(data_, total_capacity_by_dim(0));
        }
    }
    multi_vector(multi_vector &&o) noexcept :
        Allocator(std::move(o.get_allocator())), size_(std::move(o.size_)), capacity_(std::move(o.capacity_)), data_(std::move(o.data_))
    {
        o.size_.fill(0);
        o.capacity_.fill(0);
        o.data_ = nullptr;
    }
    //multi_vector(multi_vector &&o, Allocator alloc) {}//TODO
    multi_vector(multi_vector const &o) :
        Allocator(std::allocator_traits<Allocator>::select_on_container_copy_construction(o.get_allocator())), size_(o.size_), capacity_(o.capacity_), data_(get_allocator().allocate(total_capacity_by_dim(0)))
    {
        std::size_t i{0};
        try {
            for (std::size_t const sz{o.total_size_by_dim(0)}; i != sz; ++i) {
                std::allocator_traits<Allocator>::construct(get_allocator(), getPos(i), o.getAtPos(i));
            }
        }
        catch (...) {
            while (i != 0) {
                --i;
                std::allocator_traits<Allocator>::destroy(get_allocator(), getPos(i));
            }
            get_allocator().deallocate(data_, total_capacity_by_dim(0));
            throw;
        }
    }
    void swap(multi_vector &o) {
        if (std::allocator_traits<Allocator>::propagate_on_container_swap::value) {
            boost::swap(get_allocator(), o.get_allocator());
        }
        boost::swap(size_, o.size_);
        boost::swap(capacity_, o.capacity_);
        boost::swap(data_, o.data_);
    }
    //multi_vector(multi_vector const &o, Allocator alloc) {} //TODO
    multi_vector &operator=(multi_vector &&o) noexcept
    {
        if (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value) {
            get_allocator() = std::move(o.get_allocator());
        }
        boost::swap(size_, o.size_);
        boost::swap(capacity_, o.capacity_);
        boost::swap(data_, o.data_);
        return *this;
    }
    multi_vector &operator=(multi_vector const &o)
    {
        //if (this == boost::addressof(o)) return *this;

        //TODO: if allocator == o.get_allocator, should the allocator still be propagated on assignment??
        if (regionFitsInRegion(o.size_, capacity_) && (get_allocator() == o.get_allocator() || !std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value)) {
            //TODO: The most high performance version might mix the steps in order to work on the longest
            //possible contiguous blocks of memory?

            auto const oldSize = size_;
            size_ = o.size_;
            std::size_t newI{0};
            try {
                //Construct/assign new elements.
                for (auto const sz{total_size_by_dim(0)}; newI != sz; ++newI) {
                    auto const idx{getIndex(newI, size_)};
                    if (!isIndexInRegion(idx, oldSize)) {
                        std::allocator_traits<Allocator>::construct(get_allocator(), getPos(newI), *o.getPos(newI));
                    }
                    else {
                        *getPos(newI) = *o.getPos(newI);
                    }
                }
            }
            catch (...) {
                while (newI != 0) {
                    --newI;
                    auto const idx{getIndex(newI, size_)};
                    if (!isIndexInRegion(idx, oldSize)) {
                        std::allocator_traits<Allocator>::destroy(get_allocator(), getPos(newI));
                    }
                }
                size_ = oldSize;
                throw;
            }
            //Destruct old elements.
            std::size_t oldI{0};
            for (auto const oldSz{ boost::accumulate(oldSize, static_cast<std::size_t>(1), std::multiplies<>{}) };
                oldI != oldSz;
                ++oldI)
            {
                auto const idx{ getIndex(oldI, oldSize) };
                if (!isIndexInRegion(idx, o.size_)) {
                    std::allocator_traits<Allocator>::destroy(get_allocator(), getPosByIndex(idx, data_, capacity_));
                }
            }

        }
        //else if (total_capacity_by_dim(0) >= o.total_size_by_dim(0)) {
            //TODO: Remember exception safety!

            //iterate old locations no longer containing data in new shape
            //destruct elements (can't throw)

            //iterate old locations that will have data in new shape
            //assign elements (if an exception is thrown here, it might be necessary to destruct everything!
            //to get back to a valid multi_vector state)

            //(if the assignment were done before the destruction of the elements, the exception safety would be a bit easier,
            // but a larger number of resources would be used (since the new elements would be assigned before the old elements
            // were destroyed. It might be worth doing anyway though.))

            //iterate previously uninitialised locations that contain data in new shape
            //construct elements (if an exception is thrown here, it might be necessary to destruct everything
            //to get back to a valid multi_vector state)

            //The most high performance version might mix these steps in order to work on the longest
            //possible contiguous blocks of memory?

        //}
        else {
            for (auto &&a : *this) {
                std::allocator_traits<Allocator>::destroy(get_allocator(), &a);
            }
            //TODO: Is this safe if data_ == nullptr?
            get_allocator().deallocate(data_, total_capacity_by_dim(0));
            //TODO: Don't deallocate if existing memory space is sufficient!! (see above)
            size_.fill(0);
            capacity_.fill(0);
            data_ = nullptr;
            if (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value) {
                get_allocator() = o.get_allocator();
            }
            data_ = get_allocator().allocate(o.total_capacity_by_dim(0));
            size_ = o.size_;
            capacity_ = o.capacity_;
            std::size_t i{0};
            try {
                for (std::size_t const sz{o.total_size_by_dim(0)}; i != sz; ++i) {
                    std::allocator_traits<Allocator>::construct(get_allocator(), getPos(i), o.getAtPos(i));
                }
            }
            catch (...) {
                while (i != 0) {
                    --i;
                    std::allocator_traits<Allocator>::destroy(get_allocator(), getPos(i));
                }
                get_allocator().deallocate(data_, total_capacity_by_dim(0));
                size_.fill(0);
                capacity_.fill(0);
                data_ = nullptr;
                throw;
            }
        }
        return *this;
    }
    template<typename RandomAccessRange>
    multi_vector(std::array<std::size_t, N_dims> const &size, RandomAccessRange const &data) :
        Allocator(), size_(size), capacity_(size), data_(get_allocator().allocate(total_capacity_by_dim(0)))
    {
        assert(boost::size(data) == total_size_by_dim(0));
        assert(boost::size(data) <= total_capacity_by_dim(0));
        auto it{boost::begin(data)};
        std::size_t i{0};
        try {
            for (std::size_t const sz{boost::size(data)}; i != sz; ++i, ++it) {
                std::allocator_traits<Allocator>::construct(get_allocator(), getPos(i), *it);
            }
        }
        catch (...) {
            while (i != 0) {
                --i;
                std::allocator_traits<Allocator>::destroy(get_allocator(), getPos(i));
            }
            get_allocator().deallocate(data_, total_capacity_by_dim(0));
            throw;
        }
    }

    template<typename RandomAccessRange>
    multi_vector(std::array<std::size_t, N_dims> const &size, RandomAccessRange const &data, Allocator alloc) :
        Allocator(std::move(alloc)), size_(size), capacity_(size), data_(get_allocator().allocate(total_capacity_by_dim(0)))
    {
        assert(boost::size(data) == total_size_by_dim(0));
        assert(boost::size(data) <= total_capacity_by_dim(0));
        auto it{boost::begin(data)};
        std::size_t i{0};
        try {
            for (std::size_t const sz{boost::size(data)}; i != sz; ++i, ++it) {
                std::allocator_traits<Allocator>::construct(get_allocator(), getPos(i), *it);
            }
        }
        catch (...) {
            while (i != 0) {
                --i;
                std::allocator_traits<Allocator>::destroy(get_allocator(), getPos(i));
            }
            get_allocator().deallocate(data_, total_capacity_by_dim(0));
            throw;
        }
    }
    //template<typename NDimArray>
    //multi_vector(NDimArray const& toCopy); TODO

    //Giving size and inserting values
    void resize(std::array<std::size_t, N_dims> const &newSize) {
        std::size_t const newStorageSpace = boost::accumulate(newSize, static_cast<std::size_t>(1), std::multiplies<>{});
        if (regionFitsInRegion(newSize, capacity_))
        {
            auto const oldSize = size_;
            size_ = newSize;
            std::size_t newI{0};
            try {
                //Construct new elements.
                for(; newI != newStorageSpace; ++newI) {
                    auto const idx{getIndex(newI, size_)};
                    if (!isIndexInRegion(idx, oldSize)) {
                        std::allocator_traits<Allocator>::construct(get_allocator(), getPos(newI));
                    }
                }
            }
            catch (...) {
                while (newI != 0) {
                    --newI;
                    auto const idx{getIndex(newI, size_)};
                    if (!isIndexInRegion(idx, oldSize)) {
                        std::allocator_traits<Allocator>::destroy(get_allocator(), getPos(newI));
                    }
                }
                size_ = oldSize;
                throw;
            }
            //Destruct old elements.
            std::size_t oldI{0};
            for (auto const oldSz{boost::accumulate(oldSize, static_cast<std::size_t>(1), std::multiplies<>{})};
                oldI != oldSz;
                ++oldI)
            {
                auto const idx{getIndex(oldI, oldSize)};
                if (!isIndexInRegion(idx, newSize)) {
                    std::allocator_traits<Allocator>::destroy(get_allocator(), getPosByIndex(idx, data_, capacity_));
                }
            }
        }
        //TODO, don't re-allocate when there is sufficient space, but it happens to be laid out in the wrong shape?
        //else if (newStorageSpace <= total_capacity_by_dim(0)) {}
        else
        {
            multi_vector const old{std::move(*this)};
            //assert(total_capacity_by_dim(0)==0 && "Missing Feature! For now, we only allow single resizing.");
            data_ = get_allocator().allocate(newStorageSpace);
            size_ = newSize;
            capacity_ = newSize;
            std::size_t i{0};
            try {
                for (; i != newStorageSpace; ++i) {
                    auto const idx{getIndex(i, size_)};
                    if (isIndexInRegion(idx, old.size_)) {
                        std::allocator_traits<Allocator>::construct(get_allocator(), getPos(i), std::move(*getPosByIndex(idx, old.data_, old.capacity_)));
                    }
                    else {
                        std::allocator_traits<Allocator>::construct(get_allocator(), getPos(i));
                    }
                }
            }
            catch (...) {
                while (i != 0) {
                    --i;
                    std::allocator_traits<Allocator>::destroy(get_allocator(), getPos(i));
                }
                get_allocator().deallocate(data_, total_capacity_by_dim(0));
                size_.fill(0);
                capacity_.fill(0);
                data_ = nullptr;
                throw;
            }
        }

        //std::cout << (void*)this << "newSize: " << newSize << "\n";
    }
    //TODO: Use iterator rather than axis/position?
    //TODO: How to support both multi-vector references, and move/emplace operations?
    //TODO: Support inserting multiple values
    void insert(std::size_t axis, std::size_t position/*=end,multi_vector(*_ref?*)<T, N_dims-1> newValues=array_of_default_T*/);
    
    auto operator[](std::size_t const i) -> decltype(auto)
    {
        return op_index_proxy<N_dims,T>{data_, this}[i];
    }
    
    auto operator[](std::size_t const i) const -> decltype(auto)
    {
        return op_index_proxy<N_dims,T const>{data_, this}[i];
    }
    
    bool empty() const noexcept
    {
        return total_size_by_dim(0) == 0;
    }
    std::array<std::size_t, N_dims> const &size() const noexcept {
        return size_;
    }
    std::array<std::size_t, N_dims> const &capacity() const noexcept {
        return capacity_;
    }
    /*
    simple_iterator_proxy begin()
    {
        
    }
    simple_iterator_proxy end()
    {
        //TODO;
    }
    */
    
    simple_const_iterator_proxy begin() const
    {
        return simple_const_iterator_proxy{this, 0};
    }
    simple_const_iterator_proxy end() const
    {
        return simple_const_iterator_proxy{this, total_size_by_dim(0)};
    }
    
    bool operator==(multi_vector const &o) const {
        return boost::equal(*this, o);
    }

    //Standard Container Typedefs and Accessors:
    //(maybe?, or do we deliberately avoid them to stop the multi_vector
    //from being accidentally used in a single-dimensional context?)
    //...
    //Standard Container Acessors on per-dimension basis:
    //... (provided by free functions? -- see below)
};

//Rotation Proxies
//...
//multi_vector_ref<T, dimenstions> rotated(multi_vector<T, dimenstions>, std::array<int, N_Dims*2> transform_descriptions)
//                               {mirroring...,  new axis order... (that is: a permutation, member of symmetric group)}
//Example transform_description: {1,-1,1,        1,0,2}:  a x b x c => -b x a x -c


//multi_vector_ref rotated_in_higher_dimensions(); // the same, assumes excess dimensions are at end of actual dimensions of array

//Iteration Proxies
//True iterates over that dimension, false does not.
//Examples:
//{true, true} -- iterate over every element (equivalent to the container interface on the multi-array itself)
//{true, false} -- iterate over rows
//{false, true} -- iterate over columns
//{false, false} -- iterate over nothing (single element iteration, which is ref to entire array)
//Extended in natural fashion to higher dimensions
//iter_proxy iterate_over(std::array<bool, dimensions>, multi_vector<T, dimensions>)
//

}//namespace hg


#endif //HG_MULTI_VECTOR_H
