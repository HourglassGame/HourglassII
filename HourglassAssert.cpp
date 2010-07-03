/*
 *  AllegroAssert.cpp
 *  HourglassIIGame
 *
 *  Created by Evan Wallace on 1/06/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "HourglassAssert.h"
#include <string>
#include <sstream>
#include <exception>
#include "Tracer.h"
#include "Logger.h"
void boost::assertion_failed(char const * const expr,
                             char const * const function, 
                             char const * const file, const long line) {
    HG_TRACE_FUNCTION
	using std::endl;
	std::stringstream message;
	message << "<---Assertion Failed--->" << endl;
	message << "Assertion: " << "(" << expr << ")" << endl;
	message << "Function: " << function << endl;
	message << "File: " << file << endl;
	message << "Line: " << line;
    hg::Logger::GetLogger().Log(std::string(message.str()), hg::loglevel::FATAL);
    throw std::logic_error(expr);
}
