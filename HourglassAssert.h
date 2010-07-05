/*
 *  HourglassAssert.h
 *  HourglassIIGame
 *
 *  Created by Evan Wallace on 1/06/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
#ifndef HOURGLASS_ASSERT_H
#define HOURGLASS_ASSERT_H

#define BOOST_ENABLE_ASSERT_HANDLER
#include <boost/assert.hpp>
#define hg_assert(x) BOOST_ASSERT(x)
#endif //HOURGLASS_ASSERT_H
