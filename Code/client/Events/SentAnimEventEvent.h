#pragma once

struct SentAnimEventEvent
{
    SentAnimEventEvent(uint32_t aFormID, String aEventName)
        : formId(aFormID)
        , eventName(aEventName)
    {
    }

    uint32_t formId;
    String eventName;
};
