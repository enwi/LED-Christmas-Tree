#pragma once

#include <functional>

#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "Config.h"
#include "Constants.h"

/// @brief Supports Home Assistant MQTT integration
///
/// Implements the necessary parts for lights integration into Home Assistant
/// @see https://www.home-assistant.io/integrations/light.mqtt/
class Mqtt
{
public:
    /// @brief Mqtt connection status
    enum class Status
    {
        connected,
        disconnected,
        connectionFailed
    };
    struct LightCommand
    {
        bool stateChanged = false;
        bool state = false;
        bool brightnessChanged = false;
        uint8_t brightness = 0;
        bool effectChanged = false;
        uint8_t effectIndex = 0;
        bool colorChanged = false;
        uint8_t colorR = 0;
        uint8_t colorG = 0;
        uint8_t colorB = 0;
        bool speedChanged = false;
        uint8_t speed = 0;
        bool colorSelectionChanged = false;
        uint8_t colorSelection = 0;
        /// @brief Set changed flags based on the old command
        void compareTo(const LightCommand& old);
    };

    using StatusListener = std::function<void(Status&)>;
    using CommandListener = std::function<void(const LightCommand&)>;
    using StatusCallback = std::function<LightCommand()>;

public:
    Mqtt(const MqttConfig& config);

    /// Cannot move or copy because of registered callbacks
    Mqtt(Mqtt&&) = delete;

    void begin();
    void connect();
    void disconnect();
    /// Attempt to reconnect in certain intervals
    void reconnect();
    Status getConnectionStatus() const { return status; }

    void update();

    void updateStatus(Status s);

    /// @brief Register listener for mqtt status updates
    ///
    /// Status updates are only received when the status has changed.
    void setStatusListener(StatusListener l);
    /// @brief Register listener for mqtt status updates
    ///
    /// Status updates are only received when the status has changed.
    void setCommandListener(CommandListener l);

    void getStatusJsonString(JsonObject& output);

    /// @brief Register callback to retrieve current status of light
    void setStatusCallback(StatusCallback c);

    bool isEnabled() const { return mqttConfig.enabled; }

    void publishState();
private:
    /// Publish to state topic
    void publish(const String& payload, uint8_t qos = 0, bool retain = false);
    /// Publish to state topic
    void publish(const char* payload, uint8_t qos = 0, bool retain = false);
    void publish(const char* topic, const char* payload, uint8_t qos = 0, bool retain = false);

    void receiveCallback(const char* topic, const uint8_t* payload, unsigned int length);

    void publishAutoConfig();
    void publishState(const LightCommand& c);
    void onConnected();

    /// @brief Parse mqtt message into LightCommand
    LightCommand parseMessage(JsonObjectConst doc);

    uint8_t getEffectIndex(const char* name);
    const char* getEffectName(uint8_t index);

private:
    static constexpr int maxTopicNameLength = 48;
    static constexpr uint16_t mqttMaxMessageSize = 2048;

    // Preallocate document for parsing
    DynamicJsonDocument parseDocument {1024};

    Status status = Status::disconnected;
    const MqttConfig& mqttConfig;
    WiFiClient espClient;
    PubSubClient mqtt;
    StatusListener statusListener;
    CommandListener commandListener;
    StatusCallback statusCallback;
    LightCommand lastStatus{};
    unsigned long lastStatusUpdate = 0;

    char stateTopic[maxTopicNameLength];
    char lastWillTopic[maxTopicNameLength];
    char setTopic[maxTopicNameLength];
}; // namespace Mqtt
