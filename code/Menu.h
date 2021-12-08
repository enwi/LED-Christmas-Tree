#ifndef MENU_H
#define MENU_H
#include <Arduino.h>

class Menu
{
public:
    enum class MenuState
    {
        mainSelect,
        brightnessSelect,
        colorSelect,
        closing
    };
    // Returns true when event is consumed
    bool handleButton(uint8_t eventType);
    bool isActive() const
    {
        return (state == MenuState::mainSelect && longPressMode != 0)
            || (state != MenuState::mainSelect && state != MenuState::closing);
    }
    uint8_t getLongPressMode() const { return longPressMode; }
    MenuState getMenuState() const { return state; }
    void setMenuState(MenuState s)
    {
        state = s;
        subSelection = 0;
    }
    void setLongPressMode(uint8_t m) { longPressMode = m; }

    using Callback = void();
    void setMainCallback(uint8_t selection, Callback* cb);
    void setBrightnessCallback(Callback* cb) { brightnessCallback = cb; }

    void setSubSelection(uint8_t selection) { subSelection = selection; }
    uint8_t getSubSelection() const { return subSelection; }
    void nextSubSelection();
    void prevSubSelection();
    void setNumSubSelections(uint8_t num) { numSelections = num; }

private:
    Callback* actions[3] = {};
    // Selected mode for long press (1 after 1s, 2 after 2s, 3 after 3s, 1 after 1s)
    uint8_t longPressMode = 0;
    MenuState state = MenuState::mainSelect;
    Callback* brightnessCallback = nullptr;
    uint8_t subSelection = 0;
    uint8_t numSelections = 1;
};

#endif