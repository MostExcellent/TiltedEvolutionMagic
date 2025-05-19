#include "DirectAnimEventRequest.h"

void DirectAnimEventRequest::SerializeRaw(TiltedPhoques::Buffer::Writer& aWriter) const noexcept
{
    Serialization::WriteVarInt(aWriter, gameId);
    Serialization::WriteString(aWriter, eventString);
}

void DirectAnimEventRequest::DeserializeRaw(TiltedPhoques::Buffer::Reader& aReader) noexcept
{
    ClientMessage::DeserializeRaw(aReader);

    gameId = Serialization::ReadVarInt(aReader) & 0xFFFFFFFF;
    eventString = Serialization::ReadString(aReader);
}
