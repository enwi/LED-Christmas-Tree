#include "Menu.h"

#include <AceButton.h>
#include <Constants.h>

using namespace ace_button;

bool Menu::handleButton(uint8_t eventType)
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
            return true;
        }
        else if (state == MenuState::colorSelect)
        {
            nextSubSelection();
            return true;
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
            return true;
        }
        else if (state == MenuState::colorSelect)
        {
            prevSubSelection();
            return true;
        }
        break;
    case AceButton::kEventRepeatPressed:
        if (state == MenuState::mainSelect && ++longPressMode > numLongPressModes)
        {
            // Leave menu (prevent more repeat events from triggering another selection)
            state = MenuState::closing;
            longPressMode = 0;
            return true;
        }
        else if (state == MenuState::brightnessSelect || state == MenuState::colorSelect)
        {
            // Leave sub menu (prevent more repeat events from triggering another selection)
            state = MenuState::closing;
            longPressMode = 0;
            return true;
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
            return true;
        }
        else if (state == MenuState::brightnessSelect)
        {
            nextSubSelection();
            if (brightnessCallback)
            {
                brightnessCallback();
            }
            return true;
        }
        else if (state == MenuState::colorSelect)
        {
            nextSubSelection();
            return true;
        }
        else if (state == MenuState::closing)
        {
            // Released, can return to normal function
            state = MenuState::mainSelect;
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

void Menu::setMainCallback(uint8_t selection, Callback* cb)
{
    if (selection < 1 || selection > numLongPressModes)
    {
        return;
    }
    actions[selection - 1] = cb;
}

void Menu::nextSubSelection()
{
    DEBUGLN("Next subselection");
    if (++subSelection >= numSelections)
    {
        subSelection = 0;
    }
}

void Menu::prevSubSelection()
{
    DEBUGLN("Prev subselection");
    if (subSelection-- == 0)
    {
        subSelection = numSelections - 1;
    }
}
