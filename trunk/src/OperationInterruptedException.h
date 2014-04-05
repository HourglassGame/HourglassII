#ifndef HG_OPERATION_INTERRUPTED_EXCEPTION_H
#define HG_OPERATION_INTERRUPTED_EXCEPTION_H
#include <exception>
namespace hg {
    struct OperationInterruptedException : std::exception {};
}
#endif //HG_OPERATION_INTERRUPTED_EXCEPTION_H
