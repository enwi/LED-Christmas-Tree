#pragma once

#include <ArduinoJson.h>
#include "Constants.h"
#include <FS.h>


class Config
{
public:
    ///@brief Static class has no constructor
    Config() = delete;

    static void initConfig();
    static void setDefaultConfig();
    static void save();

    static StaticJsonDocument<1024> *config;

private:
}; // namespace Networking
