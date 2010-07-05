#ifndef HG_ACTION_H
#define HG_ACTION_H
/*
 *  Action.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 5/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
namespace hg {
class Action
{
public:
    Action()
    {
    }
    virtual ~Action() = 0;
    virtual void operator()() = 0;
};
}
#endif //HG_ACTION_H
