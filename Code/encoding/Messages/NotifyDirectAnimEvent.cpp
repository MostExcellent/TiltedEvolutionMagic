#include "NotifyDirectAnimEvent.h"
void NotifyDirectAnimEvent::SerializeRaw(TiltedPhoques::Buffer::Writer& aWriter) const noexcept
{
    Serialization::WriteVarInt(aWriter, gameId);
    Serialization::WriteString(aWriter, eventString);
}

void NotifyDirectAnimEvent::DeserializeRaw(TiltedPhoques::Buffer::Reader& aReader) noexcept
{
    ServerMessage::DeserializeRaw(aReader);

    gameId = Serialization::ReadVarInt(aReader) & 0xFFFFFFFF;
    eventString = Serialization::ReadString(aReader);
}
