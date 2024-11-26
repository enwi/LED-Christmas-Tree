#include "Mqtt.h"

#include "FastLED.h"
#include "TreeEffects.h"

#ifndef TREE_SOFTWARE_VERSION
#define TREE_SOFTWARE_VERSION 2021.11.30
#endif

// Macro tricks to expand s to a string literal
#define XSTR(s) STR(s)
#define STR(s) #s

namespace
{
    /// Format string with the autoconfig message, which is published to the configTopic
    /// Describes the properties of the device to Home Assistant
    /// It needs to be formatted by replacing
    ///   1. #1 with unique id (MAC) at position
    ///   2. #2 with ip address
    ///   2. #3 with list of effect names in quotes
    ///
    /// Example config:
    /// {"dev":{"ids":["D4A67829"],"mf":"enwi","mdl":"LED Christmas Tree",
    ///         "name":"LED Christmas Tree","sw":"2021.11.30"},
    ///     "name":"LED Christmas Tree",
    ///     "uniq_id":"lightD4A67829",
    ///     "~":"esp8266-christmas-tree/D4A67829",
    ///     "avty_t":"~/lwt","cmd_t":"~/set","stat_t":"~/state","pl_avail":"Online",
    ///     "pl_not_avail":"Offline","schema":"json","brightness":true,"color_mode":true,
    ///     "supported_color_modes":["rgb"],"effect":true,"fx_list":["static"]}
    const char* autoConfigFormat PROGMEM
        = R"({"dev":{"ids":["#1"],"mf":"enwi","mdl":"LED Christmas Tree","name":"LED Christmas Tree","sw":")" XSTR(TREE_SOFTWARE_VERSION) R"(","cu":"http://#2"},"o":{"name":"LED Christmas Tree","sw":")" XSTR(
            TREE_SOFTWARE_VERSION) R"(","url":"https://github.com/enwi/LED-Christmas-Tree"},"avty_t":"esp8266-christmas-tree/#1/lwt","cmd_t":"esp8266-christmas-tree/#1/set","stat_t":"esp8266-christmas-tree/#1/state","pl_avail":"Online","pl_not_avail":"Offline","cmps":{ )"
                                   R"("light":{"p":"light","name":"Light","unique_id":"light#1","schema":"json","brightness":true,"supported_color_modes":["rgb"],"effect": true,"fx_list":[#2]},)"
                                   R"("speed":{"p":"number","name":"Effect Speed","unique_id":"speed#1","min": 0,"max":4,"value_template":"{{value_json.speed}}","command_template":"{\"speed\":{{value}}}"})"
                                   "}}";
    /// Base topic for all requests to the device
    /// The device id and child topics are inserted
    const char* baseTopic PROGMEM = R"(esp8266-christmas-tree/%s%s)";
    /// The auto discovery message is published to this topic
    /// The device id is inserted
    const char* configTopicFormat PROGMEM
        = R"(homeassistant/device/esp8266-christmas-tree/%s/config)"; // TODO: make prefix configurable

    /// Template for last will message
    const char* lastWillFormat PROGMEM = "Offline";
    /// Template for connected message
    const char* connectionMsgFormat PROGMEM = R"({"device":"%s","connected":true})";
    const char* onlineMsg = "Online";
} // namespace

constexpr int Mqtt::maxTopicNameLength;
constexpr uint16_t Mqtt::mqttMaxMessageSize;

Mqtt::Mqtt(const MqttConfig& config) : mqttConfig(config), mqtt(espClient) { }

void Mqtt::getStatusJsonString(JsonObject& output)
{
    auto&& mqttJson = output.createNestedObject("mqtt");

    if (!mqttConfig.enabled)
    {
        mqttJson["status"] = "disabled";
    }
    else if (status == Status::connected)
    {
        mqttJson["status"] = "connected";
    }
    else if (status == Status::disconnected)
    {
        mqttJson["status"] = "disconnected";
    }
    else if (status == Status::connectionFailed)
    {
        mqttJson["status"] = "connectionFailed";
    }
}

void Mqtt::setStatusCallback(StatusCallback c)
{
    statusCallback = c;
}

void Mqtt::receiveCallback(const char* topic, const uint8_t* payload, unsigned int length)
{
    DEBUGLN("Received message");
    // Free memory
    parseDocument.clear();
    deserializeJson(parseDocument, payload);
    Mqtt::LightCommand command = parseMessage(parseDocument.as<JsonObject>());
    if (commandListener)
    {
        commandListener(command);
    }
}

