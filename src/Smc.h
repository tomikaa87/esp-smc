#pragma once

#include "Automator.h"
#include "Blynk.h"
#include "RelayController.h"
#include "Settings.h"
#include "TemperatureSensor.h"

#include <CoreApplication.h>
#include <Logger.h>

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
};