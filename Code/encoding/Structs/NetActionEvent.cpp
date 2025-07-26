#include <Structs/NetActionEvent.h>

using TiltedPhoques::Serialization;

namespace
{
enum DifferentialFlags : uint16_t
{
    kActorHandle = 1 << 0,
    kTargetHandle = 1 << 1,
    kTargetGameId = 1 << 2,
    kActionId = 1 << 3,
    kSequenceId = 1 << 4,
    kIdleId = 1 << 5,
    kState = 1 << 6,
    kType = 1 << 7,
    kEventName = 1 << 8,
    kTargetEventName = 1 << 9,
    kVariables = 1 << 10,
    kSequenceIndex = 1 << 11
};
}

bool NetActionEvent::operator==(const NetActionEvent& acRhs) const noexcept
{
    return Tick == acRhs.Tick &&
           ActorId == acRhs.ActorId &&
           TargetId == acRhs.TargetId &&
           TargetGameId == acRhs.TargetGameId &&
           ActionId == acRhs.ActionId &&
           SequenceId == acRhs.SequenceId &&
           IdleId == acRhs.IdleId &&
           State1 == acRhs.State1 &&
           State2 == acRhs.State2 &&
           Type == acRhs.Type &&
           SequenceIndex == acRhs.SequenceIndex &&
           EventName == acRhs.EventName &&
           TargetEventName == acRhs.TargetEventName &&
           Variables == acRhs.Variables;
}

bool NetActionEvent::operator!=(const NetActionEvent& acRhs) const noexcept
{
    return !operator==(acRhs);
}

void NetActionEvent::GenerateDifferential(const NetActionEvent& aPrevious,
                                          TiltedPhoques::Buffer::Writer& aWriter) const noexcept
{
    uint16_t flags = 0;

    if (ActorId != aPrevious.ActorId)
        flags |= kActorHandle;

    if (TargetId != aPrevious.TargetId)
        flags |= kTargetHandle;

    if (TargetGameId != aPrevious.TargetGameId)
        flags |= kTargetGameId;

    if (ActionId != aPrevious.ActionId)
        flags |= kActionId;

    if (SequenceId != aPrevious.SequenceId)
        flags |= kSequenceId;

    if (IdleId != aPrevious.IdleId)
        flags |= kIdleId;

    if (State1 != aPrevious.State1 || State2 != aPrevious.State2)
        flags |= kState;

    if (Type != aPrevious.Type)
        flags |= kType;

    if (EventName != aPrevious.EventName)
        flags |= kEventName;

    if (TargetEventName != aPrevious.TargetEventName)
        flags |= kTargetEventName;

    if (Variables != aPrevious.Variables)
        flags |= kVariables;

    if (SequenceIndex != aPrevious.SequenceIndex)
        flags |= kSequenceIndex;

    // Write flags
    aWriter.WriteBits(flags, 16);

    // Always write tick difference
    {
        const auto tickDiff = Tick - aPrevious.Tick;
        Serialization::WriteVarInt(aWriter, tickDiff);
    }

    if (flags & kActorHandle)
    {
        Serialization::WriteVarInt(aWriter, ActorId);
    }

    if (flags & kTargetHandle)
    {
        Serialization::WriteVarInt(aWriter, TargetId);
    }

    if (flags & kTargetGameId)
    {
        TargetGameId.Serialize(aWriter);
    }

    if (flags & kActionId)
    {
        ActionId.Serialize(aWriter);
    }

    if (flags & kSequenceId)
    {
        SequenceId.Serialize(aWriter);
    }

    if (flags & kIdleId)
    {
        IdleId.Serialize(aWriter);
    }

    if (flags & kState)
    {
        aWriter.WriteBits(State1, 32);
        aWriter.WriteBits(State2, 32);
    }

    if (flags & kType)
    {
        Serialization::WriteVarInt(aWriter, Type);
    }

    if (flags & kEventName)
    {
        EventName.Serialize(aWriter);
    }

    if (flags & kTargetEventName)
    {
        TargetEventName.Serialize(aWriter);
    }

    if (flags & kVariables)
    {
        Variables.GenerateDiff(aPrevious.Variables, aWriter);
    }

    if (flags & kSequenceIndex)
    {
        Serialization::WriteVarInt(aWriter, SequenceIndex);
    }
}

void NetActionEvent::ApplyDifferential(TiltedPhoques::Buffer::Reader& aReader) noexcept
{
    uint64_t flags = 0;

    aReader.ReadBits(flags, 16);

    // Always read tick difference
    {
        const auto tickDiff = Serialization::ReadVarInt(aReader);
        Tick += tickDiff;
    }

    if (flags & kActorHandle)
    {
        ActorId = Serialization::ReadVarInt(aReader) & 0xFFFFFFFF;
    }

    if (flags & kTargetHandle)
    {
        TargetId = Serialization::ReadVarInt(aReader) & 0xFFFFFFFF;
    }

    if (flags & kTargetGameId)
    {
        TargetGameId.Deserialize(aReader);
    }

    if (flags & kActionId)
    {
        ActionId.Deserialize(aReader);
    }

    if (flags & kSequenceId)
    {
        SequenceId.Deserialize(aReader);
    }

    if (flags & kIdleId)
    {
        IdleId.Deserialize(aReader);
    }

    if (flags & kState)
    {
        uint64_t tmp = 0;
        aReader.ReadBits(tmp, 32);
        State1 = tmp & 0xFFFFFFFF;
        aReader.ReadBits(tmp, 32);
        State2 = tmp & 0xFFFFFFFF;
    }

    if (flags & kType)
    {
        Type = Serialization::ReadVarInt(aReader) & 0xFFFFFFFF;
    }

    if (flags & kEventName)
    {
        EventName.Deserialize(aReader);
    }

    if (flags & kTargetEventName)
    {
        TargetEventName.Deserialize(aReader);
    }

    if (flags & kVariables)
    {
        Variables.ApplyDiff(aReader);
    }

    if (flags & kSequenceIndex)
    {
        SequenceIndex = Serialization::ReadVarInt(aReader) & 0xFFFFFFFF;
    }
}
