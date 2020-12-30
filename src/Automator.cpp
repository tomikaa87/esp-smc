#include "Automator.h"
#include "Config.h"
#include "RelayController.h"
#include "Settings.h"

#include <ISystemClock.h>

#include <Arduino.h>
#include <ctime>
#include <EEPROM.h>

Automator::Automator(
    const ISystemClock& clock,
    const Settings& settings,
    RelayController& relay
)
    : _log{ "Automator" }
    , _systemClock(clock)
    , _settings(settings)
    , _relayController(relay)
{
    _log.info("loaded open time: from=%d:%02d, to=%d:%02d, active=%d",
        _settings.data.ShutterOpenHour,
        _settings.data.ShutterOpenMinute,
        _settings.data.ShutterCloseHour,
        _settings.data.ShutterCloseMinute,
        _settings.data.ShutterTimerActive
    );
}

void Automator::task()
{
    if (::millis() - _lastCheckTime < Config::Automator::CheckTimerInterval)
        return;

    _lastCheckTime = ::millis();

    checkShutterSchedule();
}

void Automator::checkShutterSchedule()
{
    const auto& cfg = _settings.data;

    if (!cfg.ShutterTimerActive)
        return;

    if (cfg.ShutterCloseHour == cfg.ShutterOpenHour && cfg.ShutterCloseMinute == cfg.ShutterOpenMinute)
        return;

    if (!_systemClock.isSynchronized())
        return;

    const auto epochTime = _systemClock.localTime();
    const auto tm = std::gmtime(&epochTime);

    if (_lastShutterEvent != LastShutterEvent::Open &&
        tm->tm_hour == cfg.ShutterCloseHour && tm->tm_min == cfg.ShutterCloseMinute)
    {
        _log.debug("Shutter closing time point reached: %02d:%02d",
                    tm->tm_hour, tm->tm_min);

        _lastShutterEvent = LastShutterEvent::Open;
        closeShutters();
    }

    if (_lastShutterEvent != LastShutterEvent::Close &&
        tm->tm_hour == cfg.ShutterOpenHour && tm->tm_min == cfg.ShutterOpenMinute)
    {
        _log.debug("Shutter opening time point reached: %02d:%02d",
                    tm->tm_hour, tm->tm_min);

        _lastShutterEvent = LastShutterEvent::Close;
        openShutters();
    }
}

void Automator::openShutters()
{
    _log.info("Opening shutters");

    _relayController.pulse(0);
    _relayController.pulse(2);
}

void Automator::closeShutters()
{
    _log.info("Closing shutters");

    _relayController.pulse(1);
    _relayController.pulse(3);
}
