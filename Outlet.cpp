/*
 *  Outlet.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 1/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "Outlet.h"

using namespace hg;

Outlet::~Outlet()
{
}

void Outlet::Log(const std::string& message, const loglevel::LogLevel importance)
{
    Log_(message, importance);
}

//Needed to use in ptr_container; however, cloning is generally meaningless for outlets, which usually 
//refer to specific hardware outputs which cannot be programmatically cloned.
Outlet* new_clone(const Outlet&)
{
    hg_assert(false);
    return 0;
}
