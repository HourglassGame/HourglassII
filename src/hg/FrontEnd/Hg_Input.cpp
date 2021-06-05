#include "Hg_Input.h"
#include "hg/TimeEngine/ArrivalDepartures/InputList.h"
#include "hg/Util/Maths.h"
namespace hg
{
static void updatePress(int &var, bool inputState)
{
	var = inputState ?
		var == 0 ? 1 : -1
		: 0;
}

static void updateAbility(int &var, Ability &abilityCursor, Ability oldAbilityCursor, Ability abilityType, bool inputState, bool cancelAction)
{
	updatePress(var, inputState);
	if (cancelAction) {
		abilityCursor = Ability::NO_ABILITY;
	}
	else {
		if (var == 1) {
			if (oldAbilityCursor == abilityType) {
				abilityCursor = Ability::NO_ABILITY;
			}
			else {
				abilityCursor = abilityType;
			}
		}
	}
}

static void updateAbilityClick(bool clicked, Ability &abilityCursor, Ability oldAbilityCursor, Ability abilityType)
{
	if (!clicked) {
		return;
	}
	if (oldAbilityCursor == abilityType) {
		abilityCursor = Ability::NO_ABILITY;
	}
	else {
		abilityCursor = abilityType;
	}
}

Input::Input() :
	left(),
	right(),
	up(),
	space(),
	down(),
	boxLeft(),
	boxRight(),
	shift(),
	shift_r(),
	enter(),
	use(),
	ability_1(),
	ability_2(),
	ability_3(),
	ability_4(),
	abilityUse(),
	abilityCursor(),
	abilityChanged(),
	mouseLeftWorld(),
	mouseLeft(),
	mouseRight(),
	mouseTimelinePosition(-1),
	mousePersonalTimelinePosition(),
	mouseX(),
	mouseY(),
	timelineLength()
{
}

void Input::updateState(
	GLFWWindow &windowglfw,
	ActivePanel const mousePanel,
	ActiveButton const hoveredButton,
	bool waitingForWave,
	bool cancelAction,
	int mouseXTimelineOffset, int mouseXOfEndOfTimeline,
	int mouseXOfEndOfPersonalTimeline, std::size_t personalTimelineLength,
	int mouseOffX, int mouseOffY, double mouseScale,
	bool frameRunSinceLastUpdate)
{
	left = (glfwGetKey(windowglfw.w, GLFW_KEY_A) == GLFW_PRESS);
	right = (glfwGetKey(windowglfw.w, GLFW_KEY_D) == GLFW_PRESS);
	up = (glfwGetKey(windowglfw.w, GLFW_KEY_W) == GLFW_PRESS);
	space = (glfwGetKey(windowglfw.w, GLFW_KEY_SPACE) == GLFW_PRESS);
	updatePress(down, glfwGetKey(windowglfw.w, GLFW_KEY_S) == GLFW_PRESS);
	updatePress(boxLeft, glfwGetKey(windowglfw.w, GLFW_KEY_Q) == GLFW_PRESS);
	updatePress(boxRight, glfwGetKey(windowglfw.w, GLFW_KEY_E) == GLFW_PRESS);
	updatePress(shift, glfwGetKey(windowglfw.w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
	updatePress(shift_r, glfwGetKey(windowglfw.w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
	updatePress(enter, glfwGetKey(windowglfw.w, GLFW_KEY_ENTER) == GLFW_PRESS);
	updatePress(use, glfwGetKey(windowglfw.w, GLFW_KEY_E) == GLFW_PRESS);

	bool mouseLeftPressed = (glfwGetMouseButton(windowglfw.w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

	updatePress(mouseLeftWorld, mouseLeftPressed && mousePanel == ActivePanel::WORLD);
	updatePress(mouseLeft, mouseLeftPressed);
	updatePress(mouseRight, glfwGetMouseButton(windowglfw.w, GLFW_MOUSE_BUTTON_RIGHT));

	double mX, mY;
	glfwGetCursorPos(windowglfw.w, &mX, &mY);

	Ability oldAbility = abilityCursor;

	updateAbility(ability_1, abilityCursor, oldAbility, Ability::TIME_JUMP, glfwGetKey(windowglfw.w, GLFW_KEY_1) == GLFW_PRESS, cancelAction);
	updateAbility(ability_2, abilityCursor, oldAbility, Ability::TIME_REVERSE, glfwGetKey(windowglfw.w, GLFW_KEY_2) == GLFW_PRESS, cancelAction);
	updateAbility(ability_3, abilityCursor, oldAbility, Ability::TIME_GUN, glfwGetKey(windowglfw.w, GLFW_KEY_3) == GLFW_PRESS, cancelAction);
	updateAbility(ability_4, abilityCursor, oldAbility, Ability::TIME_PAUSE, glfwGetKey(windowglfw.w, GLFW_KEY_4) == GLFW_PRESS, cancelAction);

	updateAbilityClick((mouseLeft == 1 && hoveredButton == ActiveButton::TIME_JUMP), abilityCursor, oldAbility, Ability::TIME_JUMP);
	updateAbilityClick((mouseLeft == 1 && hoveredButton == ActiveButton::TIME_REVERSE), abilityCursor, oldAbility, Ability::TIME_REVERSE);
	updateAbilityClick((mouseLeft == 1 && hoveredButton == ActiveButton::TIME_GUN), abilityCursor, oldAbility, Ability::TIME_GUN);
	updateAbilityClick((mouseLeft == 1 && hoveredButton == ActiveButton::TIME_PAUSE), abilityCursor, oldAbility, Ability::TIME_PAUSE);

	if (frameRunSinceLastUpdate && abilityCursor == oldAbility) {
		abilityCursor = Ability::NO_ABILITY;
	}

	pausePressed = (mouseLeft == 1 && hoveredButton == ActiveButton::PAUSE);

	if (mouseLeftPressed && mousePanel == ActivePanel::PERSONAL_TIME) {
		int mousePosition = std::max(0, std::min(mouseXOfEndOfPersonalTimeline, static_cast<int>(std::round(mX)) - mouseXTimelineOffset));
		mousePersonalTimelinePosition = static_cast<int>(personalTimelineLength - (static_cast<int>(mousePosition*personalTimelineLength / static_cast<double>(mouseXOfEndOfPersonalTimeline))));
	}
	else if (waitingForWave) {
		mousePersonalTimelinePosition += 1;
	}

	abilityUse = false;
	if (abilityCursor == Ability::TIME_REVERSE || abilityCursor == Ability::TIME_PAUSE) {
		abilityUse = true;
		mouseTimelinePosition = -1;
	}
	if (abilityCursor == Ability::NO_ABILITY) {
		mouseTimelinePosition = -1;
	}

	if (abilityCursor == Ability::TIME_JUMP) {
		if (mouseRight == 1) {
			abilityCursor = Ability::NO_ABILITY;
		}
		else {
			if (mouseLeftPressed && mousePanel == ActivePanel::GLOBAL_TIME) {
				int mousePosition = std::max(0, std::min(mouseXOfEndOfTimeline - 1, static_cast<int>(std::round(mX)) - mouseXTimelineOffset));
				mouseTimelinePosition = static_cast<int>(mousePosition*timelineLength / static_cast<double>(mouseXOfEndOfTimeline));
				abilityUse = true;
			}
		}
	}

	if (abilityCursor == Ability::TIME_GUN) {
		if (mouseRight == 1) {
			abilityCursor = Ability::NO_ABILITY;
		}
		else {
			if (mouseLeftPressed && mousePanel == ActivePanel::GLOBAL_TIME) {
				int mousePosition = std::max(0, std::min(mouseXOfEndOfTimeline - 1, static_cast<int>(std::round(mX)) - mouseXTimelineOffset));
				mouseTimelinePosition = static_cast<int>(mousePosition*timelineLength / static_cast<double>(mouseXOfEndOfTimeline));
			}
			if (mouseLeftWorld == 1 && mouseTimelinePosition != -1) {
				abilityUse = true;
			}
		}
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
			up || space,
			down == 1,
			boxLeft == 1,
			boxRight == 1,
			(shift == 1) || (shift_r == 1) || (enter == 1) || (use == 1), //portalUsed
			abilityUse, //abilityUsed
			abilityCursor,
			getTimeCursor(),
			mouseX,
			mouseY),
		mousePersonalTimelinePosition);
}

FrameID Input::getTimeCursor() const
{
	return mouseTimelinePosition == -1 ? FrameID() : FrameID(mouseTimelinePosition, UniverseID(timelineLength));
}

Ability Input::getAbilityCursor() const
{
	return abilityCursor;
}

bool Input::getPausePressed() const
{
	return pausePressed;
}

bool Input::getInCancelAbsorbingState() const
{
	return (abilityCursor != Ability::NO_ABILITY);
}


}

