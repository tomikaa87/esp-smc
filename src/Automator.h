#pragma once

#include <Logger.h>

#include <cstdint>

class ISystemClock;
class PersistentStorage;
class RelayController;
class Settings;

class Automator
{
public:
    Automator(const ISystemClock& clock,
              const Settings& settings,
              RelayController& relay);

    void task();

private:
    const Logger _log{ "Automator" };
    const ISystemClock& _systemClock;
    const Settings& _settings;
    RelayController& _relayController;

    uint32_t _lastCheckTime = 0;

    enum class LastShutterEvent
    {
        None,
        Open,
        Close
    };

    LastShutterEvent _lastShutterEvent = LastShutterEvent::None;

    void checkShutterSchedule();

    void openShutters();
    void closeShutters();
};