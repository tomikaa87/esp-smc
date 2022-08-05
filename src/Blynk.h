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

#pragma once

#ifdef IOT_ENABLE_BLYNK

#include "Config.h"

#include <Logger.h>
#include <Variant.h>

#include <functional>
#include <stdint.h>

class IBlynkHandler;
class RelayController;
class Settings;
class TimeInputParam;

class Blynk
{
public:
    Blynk(
        IBlynkHandler& blynkHandler,
        Settings& settings,
        RelayController& relayController
    );

    void task();

    enum class EventType
    {
        RelayButtonPressed
    };

    void updateRoomTemperature(int16_t value);

private:
    IBlynkHandler& _blynkHandler;
    Settings& _settings;
    RelayController& _relayController;
    Logger _log{ "Blynk" };

    uint16_t _lastRoomTemperature = 0;

    void setupHandlers();

    void onConnected();
    void handleTimerInputChange(const TimeInputParam& param);
    void updateTimerPin();
    void updatePins();
};

#endif