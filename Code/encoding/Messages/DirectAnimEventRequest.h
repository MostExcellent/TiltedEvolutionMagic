#pragma once

#include "Message.h"
#include "Structs/CachedString.h"

struct DirectAnimEventRequest final : ClientMessage
{
    static constexpr ClientOpcode Opcode = kDirectAnimEventRequest;

    DirectAnimEventRequest()
        : ClientMessage(Opcode)
    {
    }

    void SerializeRaw(TiltedPhoques::Buffer::Writer& aWriter) const noexcept override;
    void DeserializeRaw(TiltedPhoques::Buffer::Reader& aReader) noexcept override;

    bool operator==(const DirectAnimEventRequest& acRhs) const { return gameId == acRhs.gameId && eventString == acRhs.eventString; }

    uint32_t gameId{};
    CachedString eventString{};
};
