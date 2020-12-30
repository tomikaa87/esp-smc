#include "Smc.h"

Smc::Smc(const ApplicationConfig& appConfig)
    : _appConfig(appConfig)
    , _coreApplication(_appConfig)
    , _settings(_coreApplication.settings())
    , _automator(_coreApplication.systemClock(), _settings, _relayController)
    , _blynk(_coreApplication.blynkHandler(), _settings, _relayController)
{
    Logger::setup(_appConfig, _coreApplication.systemClock());

    _coreApplication.setBlynkUpdateHandler([this]{ onBlynkUpdateNeeded(); });
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