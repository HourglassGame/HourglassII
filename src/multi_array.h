#ifndef HG_MULTI_ARRAY_H
#define HG_MULTI_ARRAY_H
#include <memory>
#include <array>
#include <boost/range/numeric.hpp>
#include <functional>
#include <boost/container/vector.hpp>
namespace hg {
template<typename T, std::size_t N_dims, typename Allocator = std::allocator<T>>
class multi_array {
private:
    struct dummy {};
public:
    typedef std::size_t index;
    template<std::size_t remaining_dims, typename unused = dummy>
    friend struct op_index_proxy;
    template<std::size_t remaining_dims, typename unused = dummy>
    struct op_index_proxy {
    //private:
        friend class multi_array;
        T *array_start;
        multi_array const *this_;
    //public:
        op_index_proxy<remaining_dims-1> operator[](std::size_t i) {
            return op_index_proxy<remaining_dims-1>{array_start + this_->capacity[N_dims - remaining_dims]*i, this_};
        }
    };
    template<typename unused>
    struct op_index_proxy<1,unused> {
    //private:
        friend class multi_array;
        T *array_start;
        multi_array const *this_;
    //public:
        T &operator[](std::size_t i) {
            return array_start[i];
        }
    };
    
    template<std::size_t remaining_dims, typename unused = dummy>
    friend struct const_op_index_proxy;
    template<std::size_t remaining_dims, typename unused = dummy>
    struct const_op_index_proxy {
    //private:
        friend class multi_array;
        T const *array_start;
        multi_array const *this_;
    //public:
        const_op_index_proxy<remaining_dims-1> operator[](std::size_t i) const {
            return op_index_proxy<remaining_dims-1>{array_start + this_->capacity[N_dims - remaining_dims]*i, this_};
        }
    };
    template<typename unused>
    struct const_op_index_proxy<1,unused> {
    //private:
        friend class multi_array;
        T const *array_start;
        multi_array const *this_;
    //public:
        T const &operator[](std::size_t i) const {
            return array_start[i];
        }
    };

    multi_array() noexcept = default;
    template<typename RandomAccessRange>
    multi_array(std::array<std::size_t, N_dims> const &size, RandomAccessRange const &data);
    
    //template<typename NDimArray>
    //multi_array(NDimArray const& toCopy); TODO
    //Default Copy/assignment/move/dtor:
    //...
    
    //Giving size and inserting values
    void resize(std::array<std::size_t, N_dims> const &newSize) {
        //TODO, allow arbitrary resizing etc
        assert(data.empty() && "Missing Feature! For now, we only allow single initialiseation.");
        std::size_t newStorageSpace = boost::accumulate(newSize, 1, std::multiplies<std::size_t>{});
        data.resize(newStorageSpace);
        size = newSize;
        capacity = newSize;
    }
    void insert(std::size_t axis, std::size_t position/*=end,array_with_compatible_size newValues=array_of_default_T*/);
    
    op_index_proxy<N_dims-1> operator[](std::size_t i) {
        return op_index_proxy<N_dims-1>{data.data() + capacity[0]*i, this};
    }
    
    const_op_index_proxy<N_dims-1> operator[](std::size_t i) const {
        return const_op_index_proxy<N_dims-1>{data.data() + capacity[0]*i, this};
    }
    //TODO - T& operator[] for 1 dimensional case
    
    //Rename to "size" or "shape" maybe?
    std::array<std::size_t, N_dims> const &extents() const {
        return size;
    }
    
    //Standard Container Typedefs and Accessors:
    //(maybe?, or do we deliberately avoid them to stop the multi_array
    //from being accidentally used in a single-dimensional context?)
    //...
    //Standard Container Acessors on per-dimension basis:
    //... (provided by free functions? -- see below)
private:
    //something storage_order
    std::array<std::size_t, N_dims> size;
    std::array<std::size_t, N_dims> capacity;
    boost::container::vector<T, Allocator> data;
};

//Rotation Proxies
//...
//multi_array_ref<T, dimenstions> rotated(multi_array<T, dimenstions>, std::array<int, N_Dims*2> transform_descriptions)
//                               {mirroring...,  new axis order... (that is: a permutation, member of symmetric group)}
//Example transform_description: {1,-1,1,        1,0,2}:  a x b x c => -b x a x -c


//multi_array_ref rotated_in_higher_dimensions(); // the same, assumes excess dimensions are at end of actual dimensions of array

//Iteration Proxies
//True iterates over that dimension, false does not.
//Examples:
//{true, true} -- iterate over every element (equivalent to the container interface on the multi-array itself)
//{true, false} -- iterate over rows
//{false, true} -- iterate over columns
//{false, false} -- iterate over nothing (single element iteration, which is ref to entire array)
//Extended in natural fashion to higher dimensions
//iter_proxy iterate_over(std::array<bool, dimensions>, multi_array<T, dimensions>)
//

}//namespace HG


#endif //HG_MULTI_ARRAY_H
