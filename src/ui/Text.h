#pragma once
#include <Arduino.h>
#include "core/Text.h"

inline String toAscii(const String &text)
{
    return String(core::toAscii(text.c_str()).c_str());
}
