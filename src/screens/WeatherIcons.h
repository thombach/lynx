#pragma once
#include <Arduino.h>
#include "bitmap.h"

// Weather icons fit in a 16x16 slot
constexpr int16_t WEATHER_ICON_SLOT = 16;

// Icon for a WMO weather code
inline const Bitmap &weatherIcon(int code, bool night)
{
    switch (code)
    {
    case 0:
        return night ? bitmap_clear_2 : bitmap_clear_1;
    case 1:
    case 2:
        return night ? bitmap_cloudy_2 : bitmap_cloudy_1;
    case 3:
    case 45:
    case 48:
        return bitmap_overcast;
    case 51:
    case 61:
    case 80:
        return bitmap_slight_rain;
    case 53:
    case 55:
    case 63:
    case 65:
    case 81:
    case 82:
        return bitmap_heavy_rain;
    case 56:
    case 66:
    case 71:
    case 85:
        return bitmap_slight_snow;
    case 57:
    case 67:
    case 73:
    case 75:
    case 77:
    case 86:
        return bitmap_heavy_snow;
    case 95:
    case 96:
    case 99:
        return bitmap_thunder;
    default:
        return bitmap_clear_1;
    }
}
