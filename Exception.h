#ifndef HG_EXCEPTION_H
#define HG_EXCEPTION_H
/*
 *  Exception.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 5/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

namespace hg {
    class Exception;
}
#include <exception>
namespace hg {
    class Exception : virtual public std::exception {
        virtual const char* what() const throw()
        {
            return "Hourglass Exception";
        }
    };
}
#endif //HG_EXCEPTION_H
