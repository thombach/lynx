#pragma once
#include <Arduino.h>

// GFX fonts only cover ASCII: replace common French accented letters (UTF-8)
inline String toAscii(const String &text)
{
    static const struct
    {
        const char *utf8;
        char ascii;
    } MAP[] = {
        {"à", 'a'}, {"â", 'a'}, {"ä", 'a'}, {"ç", 'c'}, {"é", 'e'}, {"è", 'e'}, {"ê", 'e'}, {"ë", 'e'},
        {"î", 'i'}, {"ï", 'i'}, {"ô", 'o'}, {"ö", 'o'}, {"ù", 'u'}, {"û", 'u'}, {"ü", 'u'},
        {"À", 'A'}, {"Â", 'A'}, {"Ç", 'C'}, {"É", 'E'}, {"È", 'E'}, {"Ê", 'E'}, {"Î", 'I'},
        {"Ô", 'O'}, {"Ù", 'U'}, {"Û", 'U'},
    };

    String result = text;
    for (const auto &entry : MAP)
    {
        result.replace(entry.utf8, String(entry.ascii));
    }
    return result;
}
