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
    Blynk _blynk;

    void onBlynkUpdateNeeded();

    struct Mqtt {
        Mqtt(MqttClient& mqttClient)
            : currentTemperature{ PSTR("smc/temp/current"), mqttClient }
            , shutterUps{
                MqttVariable<bool>{ PSTR("smc/shutter/1/up"), PSTR("smc/shutter/1/up/set"), mqttClient },
                MqttVariable<bool>{ PSTR("smc/shutter/2/up"), PSTR("smc/shutter/2/up/set"), mqttClient }
            }
            , shutterDowns{
                MqttVariable<bool>{ PSTR("smc/shutter/1/down"), PSTR("smc/shutter/1/down/set"), mqttClient },
                MqttVariable<bool>{ PSTR("smc/shutter/2/down"), PSTR("smc/shutter/2/down/set"), mqttClient }
            }
            , upTimerTime{ PSTR("smc/timer/up"), PSTR("smc/timer/up/set"), mqttClient }
            , downTimerTime{ PSTR("smc/timer/down"), PSTR("smc/timer/down/set"), mqttClient }
        {}

        MqttVariable<float> currentTemperature;
        std::array<MqttVariable<bool>, 2> shutterUps;
        std::array<MqttVariable<bool>, 2> shutterDowns;
        MqttVariable<int> upTimerTime;
        MqttVariable<int> downTimerTime;
    } _mqtt;

    void setupMqtt();
    void updateMqtt();
};