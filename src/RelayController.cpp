#include "RelayController.h"

#include <Arduino.h>

RelayController::RelayController()
    : _log{ "RelayController" }
{
    static_assert(sizeof(Config::Pins::Relay) > 0, "No relay pins defined");
    static_assert(sizeof(Config::Pins::Relay) < 256, "Maximum 255 relay pins supported");
    static_assert(sizeof(Config::Relays::Groups) == sizeof(Config::Pins::Relay) || sizeof(Config::Relays::Groups) == 0,
                  "Number of relay group definitions must match the number of relays or zero");

    uint8_t i = 0;

    for (const auto pin : Config::Pins::Relay)
    {
        _log.debug_P(PSTR("Setting up GPIO%d for relay %u"), pin, i);

        ::pinMode(pin, OUTPUT);
        setState(i, false);

        ++i;
    }
}

void RelayController::switchOn(const uint8_t relay) const
{
    _log.debug_P(PSTR("Switching on relay %u"), relay);

    setState(relay, true);
}

void RelayController::switchOff(const uint8_t relay) const
{
    _log.debug_P(PSTR("Switching off relay %u"), relay);

    setState(relay, false);
}

void RelayController::setState(const uint8_t relay, const bool on) const
{
    if (!isValidRelayIndex(relay))
        return;

    if (on && isAnyRelayInTheSameGroupTurnedOn(relay))
    {
        _log.warning_P(PSTR("Relay %u can't be turned on, another one in the same group is already active"), relay);
        return;
    }

    _log.debug_P(PSTR("Setting state of relay %u to %s"), relay, (on ? "On" : "Off"));

    ::digitalWrite(Config::Pins::Relay[relay], on ? 1 : 0);
}

void RelayController::toggle(const uint8_t relay) const
{
    _log.debug_P(PSTR("Toggling relay %u"), relay);

    setState(relay, isRelayTurnedOn(relay) ? false : true);
}

void RelayController::pulse(const uint8_t relay)
{
    _log.debug_P(PSTR("Pulsing relay %u"), relay);

    if (!isValidRelayIndex(relay))
        return;

    if (m_pulseStates[relay])
    {
        _log.warning_P(PSTR("Relay %u is already being pulsed"), relay);
        return;
    }

    m_pulseStartTimes[relay] = ::millis();
    m_pulseStates[relay] = true;

    switchOn(relay);
}

bool RelayController::isRelayTurnedOn(const uint8_t relay) const
{
    if (!isValidRelayIndex(relay))
        return false;

    return ::digitalRead(Config::Pins::Relay[relay]);
}

bool RelayController::isAnyRelayInTheSameGroupTurnedOn(const uint8_t relay) const
{
    // This will also handle if there are no groups defined
    if (relay >= sizeof(Config::Relays::Groups))
        return false;

    auto actualGroup = Config::Relays::Groups[relay];

    for (uint8_t i = 0; i < sizeof(Config::Pins::Relay); ++i)
    {
        if (Config::Relays::Groups[i] == actualGroup && isRelayTurnedOn(i))
            return true;
    }

    return false;
}

bool RelayController::isValidRelayIndex(const uint8_t relay) const
{
    if (relay >= sizeof(Config::Pins::Relay))
    {
        _log.warning_P(PSTR("Invalid relay index: %u"), relay);
        return false;
    }

    return true;
}

void RelayController::task()
{
    for (uint8_t i = 0; i < sizeof(Config::Pins::Relay); ++i)
    {
        if (m_pulseStates[i] && ::millis() - m_pulseStartTimes[i] >= Config::Relays::PulseWidth)
        {
            switchOff(i);
            m_pulseStates[i] = false;
        }
    }
}
