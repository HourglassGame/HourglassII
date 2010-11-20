/*
 *  Level.cpp
 *  HourglassIIGameSFML_Wave
 *
 *  Created by Evan Wallace on 3/10/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "Level.h"
namespace hg {
Level::Level(unsigned int nspeedOfTime,
    unsigned int ntimeLineLength,
    const ::std::vector< ::std::vector<bool> >& nwallmap,
    int nwallSize,
    int ngravity,
    const ObjectList& ninitialObjects,
    const NewFrameID& nguyStartTime,
    const AttachmentMap& nattachmentMap,
    const TriggerSystem& ntriggerSystem) :
    speedOfTime(nspeedOfTime),
    timeLineLength(ntimeLineLength),
    wallmap(nwallmap),
    wallSize(nwallSize),
    gravity(ngravity),
    initialObjects(ninitialObjects),
    guyStartTime(nguyStartTime),
    attachmentMap(nattachmentMap),
    triggerSystem(ntriggerSystem)
    {
    }
}
