#pragma once

#include "hkRefPtr.h"
#include "hkReferencedObject.h"

struct hkRefVariant : hkRefPtr<hkReferencedObject> {};
static_assert(sizeof(hkRefVariant) == 0x8);
