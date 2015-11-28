#ifndef HG_MULTI_VECTOR_H
#define HG_MULTI_VECTOR_H
#include <memory>
#include <array>
#include <boost/range/numeric.hpp>
#include <functional>
#include <boost/container/vector.hpp>
#include <boost/operators.hpp>
#include <boost/range/algorithm/equal.hpp>
namespace hg {

template<typename T, std::size_t N_dims, typename Allocator = std::allocator<T>>
class multi_vector {
private:
    T &getAtPos(std::size_t pos)
    {
        T &elem(*data.data());
        for (size_t dim{ 0 }; dim != N_dims-1; ++dim)
        {
            auto const current_block_physical_length(total_capacity_by_dim(dim + 1));
            auto const current_block_logical_length(total_size_by_dim(dim + 1));

            //Deliberate integer truncation.
            std::size_t block_num = pos / current_block_logical_length;
            pos -= block_num * current_block_logical_length;
            elem += block_num * current_block_physical_length;
        }
        assert(pos < data.size());
        return elem[pos];
    }

    T const &getAtPos(std::size_t pos) const
    {
        T const *elem(data.data());
        for (size_t dim{ 0 }; dim != N_dims - 1; ++dim)
        {
            auto const current_block_physical_length(total_capacity_by_dim(dim + 1));
            auto const current_block_logical_length(total_size_by_dim(dim + 1));

            //Deliberate integer truncation.
            std::size_t block_num = pos / current_block_logical_length;
            pos -= block_num * current_block_logical_length;
            elem += block_num * current_block_physical_length;
        }
        assert(pos < data.size());
        return elem[pos];
    }


    std::size_t total_capacity_by_dim(size_t dim) const
    {
        return std::accumulate(
                capacity.begin()+dim,
                capacity.end(),
                std::size_t(1),
                std::multiplies<std::size_t>());
    }

    std::size_t total_size_by_dim(size_t dim) const
    {
        return std::accumulate(
            capacity.begin() + dim,
            capacity.end(),
            std::size_t(1),
            std::multiplies<std::size_t>());
    }

    //something storage_order; //maybe?
    boost::container::vector<T, Allocator> data;
    std::array<std::size_t, N_dims> size;
    std::array<std::size_t, N_dims> capacity; //[2,3,4] stored as {{{a,a,a,a},{a,a,a,a},{a,a,a,a}}, {{a,a,a,a},{a,a,a,a},{a,a,a,a}}}
public:
    class simple_const_iterator_proxy : boost::totally_ordered<simple_const_iterator_proxy>
    {
    private:
        multi_vector const *const this_;
        std::size_t pos;

    public:
        simple_const_iterator_proxy(multi_vector const *this_, std::size_t pos) : this_(this_), pos(pos) {}

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
        bool operator<(simple_const_iterator_proxy o)
        {
            return pos < o.pos;
        }
        
        simple_const_iterator_proxy &operator+=(difference_type dif)
        {
            pos += dif;
            return *this;
        }

        simple_const_iterator_proxy &operator-=(difference_type dif)
        {
            pos -= dif;
            return *this;
        }

        simple_const_iterator_proxy &operator+(difference_type dif)
        {
            return simple_const_iterator_proxy(*this) += dif;
        }

        simple_const_iterator_proxy &operator-(difference_type dif)
        {
            return simple_const_iterator_proxy(*this) -= dif;
        }

        simple_const_iterator_proxy operator+(simple_const_iterator_proxy r)
        {
            return simple_const_iterator_proxy(*this) += r.pos;
        }

        simple_const_iterator_proxy operator-(simple_const_iterator_proxy r)
        {
            return simple_const_iterator_proxy(*this) -= r.pos;
        }
        
        reference operator[](difference_type dif)
        {
            return *(*this + dif);
        }
    };
    typedef simple_const_iterator_proxy const_iterator;

    typedef std::size_t index;
    template<std::size_t remaining_dims, typename U>
    friend struct op_index_proxy;
    template<std::size_t remaining_dims, typename U>
    struct op_index_proxy {
    //private:
        U *const array_start;
        multi_vector const *const this_;
        static std::size_t const current_axis = N_dims - remaining_dims;
    //public:
        op_index_proxy<remaining_dims-1,U> operator[](std::size_t i) const {
            assert(current_axis < this_->size.size());
            assert(i < this_->size[current_axis]);
            auto current_block_length =
                std::accumulate(
                    this_->capacity.begin()+current_axis+1,
                    this_->capacity.end(),
                    std::size_t(1),
                    std::multiplies<std::size_t>());

            return {array_start + current_block_length*i, this_};
        }
    };
    template<typename U>
    struct op_index_proxy<1,U> {
    //private:
        U *const array_start;
        multi_vector const *const this_;
        static std::size_t const current_axis = N_dims - 1;
    //public:
        U &operator[](std::size_t i) const {
            assert(current_axis < this_->size.size());
            assert(i < this_->size[current_axis]);
            assert(array_start+i >= this_->data.data() && array_start+i < &this_->data[this_->data.size()]);
            return array_start[i];
        }
    };

    multi_vector() noexcept :
        data(), size(), capacity()
    {
    }
    template<typename RandomAccessRange>
    multi_vector(std::array<std::size_t, N_dims> const &size, RandomAccessRange const &data);
    
    //template<typename NDimArray>
    //multi_vector(NDimArray const& toCopy); TODO
    //Copy/assignment/move/dtor: //(Is default good enough, or do we want better exception safety?)
    //...
    
    //Giving size and inserting values
    void resize(std::array<std::size_t, N_dims> const &newSize) {
        //TODO, allow arbitrary resizing etc
        assert(data.empty() && "Missing Feature! For now, we only allow single initialisation.");
        std::size_t newStorageSpace = boost::accumulate(newSize, static_cast<std::size_t>(1), std::multiplies<std::size_t>{});
        data.resize(newStorageSpace);
        size = newSize;
        capacity = newSize;
        //std::cout << (void*)this << "newSize: " << newSize << "\n";
    }
    void insert(std::size_t axis, std::size_t position/*=end,array_with_compatible_size newValues=array_of_default_T*/);
    
    auto operator[](std::size_t i) -> decltype(auto)
    {
        return op_index_proxy<N_dims,T>{data.data(), this}[i];
    }
    
    auto operator[](std::size_t i) const -> decltype(auto)
    {
        return op_index_proxy<N_dims,T const>{data.data(), this}[i];
    }
    
    //Rename to "size" or "shape" maybe?
    std::array<std::size_t, N_dims> const &extents() const {
        return size;
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
