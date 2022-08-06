#include "Smc.h"
#include "Utils.h"

#include <sstream>

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

    const auto makeButtonConfig = [](
        const bool open,
        PGM_P name,
        PGM_P buttonId,
        PGM_P commandTopic
    ) {
        std::stringstream config;

        config << '{';
        config << R"("icon":)" << (open ? R"("mdi:window-shutter-open")" : R"("mdi:window-shutter")");
        config << R"(,"name":")" << Utils::pgmToStdString(name) << (open ? " Open" : " Close") << '"';
        config << R"(,"object_id":"smc_)" << Utils::pgmToStdString(buttonId) << (open ? "_open" : "_close") << '"';
        config << R"(,"unique_id":"smc_)" << Utils::pgmToStdString(buttonId) << (open ? "_open" : "_close") << '"';
        config << R"(,"command_topic":")" << Utils::pgmToStdString(commandTopic) << '"';
        config << R"(,"payload_press":")" << (open ? "1" : "0") << '"';
        config << '}';

        return config.str();
    };

    static const std::array<PGM_P, 2> OpenButtonConfigTopics{
        PSTR("homeassistant/button/smc_bedroom_door_open/config"),
        PSTR("homeassistant/button/smc_bedroom_window_open/config")
    };

    static const std::array<PGM_P, 2> CloseButtonConfigTopics{
        PSTR("homeassistant/button/smc_bedroom_door_close/config"),
        PSTR("homeassistant/button/smc_bedroom_window_close/config")
    };

    static const std::array<std::tuple<PGM_P, PGM_P, PGM_P>, Config::DeviceCount> CommandButtons{
        std::tuple<PGM_P, PGM_P, PGM_P>{ PSTR("Bedroom Door"), PSTR("bedroom_door"), PSTR("home/shutters/bedroom/door/state/set") },
        std::tuple<PGM_P, PGM_P, PGM_P>{ PSTR("Bedroom Window"), PSTR("bedroom_window"), PSTR("home/shutters/bedroom/window/state/set") },
    };

    for (auto i = 0u; i < Config::DeviceCount; ++i) {
        _coreApplication.mqttClient().publish(
            OpenButtonConfigTopics[i],
            makeButtonConfig(
                true,
                std::get<0>(CommandButtons[i]),
                std::get<1>(CommandButtons[i]),
                std::get<2>(CommandButtons[i])
            ),
            false
        );

        _coreApplication.mqttClient().publish(
            CloseButtonConfigTopics[i],
            makeButtonConfig(
                false,
                std::get<0>(CommandButtons[i]),
                std::get<1>(CommandButtons[i]),
                std::get<2>(CommandButtons[i])
            ),
            false
        );
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
