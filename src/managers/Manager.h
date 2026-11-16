#pragma once
#include <Arduino.h>
#include <atomic>
#include <mutex>

// Periodically fetches remote data. tick() runs on the background fetch task,
// so screens only ever read a published copy of the data.
class Manager
{
public:
    Manager(const char *name, uint32_t interval) : name(name), interval(interval) {}
    virtual ~Manager() = default;

    void tick()
    {
        uint32_t now = millis();
        if ((int32_t)(now - nextAttempt) < 0)
        {
            return;
        }

        if (fetch())
        {
            failures = 0;
            lastSuccess = now;
            succeeded = true;
            nextAttempt = now + interval;
            Serial.printf("[%s] Sync successful\n", name);
        }
        else
        {
            // Exponential backoff, never waiting longer than the normal interval
            uint32_t retry = RETRY_BASE_MS << min<uint32_t>(failures, 6);
            failures++;
            nextAttempt = now + min(retry, interval);
            Serial.printf("[%s] Sync failed, retrying in %lus\n", name, (unsigned long)(min(retry, interval) / 1000));
        }
    }

    bool hasData() const { return succeeded; }

    bool isStale() const
    {
        return !succeeded || millis() - lastSuccess > STALE_FACTOR * interval;
    }

protected:
    virtual bool fetch() = 0;

private:
    static constexpr uint32_t RETRY_BASE_MS = 10000;
    static constexpr uint32_t STALE_FACTOR = 3;

    const char *name;
    const uint32_t interval;
    uint32_t nextAttempt = 0;
    uint32_t failures = 0;
    std::atomic<uint32_t> lastSuccess{0};
    std::atomic<bool> succeeded{false};
};

template <typename T>
class DataManager : public Manager
{
public:
    using Manager::Manager;

    T get() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return data;
    }

protected:
    void publish(const T &value)
    {
        std::lock_guard<std::mutex> lock(mutex);
        data = value;
    }

private:
    mutable std::mutex mutex;
    T data{};
};
