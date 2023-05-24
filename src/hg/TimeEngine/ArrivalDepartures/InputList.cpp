#include "InputList.h"
namespace hg {
GuyInput::GuyInput() noexcept :
	moveLeft(),
	moveRight(),
	jump(),
	boxAction(),
	boxLeft(),
	boxRight(),
	portalUsed(),
	abilityUsed(),
	abilityCursor(Ability::NO_ABILITY),
	timeCursor(),
	xCursor(),
	yCursor()
{
}
GuyInput::GuyInput(
	bool moveLeft,
	bool moveRight,
	bool jump,
	bool boxAction,
	bool boxLeft,
	bool boxRight,
	bool portalUsed,
	bool abilityUsed,
	Ability abilityCursor,
	FrameID timeCursor,
	int xCursor,
	int yCursor) noexcept :
		moveLeft(moveLeft),
		moveRight(moveRight),
		jump(jump),
		boxAction(boxAction),
		boxLeft(boxLeft),
		boxRight(boxRight),
		portalUsed(portalUsed),
		abilityUsed(abilityUsed),
		abilityCursor(abilityCursor),
		timeCursor(timeCursor),
		xCursor(xCursor),
		yCursor(yCursor)
{
}
} //namespace hg
