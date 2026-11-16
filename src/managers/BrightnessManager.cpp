#include "BrightnessManager.h"
#include <Preferences.h>
#include "ConfigManager.h"
#include "core/TimeMath.h"

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

void BrightnessManager::step(int delta)
{
    setLevel(level + delta);
}

void BrightnessManager::setUserLevel(float value)
{
    setLevel(lroundf(value * LEVELS));
}

void BrightnessManager::setLevel(int value)
{
    value = constrain(value, 1, LEVELS);
    if (value == level)
    {
        return;
    }
    level = value;

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

    return core::inWindow(now.tm_hour * 60 + now.tm_min, config.nightStart, config.nightEnd);
}
