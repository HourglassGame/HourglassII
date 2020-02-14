#ifndef HG_COPY_AS_NEW_PTR_H
#define HG_COPY_AS_NEW_PTR_H
namespace hg{
//The behaviour of this class is somewhat strange and not particularly useful
//in general, so the use of this class should be limited to cases where there
//are no better options.
template<typename T>
class copy_as_new_ptr final
{
public:
    copy_as_new_ptr() :
        ptr(new T())
    {
    }
    //Notice that these do not actually copy their arguments.
    //The arguments are not even moved. This is deliberate.
    copy_as_new_ptr(copy_as_new_ptr const &) :
        ptr(new T())
    {
    }
    copy_as_new_ptr &operator=(copy_as_new_ptr const&)
    {
        return *this;
    }

    //Move operations are not useful, because they cannot be made no-throw.
    copy_as_new_ptr(copy_as_new_ptr &&o) = delete;
    copy_as_new_ptr &operator=(copy_as_new_ptr &&o) = delete;
    ~copy_as_new_ptr() noexcept
    {
        delete ptr;
    }
    T &operator*() const
    {
        return get();
    }
    T *operator->() const
    {
        return get();
    }
    T *get() const
    {
        Expects(ptr);
        return ptr;
    }
private:
    T *ptr;
};
}
#endif
