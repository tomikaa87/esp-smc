#include "Smc.h"

Smc::Smc(const ApplicationConfig& appConfig)
    : _appConfig(appConfig)
    , _coreApplication(_appConfig)
    , _settings(_coreApplication.settings())
    , _automator(_coreApplication.systemClock(), _settings, _relayController)
    , _blynk(_coreApplication.blynkHandler(), _settings, _relayController)
    , _mqtt(_coreApplication.mqttClient())
{
    Logger::setup(_appConfig, _coreApplication.systemClock());

    _coreApplication.setBlynkUpdateHandler([this]{ onBlynkUpdateNeeded(); });

    setupMqtt();
    _coreApplication.setMqttUpdateHandler([this]{ updateMqtt(); });
}

void Smc::task()
{
    _coreApplication.task();
    _relayController.task();
    _temperatureSensor.task();
    _automator.task();
    _blynk.task();
}

void Smc::onBlynkUpdateNeeded()
{
    _blynk.updateRoomTemperature(_temperatureSensor.read());
}

void Smc::setupMqtt()
{
    // TODO add MQTT setup code
}

void Smc::updateMqtt()
{
    _mqtt.currentTemperature = _temperatureSensor.read() / 100.f;
}
