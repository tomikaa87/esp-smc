#include "Smc.h"
#include "PrivateConfig.h"

#include <Arduino.h>

#include <memory>

static std::unique_ptr<Smc> smc;

void setup()
{
    static ApplicationConfig appConfig;

    appConfig.firmwareVersion = VersionNumber{ 1, 4, 0 };

#ifdef IOT_ENABLE_BLYNK
    appConfig.blynk.appToken = Config::Blynk::AppToken;
    appConfig.blynk.serverHostName = Config::Blynk::ServerHostName;
    appConfig.blynk.serverPort = Config::Blynk::ServerPort;
#endif

    appConfig.logging.syslog.enabled = true;
    appConfig.logging.syslog.hostName = Config::Logging::SyslogHostName;
    appConfig.logging.syslog.serverHostName = Config::Logging::SyslogServerHost;
    appConfig.logging.syslog.serverPort = Config::Logging::SyslogServerPort;

    appConfig.mqtt.brokerIp = Config::Mqtt::BrokerIp;
    appConfig.mqtt.brokerPort = Config::Mqtt::BrokerPort;
    appConfig.mqtt.enabled = true;
    appConfig.mqtt.id = Config::Mqtt::Id;
    appConfig.mqtt.user = Config::Mqtt::User;
    appConfig.mqtt.password = Config::Mqtt::Password;

    appConfig.persistence.Size = 8;

#ifdef IOT_ENABLE_HTTP_OTA_UPDATE
    appConfig.otaUpdate.updateCheckIntervalMs = 60000;
    appConfig.otaUpdate.updateUrl = Config::OtaUpdate::FirmwareUpdateUrl;
#endif
    appConfig.otaUpdate.arduinoOtaPasswordHash = Config::OtaUpdate::ArduinoOtaPasswordHash;

    appConfig.wifi.password = Config::WiFi::Password;
    appConfig.wifi.ssid = Config::WiFi::SSID;

    appConfig.hostName = Config::HostName;

    smc.reset(new Smc(appConfig));
}

void loop()
{
    smc->task();
}