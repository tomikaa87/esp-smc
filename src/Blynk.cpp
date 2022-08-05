/*
    This file is part of esp-thermostat.

    esp-thermostat is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    esp-thermostat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with esp-thermostat.  If not, see <http://www.gnu.org/licenses/>.

    Author: Tamas Karpati
    Created on 2020-12-21
*/

#ifdef IOT_ENABLE_BLYNK

#include "Blynk.h"
#include "RelayController.h"
#include "Settings.h"

#include <IBlynkHandler.h>
#include <WidgetTimeInput.h>

namespace VirtualPins
{
    namespace Sensors
    {
        static constexpr auto RoomTemperature = 80;
    }

    namespace Buttons
    {
        static constexpr auto Relay1 = 79;
        static constexpr auto Relay2 = 78;
        static constexpr auto Relay3 = 77;
        static constexpr auto Relay4 = 76;
    }

    namespace Timer
    {
        static constexpr auto OpenTime = 75;
        static constexpr auto Active = 74;
    }
}

Blynk::Blynk(
    IBlynkHandler& blynkHandler,
    Settings& settings,
    RelayController& relayController
)
    : _blynkHandler(blynkHandler)
    , _settings(settings)
    , _relayController(relayController)
{
    setupHandlers();
}

void Blynk::task()
{
}

void Blynk::updateRoomTemperature(const int16_t value)
{
    _lastRoomTemperature = value;

    _blynkHandler.writePin(
        VirtualPins::Sensors::RoomTemperature,
        Variant{ value / 100.0 }
    );
}

void Blynk::setupHandlers()
{
    _blynkHandler.setConnectedHandler([this] {
        onConnected();
    });

    //
    // Write handlers
    //

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::Buttons::Relay1,
        [this](const int pin, const Variant& value) {
            if (value == 1) {
                _relayController.pulse(0);
            }
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::Buttons::Relay2,
        [this](const int pin, const Variant& value) {
            if (value == 1) {
                _relayController.pulse(1);
            }
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::Buttons::Relay3,
        [this](const int pin, const Variant& value) {
            if (value == 1) {
                _relayController.pulse(2);
            }
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::Buttons::Relay4,
        [this](const int pin, const Variant& value) {
            if (value == 1) {
                _relayController.pulse(3);
            }
        }
    );

    _blynkHandler.setTimeInputPinHandler(
        VirtualPins::Timer::OpenTime,
        [this](const int, const TimeInputParam& p) {
            handleTimerInputChange(p);
        }
    );

    _blynkHandler.setPinWrittenHandler(
        VirtualPins::Timer::Active,
        [this](const int, const Variant& value) {
            _settings.data.ShutterTimerActive = static_cast<int>(value) ? 1 : 0;
            _log.debug("timer state changed: enabled=%d", _settings.data.ShutterTimerActive);
        }
    );

    //
    // Read handlers
    //

    _blynkHandler.setPinReadHandler(
        VirtualPins::Sensors::RoomTemperature,
        [this](const int) {
            return Variant{ _lastRoomTemperature / 100.0 };
        }
    );

    _blynkHandler.setSimplePinReadHandler(
        VirtualPins::Timer::OpenTime,
        [this](const int) {
            updateTimerPin();
        }
    );

    _blynkHandler.setPinReadHandler(
        VirtualPins::Timer::Active,
        [this](const int) {
            return Variant{ _settings.data.ShutterTimerActive ? 1 : 0 };
        }
    );
}

void Blynk::onConnected()
{
    updatePins();
    updateTimerPin();
}

void Blynk::handleTimerInputChange(const TimeInputParam& param)
{
    if (!param.hasStartTime())
    {
        _log.warning("Shutter open time input has no start time");
        return;
    }

    if (!param.hasStopTime())
    {
        _log.warning("Shutter open time input has no stop time");
        return;
    }

    _settings.data.ShutterOpenHour = param.getStartHour();
    _settings.data.ShutterOpenMinute = param.getStartMinute();
    _settings.data.ShutterCloseHour = param.getStopHour();
    _settings.data.ShutterCloseMinute = param.getStopMinute();

    _log.info("Shutter open time updated. From: %02d:%02d, to: %02d:%02d",
        _settings.data.ShutterOpenHour,
        _settings.data.ShutterOpenMinute,
        _settings.data.ShutterCloseHour,
        _settings.data.ShutterCloseMinute
    );
}

void Blynk::updateTimerPin()
{
    const auto& s = _settings.data;

    _blynkHandler.writeTimeRange(
        VirtualPins::Timer::OpenTime,
        s.ShutterOpenHour * 3600 + s.ShutterCloseMinute * 60,
        s.ShutterCloseHour * 3600 + s.ShutterCloseMinute * 60
    );
}

void Blynk::updatePins()
{
    _blynkHandler.writePin(
        VirtualPins::Sensors::RoomTemperature,
        Variant{ _lastRoomTemperature / 100.0 }
    );

    _blynkHandler.writePin(
        VirtualPins::Timer::Active,
        Variant{ _settings.data.ShutterTimerActive ? 1 : 0 }
    );
}

#endif