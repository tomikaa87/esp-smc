#pragma once

#include "Automator.h"
#include "Blynk.h"
#include "RelayController.h"
#include "Settings.h"
#include "TemperatureSensor.h"

#include <CoreApplication.h>
#include <Logger.h>
#include <network/MQTT/MqttVariable.h>

#include <array>

class MqttClient;

class Smc
{
public:
    explicit Smc(const ApplicationConfig& appConfig);

    void task();

private:
    const ApplicationConfig& _appConfig;
    CoreApplication _coreApplication;
    const Logger _log{ "Smc" };
    Settings _settings;
    RelayController _relayController;
    TemperatureSensor _temperatureSensor;
    Automator _automator;

#ifdef IOT_ENABLE_BLYNK
    Blynk _blynk;
    void onBlynkUpdateNeeded();
#endif

    struct Mqtt {
        Mqtt(MqttClient& mqttClient)
            : currentTemperature{ PSTR("home/temperature/bedroom"), mqttClient }
            , shutters{
                MqttVariable<int>{ PSTR("home/shutters/bedroom/door/state/get"), PSTR("home/shutters/bedroom/door/state/set"), mqttClient },
                MqttVariable<int>{ PSTR("home/shutters/bedroom/window/state/get"), PSTR("home/shutters/bedroom/window/state/set"), mqttClient }
            }
            , openTimerTime{ PSTR("home/shutters/bedroom/timer/open/get"), PSTR("home/shutters/bedroom/timer/open/set"), mqttClient }
            , closeTimerTime{ PSTR("home/shutters/bedroom/timer/close/get"), PSTR("home/shutters/bedroom/timer/close/set"), mqttClient }
        {}

        MqttVariable<float> currentTemperature;
        std::array<MqttVariable<int>, Config::DeviceCount> shutters;
        MqttVariable<int> openTimerTime;
        MqttVariable<int> closeTimerTime;
    } _mqtt;

    void setupMqtt();
    void updateMqtt();
};