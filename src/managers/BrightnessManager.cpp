#include "BrightnessManager.h"
#include <Preferences.h>
#include "ConfigManager.h"

namespace
{
    constexpr const char *NVS_NAMESPACE = "lynx";
    constexpr const char *NVS_KEY = "brightness";
}

void BrightnessManager::begin()
{
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false); // Read-only fails if the namespace doesn't exist yet
    int fallback = lroundf(ConfigManager::getInstance().brightness * LEVELS);
    level = constrain(prefs.getInt(NVS_KEY, fallback), 1, LEVELS);
    prefs.end();
}

void BrightnessManager::cycle()
{
    level = level >= LEVELS ? 1 : level + 1;

    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putInt(NVS_KEY, level);
    prefs.end();
}

bool BrightnessManager::wake()
{
    if (!isNight())
    {
        return false;
    }
    bool wasAwake = wakeTimer.isRunning();
    wakeTimer.start(WAKE_MS);
    return !wasAwake;
}

float BrightnessManager::current()
{
    if (isNight() && !wakeTimer.isRunning())
    {
        return ConfigManager::getInstance().nightBrightness;
    }
    return userLevel();
}

bool BrightnessManager::isNight() const
{
    const ConfigManager &config = ConfigManager::getInstance();
    struct tm now;
    if (!config.hasNightMode() || !time.now(now))
    {
        return false;
    }

    int minutes = now.tm_hour * 60 + now.tm_min;
    if (config.nightStart < config.nightEnd)
    {
        return minutes >= config.nightStart && minutes < config.nightEnd;
    }
    return minutes >= config.nightStart || minutes < config.nightEnd; // Spans midnight
}
