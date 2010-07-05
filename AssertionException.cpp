/*
 *  AssertionException.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 5/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "AssertionException.h"
using namespace hg;
AssertionException::AssertionException(const std::string& description) :
logic_error(description)
{
}

AssertionException::AssertionException(const hg::AssertionException& ae) :
logic_error(ae)
{    
}


AssertionException::~AssertionException() throw()
{
}
