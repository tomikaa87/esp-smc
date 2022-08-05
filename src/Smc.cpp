#include "Smc.h"

Smc::Smc(const ApplicationConfig& appConfig)
    : _appConfig(appConfig)
    , _coreApplication(_appConfig)
    , _settings(_coreApplication.settings())
    , _automator(_coreApplication.systemClock(), _settings, _relayController)
#ifdef IOT_ENABLE_BLYNK
    , _blynk(_coreApplication.blynkHandler(), _settings, _relayController)
#endif
    , _mqtt(_coreApplication.mqttClient())
{
    Logger::setup(_appConfig, _coreApplication.systemClock());

#ifdef IOT_ENABLE_BLYNK
    _coreApplication.setBlynkUpdateHandler([this]{ onBlynkUpdateNeeded(); });
#endif

    setupMqtt();
    _coreApplication.setMqttUpdateHandler([this]{ updateMqtt(); });
}

void Smc::task()
{
    _coreApplication.task();
    _relayController.task();
    _temperatureSensor.task();
    _automator.task();
#ifdef IOT_ENABLE_BLYNK
    _blynk.task();
#endif
}

#ifdef IOT_ENABLE_BLYNK
void Smc::onBlynkUpdateNeeded()
{
    _blynk.updateRoomTemperature(_temperatureSensor.read());
}
#endif

void Smc::setupMqtt()
{
    for (unsigned i = 0; i < Config::DeviceCount; ++i) {
        _mqtt.shutters[i] = false;

        _mqtt.shutters[i].setChangedHandler([this, i](const int v) {
            const auto relayIndex =
                v > 0
                    ? i * 2         // 'open' relay
                    : i * 2 + 1;    // 'close' relay

            _relayController.pulse(relayIndex);
        });
    }

    _mqtt.openTimerTime.setChangedHandler([this](const int v) {
        _settings.data.ShutterOpenHour = v / 60;
        _settings.data.ShutterOpenMinute = v - _settings.data.ShutterOpenHour * 60;
        _settings.save();
    });

    _mqtt.closeTimerTime.setChangedHandler([this](const int v) {
        _settings.data.ShutterCloseHour = v / 60;
        _settings.data.ShutterCloseMinute = v - _settings.data.ShutterCloseHour * 60;
        _settings.save();
    });

    updateMqtt();
}

void Smc::updateMqtt()
{
    _mqtt.currentTemperature = _temperatureSensor.read() / 100.f;
    _mqtt.openTimerTime = _settings.data.ShutterOpenHour * 60 + _settings.data.ShutterOpenMinute;
    _mqtt.closeTimerTime = _settings.data.ShutterCloseHour * 60 + _settings.data.ShutterCloseMinute;
}
