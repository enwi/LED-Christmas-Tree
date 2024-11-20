#pragma once

#include <ArduinoJson.h>

#include "Constants.h"
#include "Config.h"

class Mqtt
{
public:
    Mqtt(Config& config) { }

    void getStatusJsonString(JsonObject& output);

private:
}; // namespace Mqtt
