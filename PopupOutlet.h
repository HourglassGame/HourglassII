#ifndef HG_POPUPOUTLET_H
#define HG_POPUPOUTLET_H
/*
 *  PopupOutlet.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 4/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

namespace hg {
    class PopupOutlet;
}

#include "Outlet.h"
#include "LogLevel.h"
#include <boost/noncopyable.hpp>
#include "EngineThreadMediator.h"
namespace hg {
    class PopupOutlet : public Outlet, private boost::noncopyable
    {
    public:
        PopupOutlet(EngineThreadMediator& mediator, loglevel::LogLevel level);
    private:
        void Log_(const std::string& message, loglevel::LogLevel importance);
        loglevel::LogLevel level_;
        EngineThreadMediator& mediator_;
    };
}
#endif //HG_POPUPOUTLET_H
