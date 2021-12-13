#pragma once

#include <ArduinoJson.h>

#include "Constants.h"

class Mqtt
{
public:
    ///@brief Static class has no constructor
    Mqtt() = delete;

    static void getStatusJsonString(JsonObject& output);

private:
}; // namespace Mqtt
