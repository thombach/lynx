#pragma once
#include <Arduino.h>
#include <atomic>
#include <mutex>
#include "net/Http.h"

struct ManagerStatus
{
    const char *name;
    bool hasData;
    bool stale;
    uint32_t lastSuccessAgo; // ms since the last successful sync
    uint32_t failures;       // Consecutive failed syncs
    String lastError;
};

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

        failReason = "";
        if (fetch())
        {
            failures = 0;
            lastSuccess = now;
            succeeded = true;
            nextAttempt = now + interval;
            Serial.printf("[%s] Sync successful\n", name);
            return;
        }

        // Exponential backoff, never waiting longer than the normal interval
        uint32_t retry = min(RETRY_BASE_MS << min<uint32_t>(failures, 6), interval);
        failures++;
        nextAttempt = now + retry;
        {
            std::lock_guard<std::mutex> lock(statusMutex);
            lastError = failReason.isEmpty() ? Http::lastError() : failReason;
        }
        Serial.printf("[%s] Sync failed, retrying in %lus\n", name, (unsigned long)(retry / 1000));
    }

    bool hasData() const { return succeeded; }

    bool isStale() const
    {
        return !succeeded || millis() - lastSuccess > STALE_FACTOR * interval;
    }

    ManagerStatus status() const
    {
        std::lock_guard<std::mutex> lock(statusMutex);
        return {name, hasData(), isStale(), succeeded ? millis() - lastSuccess : 0, failures, lastError};
    }

protected:
    virtual bool fetch() = 0;

    // For failures other than HTTP errors, which are reported automatically
    bool fail(const char *reason)
    {
        failReason = reason;
        Serial.printf("[%s] %s\n", name, reason);
        return false;
    }

private:
    static constexpr uint32_t RETRY_BASE_MS = 10000;
    static constexpr uint32_t STALE_FACTOR = 3;

    const char *name;
    const uint32_t interval;
    uint32_t nextAttempt = 0;
    String failReason; // Only touched by the fetch task
    std::atomic<uint32_t> failures{0};
    std::atomic<uint32_t> lastSuccess{0};
    std::atomic<bool> succeeded{false};
    mutable std::mutex statusMutex;
    String lastError;
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
