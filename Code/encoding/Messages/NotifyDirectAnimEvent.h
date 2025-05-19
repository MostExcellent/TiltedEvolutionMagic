#pragma once

#include "Message.h"
#include "Opcodes.h"
#include "Structs/CachedString.h"

struct NotifyDirectAnimEvent final : ServerMessage
{
    static constexpr ServerOpcode Opcode = kNotifyDirectAnimEvent;

    NotifyDirectAnimEvent()
        : ServerMessage(Opcode)
    {
    }

    void SerializeRaw(TiltedPhoques::Buffer::Writer& aWriter) const noexcept override;
    void DeserializeRaw(TiltedPhoques::Buffer::Reader& aReader) noexcept override;

    bool operator==(const NotifyDirectAnimEvent& acRhs) const { return gameId == acRhs.gameId && eventString == acRhs.eventString; }

    uint32_t gameId{};
    CachedString eventString{};
};
