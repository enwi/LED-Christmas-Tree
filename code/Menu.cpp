#include "Menu.h"

#include <AceButton.h>
using namespace ace_button;

void Menu::handleButton(uint8_t eventType)
{
    switch (eventType)
    {
    case AceButton::kEventClicked:
        if (state == MenuState::brightnessSelect)
        {
            nextSubSelection();
            if (brightnessCallback)
            {
                brightnessCallback();
            }
        }
        else if (state == MenuState::colorSelect)
        {
            nextSubSelection();
        }
        break;
    case AceButton::kEventDoubleClicked:
        if (state == MenuState::brightnessSelect)
        {
            prevSubSelection();
            if (brightnessCallback)
            {
                brightnessCallback();
            }
        }
        else if (state == MenuState::colorSelect)
        {
            prevSubSelection();
        }
        break;
    case AceButton::kEventRepeatPressed:
        if (state == MenuState::mainSelect && ++longPressMode > 3)
        {
            longPressMode = 1;
        }
        else if (state == MenuState::brightnessSelect || state == MenuState::colorSelect)
        {
            // Leave sub menu (prevent more repeat events from triggering another selection)
            state = MenuState::closing;
            longPressMode = 0;
        }
        break;
    case AceButton::kEventReleased:
        if (state == MenuState::mainSelect && longPressMode != 0)
        {
            Callback* cb = actions[longPressMode - 1];
            if (cb != nullptr)
            {
                cb();
            }
            longPressMode = 0;
        }
        else if (state == MenuState::closing)
        {
            // Released, can return to normal function
            state = MenuState::mainSelect;
        }
        break;
    default:
        break;
    }
}

void Menu::setMainCallback(uint8_t selection, Callback* cb)
{
    if (selection < 1 || selection > 3)
    {
        return;
    }
    actions[selection - 1] = cb;
}

void Menu::nextSubSelection()
{
    if (++subSelection >= numSelections)
    {
        subSelection = 0;
    }
}

void Menu::prevSubSelection()
{
    if (subSelection-- == 0)
    {
        subSelection = numSelections - 1;
    }
}
