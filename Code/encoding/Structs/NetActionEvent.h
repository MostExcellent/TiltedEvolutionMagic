#pragma once

#include <cstdint>
#include "AnimationVariables.h"
#include "CachedString.h"
#include "GameId.h"

using TiltedPhoques::String;

/**
 * Network-transmission-specific version of ActionEvent that uses entity Ids and GameIds
 */
struct NetActionEvent
{
    // Time and state data
    uint64_t Tick{0};
    uint32_t State1{0};
    uint32_t State2{0};
    uint32_t Type{0};

    // Not sure if this varies across clients
    // Has not been a problem in testing, so far
    uint32_t SequenceIndex{0};

    // Actor (always an entity unless something is seriously wrong)
    uint32_t ActorId{0};

    // Target (not sure if they are always entities)
    uint32_t TargetId{0};
    GameId TargetGameId{};

    // Form IDs that need GameId for mod compatibility
    GameId ActionId{};
    GameId SequenceId{};
    GameId IdleId{};

    // String data
    CachedString EventName{};
    CachedString TargetEventName{};
    AnimationVariables Variables{};

    NetActionEvent() = default;
    NetActionEvent(const NetActionEvent& acRhs) = default;
    NetActionEvent(NetActionEvent&& acRhs) = default;
    ~NetActionEvent() = default;
    NetActionEvent& operator=(const NetActionEvent& acRhs) = default;
    NetActionEvent& operator=(NetActionEvent&& acRhs) = default;

    bool operator==(const NetActionEvent& acRhs) const noexcept;
    bool operator!=(const NetActionEvent& acRhs) const noexcept;

    void GenerateDifferential(const NetActionEvent& aPrevious, TiltedPhoques::Buffer::Writer& aWriter) const noexcept;
    void ApplyDifferential(TiltedPhoques::Buffer::Reader& aReader) noexcept;
};
