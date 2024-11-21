#include "Mqtt.h"

void Mqtt::getStatusJsonString(JsonObject& output)
{
    auto&& mqtt = output.createNestedObject("mqtt");

    mqtt["status"] = "disabled";
}

