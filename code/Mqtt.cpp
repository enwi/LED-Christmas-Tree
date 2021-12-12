#if defined(ESP8266) || defined(ESP32)
#include "Mqtt.h"

void Mqtt::getStatusJsonString(JsonObject &output)
{
    auto && mqtt = output.createNestedObject("mqtt");

    mqtt["status"] = "disabled";
}


#endif
