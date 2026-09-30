#pragma once

struct IPostAnimationChannelUpdateFunctor
{
    virtual ~IPostAnimationChannelUpdateFunctor(); // 00
    
    virtual void DoPostAnimationChannelUpdate();  // 01
};
static_assert(sizeof(IPostAnimationChannelUpdateFunctor) == 0x8);
