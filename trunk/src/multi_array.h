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
    //something storage_order; //maybe?
    boost::container::vector<T, Allocator> data;
    std::array<std::size_t, N_dims> size;
    std::array<std::size_t, N_dims> capacity;
public:
    typedef std::size_t index;
    template<std::size_t remaining_dims, typename U>
    friend struct op_index_proxy;
    template<std::size_t remaining_dims, typename U>
    struct op_index_proxy {
    //private:
        U *const array_start;
        multi_array const *const this_;
        static std::size_t const current_axis = N_dims - remaining_dims;
    //public:
        op_index_proxy<remaining_dims-1,U> operator[](std::size_t i) const {
            assert(current_axis < this_->size.size());
            assert(i < this_->size[current_axis]);
            auto current_block_length =
                std::accumulate(
                    &this_->capacity[current_axis+1],
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
        multi_array const *const this_;
        static std::size_t const current_axis = N_dims - 1;
    //public:
        U &operator[](std::size_t i) const {
            assert(current_axis < this_->size.size());
            assert(i < this_->size[current_axis]);
            assert(array_start+i >= this_->data.data() && array_start+i < &this_->data[this_->data.size()]);
            return array_start[i];
        }
    };

    multi_array() noexcept = default;
    template<typename RandomAccessRange>
    multi_array(std::array<std::size_t, N_dims> const &size, RandomAccessRange const &data);
    
    //template<typename NDimArray>
    //multi_array(NDimArray const& toCopy); TODO
    //Copy/assignment/move/dtor: //(Is default good enough, or do we want better exception safety?)
    //...
    
    //Giving size and inserting values
    void resize(std::array<std::size_t, N_dims> const &newSize) {
        //TODO, allow arbitrary resizing etc
        assert(data.empty() && "Missing Feature! For now, we only allow single initialiseation.");
        std::size_t newStorageSpace = boost::accumulate(newSize, 1, std::multiplies<std::size_t>{});
        data.resize(newStorageSpace);
        size = newSize;
        capacity = newSize;
        //std::cout << (void*)this << "newSize: " << newSize << "\n";
    }
    void insert(std::size_t axis, std::size_t position/*=end,array_with_compatible_size newValues=array_of_default_T*/);
    
    auto operator[](std::size_t i)
   -> decltype(op_index_proxy<N_dims,T>{data.data(), this}[i])
    {
        return op_index_proxy<N_dims,T>{data.data(), this}[i];
    }
    
    auto operator[](std::size_t i) const
   -> decltype(op_index_proxy<N_dims,T const>{data.data(), this}[i])
    {
        return op_index_proxy<N_dims,T const>{data.data(), this}[i];
    }
    
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