void Mqtt::publishAutoConfig()
{
    constexpr int size2 = 430;
    char topic[size2];

    String configString(FPSTR(autoConfigFormat));
    configString.replace("#1", deviceMAC);
    configString.replace("#2", WiFi.localIP().toString());
    configString.replace("#3", createEffectList());
    snprintf_P(topic, size2, configTopicFormat, deviceMAC);
    publish(topic, configString.c_str(), 0, true);
}

void Mqtt::onConnected()
{
    DEBUGF("Publishing %s on %s\n", onlineMsg, lastWillTopic);
    // Publish connected message
    publish(lastWillTopic, onlineMsg, 2, true);

    // Subscribe to state topic (qos 1: at least once, 2 not supported)
    DEBUGF("Subscribing to: %s\n", setTopic);
    mqtt.subscribe(setTopic, 1);

    publishAutoConfig();
    publishState();
}

Mqtt::LightCommand Mqtt::parseMessage(JsonObjectConst doc)
{
    LightCommand result;
    auto state = doc["state"];
    if (!state.isNull())
    {
        result.stateChanged = true;
        result.state = (state == "ON");
    }
    auto effect = doc["effect"];
    if (!effect.isNull())
    {
        result.effectChanged = true;
        result.effectIndex = getEffectIndex(effect | "");
    }
    auto brightness = doc["brightness"];
    if (!brightness.isNull())
    {
        result.brightnessChanged = true;
        result.brightness = brightness;
    }
    auto color = doc["color"];
    if (color)
    {
        result.colorChanged = true;
        result.colorR = color["r"];
        result.colorG = color["g"];
        result.colorB = color["b"];
    }
    auto speed = doc["speed"];
    if (!speed.isNull())
    {
        result.speedChanged = true;
        result.speed = speed;
    }
    return result;
}

const String& Mqtt::createEffectList()
{
    if (!effectList.isEmpty())
    {
        return effectList;
    }
    IEffect** begin = createEffects();
    IEffect** end = begin + static_cast<ptrdiff_t>(EffectType::maxValue);
    unsigned int reserveSize = 0;
    for (IEffect** it = begin; it != end; ++it)
    {
        if (it - begin == (int)EffectType::off)
        {
            continue;
        }
        reserveSize += std::strlen((*it)->getName()) + 2 + int(it + 1 != end);
    }
    effectList.reserve(reserveSize);
    for (IEffect** it = begin; it != end; ++it)
    {
        if (it - begin == (int)EffectType::off)
        {
            continue;
        }
        effectList.concat('\"');
        effectList.concat((*it)->getName());
        effectList.concat('\"');
        if (it + 1 != end)
        {
            effectList.concat(',');
        }
    }
    return effectList;
}

uint8_t Mqtt::getEffectIndex(const char* name)
{
    auto begin = createEffects();
    auto end = begin + static_cast<std::ptrdiff_t>(EffectType::maxValue);
    auto it = std::find_if(begin, end, [&](IEffect* effect) { return std::strcmp(effect->getName(), name) == 0; });
    return static_cast<uint8_t>(it - begin);
}

const char* Mqtt::getEffectName(uint8_t index)
{
    return createEffects()[index]->getName();
}

void Mqtt::publishState()
{
    if (statusCallback)
    {
        publishState(statusCallback());
    }
    else
    {
        publish("", 0, true);
        lastStatusUpdate = millis();
    }
}

void Mqtt::publishState(const LightCommand& status)
{
    String stateStr;
    parseDocument.clear();
    parseDocument["state"] = status.state ? "ON" : "OFF";
    auto color = parseDocument["color"].to<JsonObject>();
    color["r"] = status.colorR;
    color["g"] = status.colorG;
    color["b"] = status.colorB;
    parseDocument["color_mode"] = "rgb";
    parseDocument["brightness"] = status.brightness;
    if (status.effectIndex != static_cast<uint8_t>(EffectType::off)
        && status.effectIndex != static_cast<uint8_t>(EffectType::solid))
    {
        parseDocument["effect"] = getEffectName(status.effectIndex);
    }
    else
    {
        parseDocument["effect"] = "";
    }
    parseDocument["speed"] = status.speed;
    serializeJson(parseDocument, stateStr);
    DEBUGLN("Publishing state");
    publish(stateStr.c_str(), 0, true);
    lastStatus = status;
    lastStatusUpdate = millis();
}

