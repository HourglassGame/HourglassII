#ifndef HG_ASSERTIONEXCEPTION_H
#define HG_ASSERTIONEXCEPTION_H
/*
 *  AssertionException.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 5/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

namespace hg {
    class AssertionException;
}

#include "Exception.h"
#include <string>
#include <stdexcept>

namespace hg {
    class AssertionException : virtual public std::logic_error::logic_error, public hg::Exception
    {
    public:
        AssertionException(const std::string&);
        AssertionException(const hg::AssertionException&);
        virtual ~AssertionException() throw();
    };
}

#endif //HG_ASSERTIONEXCEPTION_H
