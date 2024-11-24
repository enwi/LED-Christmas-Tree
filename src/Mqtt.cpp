#include "Mqtt.h"

namespace
{
    /// Format string with the autoconfig message, which is published to the configTopic
    /// Describes the properties of the device to Home Assistant
    /// It needs to be formatted using
    ///   1. unique id (MAC) at position 1, 2 and 3
    ///   2. list of effect names in quotes as the 4th format argument
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
        = R"({"dev":{"ids":["%s"],"mf":"enwi","mdl":"LED Christmas Tree","name":"LED Christmas Tree","sw":"2021.11.30"},"uniq_id":"light%s","~":"esp8266-christmas-tree/%s","avty_t":"~/lwt","cmd_t":"~/set","stat_t":"~/state","pl_avail":"Online","pl_not_avail":"Offline","schema":"json","brightness":true,"color_mode":true,"supported_color_modes":["rgb"],"effect": true,"fx_list":[%s]})";
    /// Base topic for all requests to the device
    /// The device id and child topics are inserted
    const char* baseTopic PROGMEM = R"(esp8266-christmas-tree/%s%s)";
    /// The auto discovery message is published to this topic
    /// The device id is inserted
    const char* configTopicFormat PROGMEM
        = R"(homeassistant/light/esp8266-christmas-tree/%s/config)"; // TODO: make prefix configurable

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
    auto&& mqtt = output.createNestedObject("mqtt");

    mqtt["status"] = "disabled";
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
    constexpr int size = 430;
    char buffer[size];
    constexpr int size2 = 430;
    char topic[size2];

    snprintf_P(buffer, size, autoConfigFormat, deviceMAC, deviceMAC, deviceMAC, R"("static","twinkle")");
    snprintf_P(topic, size2, configTopicFormat, deviceMAC);
    publish(topic, buffer, 0, true);
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
    if (state)
    {
        result.stateChanged = true;
        result.state = (state == "ON");
    }
    auto effect = doc["effect"];
    if (effect)
    {
        result.effectChanged = true;
        result.effectIndex = getEffectIndex(effect | "");
    }
    auto brightness = doc["brightness"];
    if (brightness)
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
    return result;
}

String Mqtt::createEffectList() const
{
    return "";
}

uint8_t Mqtt::getEffectIndex(const char* name)
{
    return uint8_t();
}

void Mqtt::publishState()
{
    String stateStr;
    // serializeJson()
    stateStr = "";
    publish(stateStr.c_str(), 0, true);
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