void Mqtt::begin()
{
    // Initialize topics with id
    snprintf_P(stateTopic, maxTopicNameLength, baseTopic, deviceMAC, "/state");
    snprintf_P(lastWillTopic, maxTopicNameLength, baseTopic, deviceMAC, "/lwt");
    snprintf_P(setTopic, maxTopicNameLength, baseTopic, deviceMAC, "/set");
    if (!mqtt.setBufferSize(mqttMaxMessageSize))
    {
        DEBUGLN("Failed to increase mqtt buffer size");
        // Out of memory, cannot recover
        abort();
    }
    mqtt.setCallback(
        [this](char* topic, uint8_t* payload, unsigned int length) { receiveCallback(topic, payload, length); });
}

void Mqtt::connect()
{
    DEBUGF("Connecting mqtt to %s:%d with %s\n", mqttConfig.server.c_str(), mqttConfig.port, mqttConfig.id.c_str());
    mqtt.setServer(mqttConfig.server.c_str(), mqttConfig.port);
    // mqtt.setSocketTimeout(5);

    char msgBuffer[64];
    snprintf_P(msgBuffer, std::size(msgBuffer), lastWillFormat, deviceMAC);
    DEBUGF("%s %s\n", lastWillTopic, msgBuffer);
    const bool connected = mqtt.connect(
        mqttConfig.id.c_str(), mqttConfig.user.c_str(), mqttConfig.password.c_str(), lastWillTopic, 2, true, msgBuffer);

    if (connected)
    {
        // Publish connected message
        snprintf_P(msgBuffer, std::size(msgBuffer), connectionMsgFormat, deviceMAC);
        publish(msgBuffer, 2, true);

        updateStatus(Status::connected);
        DEBUGLN("mqtt connected");
    }
    else
    {
        updateStatus(Status::connectionFailed);
        DEBUG("mqtt connection failed ");
        DEBUGLN(mqtt.state());
    }
}
void Mqtt::disconnect()
{
    mqtt.disconnect();
    updateStatus(Status::disconnected);
    DEBUGLN("Mqtt disconnected");
}

void Mqtt::reconnect()
{
    EVERY_N_SECONDS(10)
    {
        connect();
    }
}

void Mqtt::update()
{
    if (!mqtt.loop())
    {
        if (status != Status::disconnected)
        {
            updateStatus(Status::disconnected);
        }
        // try to reconnect
        reconnect();
    }
    else
    {
        if (status == Status::disconnected)
        {
            updateStatus(Status::connected);
        }
        if (statusCallback && millis() - lastStatusUpdate > 1000)
        {
            // Check if status changed
            LightCommand newStatus = statusCallback();
            newStatus.compareTo(lastStatus);
            if (newStatus.stateChanged || newStatus.brightnessChanged || newStatus.effectChanged
                || newStatus.colorChanged)
            {
                publishState(newStatus);
            }
        }
    }
}

void Mqtt::updateStatus(Status s)
{
    if (status == s)
    {
        return;
    }
    status = s;
    if (status == Status::connected)
    {
        onConnected();
    }
    if (statusListener)
    {
        statusListener(status);
    }
}

void Mqtt::setStatusListener(StatusListener l)
{
    statusListener = l;
}

void Mqtt::setCommandListener(CommandListener l)
{
    commandListener = l;
}

void Mqtt::publish(const String& payload, uint8_t qos, bool retain)
{
    publish(stateTopic, payload.c_str(), qos, retain);
}

void Mqtt::publish(const char* payload, uint8_t qos, bool retain)
{
    publish(stateTopic, payload, qos, retain);
}

void Mqtt::publish(const char* topic, const char* payload, uint8_t qos, bool retain)
{
    if (mqtt.connected())
    {
        mqtt.publish(topic, reinterpret_cast<const uint8_t*>(payload), strlen(payload), retain);
    }
}

void Mqtt::LightCommand::compareTo(const LightCommand& old)
{
    stateChanged = (state != old.state);
    brightnessChanged = (brightness != old.brightness);
    effectChanged = (effectIndex != old.effectIndex);
    colorChanged = (colorR != old.colorR || colorG != old.colorG || colorB != old.colorB);
    speedChanged = (speed != old.speed);
}
