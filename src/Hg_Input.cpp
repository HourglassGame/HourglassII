#include "Hg_Input.h"
#include "InputList.h"
#include "Maths.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
namespace hg
{
static void updatePress(int &var, bool inputState)
{
    var = inputState ?
            var == 0 ? 1 : -1
          : 0;
}

Input::Input() :
    left(),
    right(),
    up(),
    down(),
    boxLeft(),
    boxRight(),
    space(),
    mouseLeft(),
    abilityCursor(),
    abilityChanged(),
    mouseTimelinePosition(),
    mousePersonalTimelinePosition(),
    mouseX(),
    mouseY(),
    timelineLength()
{
}

void Input::updateState(
    hg::RenderWindow::InputState const &input,
    GLFWWindow &windowglfw,
    ActivePanel const mousePanel,
    bool waitingForWave,
    int mouseXTimelineOffset, int mouseXOfEndOfTimeline,
    int mouseXOfEndOfPersonalTimeline, std::size_t personalTimelineLength,
    int mouseOffX, int mouseOffY, double mouseScale)
{
    left = (glfwGetKey(windowglfw.w, GLFW_KEY_A) == GLFW_PRESS);
    right = (glfwGetKey(windowglfw.w, GLFW_KEY_D) == GLFW_PRESS);
    up = (glfwGetKey(windowglfw.w, GLFW_KEY_W) == GLFW_PRESS);
    updatePress(down, glfwGetKey(windowglfw.w, GLFW_KEY_S) == GLFW_PRESS);
    updatePress(boxLeft, glfwGetKey(windowglfw.w, GLFW_KEY_Q) == GLFW_PRESS);
    updatePress(boxRight, glfwGetKey(windowglfw.w, GLFW_KEY_E) == GLFW_PRESS);
    updatePress(space, glfwGetKey(windowglfw.w, GLFW_KEY_SPACE) == GLFW_PRESS);

    bool mouseLeftPressed = (glfwGetMouseButton(windowglfw.w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

    double mX, mY;
    glfwGetCursorPos(windowglfw.w, &mX, &mY);

    updatePress(mouseLeft, mouseLeftPressed && mousePanel == ActivePanel::WORLD);

    Ability oldAbility = abilityCursor;
    if (glfwGetKey(windowglfw.w, GLFW_KEY_1) == GLFW_PRESS) {
        abilityCursor = Ability::TIME_JUMP;
    }
    if (glfwGetKey(windowglfw.w, GLFW_KEY_2) == GLFW_PRESS) {
        abilityCursor = Ability::TIME_REVERSE;
    }
    if (glfwGetKey(windowglfw.w, GLFW_KEY_3) == GLFW_PRESS) {
        abilityCursor = Ability::TIME_GUN;
    }
    if (glfwGetKey(windowglfw.w, GLFW_KEY_4) == GLFW_PRESS) {
        abilityCursor = Ability::TIME_PAUSE;
    }
    abilityChanged = (abilityCursor != oldAbility);

    if (mouseLeftPressed && mousePanel == ActivePanel::PERSONAL_TIME) {
        int mousePosition = std::max(0, std::min(mouseXOfEndOfPersonalTimeline, static_cast<int>(std::round(mX)) - mouseXTimelineOffset));
        mousePersonalTimelinePosition = static_cast<int>(personalTimelineLength - (static_cast<int>(mousePosition*personalTimelineLength / static_cast<double>(mouseXOfEndOfPersonalTimeline))));
    }
    else if (waitingForWave) {
        mousePersonalTimelinePosition += 1;
    }

    if ((mouseLeftPressed && mousePanel == ActivePanel::GLOBAL_TIME) ||
        (glfwGetMouseButton(windowglfw.w, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) ||
        (glfwGetMouseButton(windowglfw.w, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) ||
        (glfwGetMouseButton(windowglfw.w, GLFW_MOUSE_BUTTON_5) == GLFW_PRESS) ||
        (glfwGetMouseButton(windowglfw.w, GLFW_MOUSE_BUTTON_6) == GLFW_PRESS))
    {
        int mousePosition = std::max(0, std::min(mouseXOfEndOfTimeline - 1, static_cast<int>(std::round(mX)) - mouseXTimelineOffset));
        mouseTimelinePosition = static_cast<int>(mousePosition*timelineLength / static_cast<double>(mouseXOfEndOfTimeline));
    }
    mouseX = static_cast<int>(std::round((mX - mouseOffX)*mouseScale));
    mouseY = static_cast<int>(std::round((mY - mouseOffY)*mouseScale));
}

InputList Input::AsInputList() const
{
    return InputList(
        GuyInput(
            left,
            right,
            up,
            down == 1,
            boxLeft == 1,
            boxRight == 1,
            space == 1, //portalUsed
            mouseLeft == 1, //abilityUsed
            abilityCursor,
            FrameID(mouseTimelinePosition, UniverseID(timelineLength)),
            mouseX,
            mouseY),
        mousePersonalTimelinePosition);
}

FrameID Input::getTimeCursor() const
{
    return FrameID(mouseTimelinePosition, UniverseID(timelineLength));
}

}

