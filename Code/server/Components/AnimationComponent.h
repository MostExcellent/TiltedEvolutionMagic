#pragma once

#ifndef TP_INTERNAL_COMPONENTS_GUARD
#error Include Components.h instead
#endif

#include <Structs/NetActionEvent.h>

struct AnimationComponent
{
    Vector<NetActionEvent> Actions;
    NetActionEvent CurrentAction;
    NetActionEvent LastSerializedAction;
};