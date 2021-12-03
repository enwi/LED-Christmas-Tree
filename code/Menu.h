#ifndef MENU_H
#define MENU_H
#include <Arduino.h>

class Menu
{
public:
    bool isActive() const { return longPressMode != 0; }
    uint8_t getLongPressMode() const { return longPressMode; }
    void setLongPressMode(uint8_t m) { longPressMode = m; }

private:
    // Selected mode for long press (1 after 1s, 2 after 2s, 3 after 3s, 1 after 1s)
    uint8_t longPressMode = 0;
};

#endif