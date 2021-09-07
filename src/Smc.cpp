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
    for (unsigned i = 0; i < Config::DeviceCount; ++i) {
        _mqtt.shutterUps[i] = false;
        _mqtt.shutterDowns[i] = false;

        _mqtt.shutterUps[i].setChangedHandler([this, i](const bool v) {
            _relayController.pulse(i * 2);
            _mqtt.shutterUps[i] = false;
        });

        _mqtt.shutterDowns[i].setChangedHandler([this, i](const bool v) {
            _relayController.pulse(i * 2 + 1);
            _mqtt.shutterDowns[i] = false;
        });
    }

    _mqtt.upTimerTime.setChangedHandler([this](const int v) {
        _settings.data.ShutterOpenHour = v / 60;
        _settings.data.ShutterOpenMinute = v - _settings.data.ShutterOpenHour * 60;
        _settings.save();
    });

    _mqtt.downTimerTime.setChangedHandler([this](const int v) {
        _settings.data.ShutterCloseHour = v / 60;
        _settings.data.ShutterCloseMinute = v - _settings.data.ShutterCloseHour * 60;
        _settings.save();
    });

    updateMqtt();
}

void Smc::updateMqtt()
{
    _mqtt.currentTemperature = _temperatureSensor.read() / 100.f;
    _mqtt.upTimerTime = _settings.data.ShutterOpenHour * 60 + _settings.data.ShutterOpenMinute;
    _mqtt.downTimerTime = _settings.data.ShutterCloseHour * 60 + _settings.data.ShutterCloseMinute;
}
