#include <Structs/ReferenceUpdate.h>
#include <TiltedCore/Serialization.hpp>

using TiltedPhoques::Serialization;

bool ReferenceUpdate::operator==(const ReferenceUpdate& acRhs) const noexcept
{
    return UpdatedMovement == acRhs.UpdatedMovement && ActionEvents == acRhs.ActionEvents;
}

bool ReferenceUpdate::operator!=(const ReferenceUpdate& acRhs) const noexcept
{
    return !this->operator==(acRhs);
}

void ReferenceUpdate::Serialize(TiltedPhoques::Buffer::Writer& aWriter) const noexcept
{
    UpdatedMovement.Serialize(aWriter);

    Serialization::WriteVarInt(aWriter, ActionEvents.size());

    static const NetActionEvent defaultEvent{};
    const NetActionEvent* previous = nullptr;
    for (const auto& entry : ActionEvents)
    {
        entry.GenerateDifferential(previous ? *previous : defaultEvent, aWriter);
        previous = &entry;
    }
}

void ReferenceUpdate::Deserialize(TiltedPhoques::Buffer::Reader& aReader)
{
    UpdatedMovement.Deserialize(aReader);

    const auto count = Serialization::ReadVarInt(aReader);

    ActionEvents.resize(count);

    for (auto i = 0u; i < count; ++i)
    {
        ActionEvents[i].ApplyDifferential(aReader);
    }
}
