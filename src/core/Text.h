#pragma once
// Pure text helpers, without Arduino dependencies so they can be unit tested natively
#include <cstring>
#include <string>

namespace core
{
    // GFX fonts only cover ASCII: transliterate common French characters (UTF-8)
    // and drop anything else outside ASCII
    inline std::string toAscii(const char *text)
    {
        static const struct
        {
            const char *utf8;
            const char *ascii;
        } MAP[] = {
            {"à", "a"}, {"â", "a"}, {"ä", "a"}, {"ç", "c"}, {"é", "e"}, {"è", "e"}, {"ê", "e"}, {"ë", "e"},
            {"î", "i"}, {"ï", "i"}, {"ô", "o"}, {"ö", "o"}, {"ù", "u"}, {"û", "u"}, {"ü", "u"}, {"œ", "oe"},
            {"À", "A"}, {"Â", "A"}, {"Ç", "C"}, {"É", "E"}, {"È", "E"}, {"Ê", "E"}, {"Î", "I"},
            {"Ô", "O"}, {"Ù", "U"}, {"Û", "U"}, {"Œ", "Oe"},
            {"’", "'"}, {"‘", "'"}, {"«", "\""}, {"»", "\""}, {"“", "\""}, {"”", "\""},
            {"–", "-"}, {"—", "-"}, {"…", "..."}, {"\xC2\xA0", " "}, {"\xE2\x80\xAF", " "},
        };

        std::string out;
        if (!text)
        {
            return out;
        }
        while (*text)
        {
            if ((unsigned char)*text < 0x80)
            {
                out += *text == '\n' || *text == '\r' || *text == '\t' ? ' ' : *text;
                text++;
                continue;
            }

            bool replaced = false;
            for (const auto &entry : MAP)
            {
                size_t length = strlen(entry.utf8);
                if (strncmp(text, entry.utf8, length) == 0)
                {
                    out += entry.ascii;
                    text += length;
                    replaced = true;
                    break;
                }
            }
            if (!replaced)
            {
                // Skip the whole unknown UTF-8 sequence
                text++;
                while (((unsigned char)*text & 0xC0) == 0x80)
                {
                    text++;
                }
            }
        }
        return out;
    }
}
