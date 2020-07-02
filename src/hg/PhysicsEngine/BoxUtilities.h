#ifndef HG_BOX_UTILITES_H
#define HG_BOX_UTILITES_H

#include "hg/GlobalConst.h"
#include "hg/TimeEngine/ArrivalDepartures/Box.h"

namespace hg {

bool boxCollidable(BoxType boxType) {
	return (boxType == BoxType::CRATE || boxType == BoxType::LIGHT || boxType == BoxType::BOMB);
}

bool getBoxPickupPriority(BoxType currentBoxType, BoxType newBoxType) {
	if (currentBoxType == BoxType::NONE) {
		return true;
	}
	if ((currentBoxType == BoxType::CRATE || currentBoxType == BoxType::LIGHT) && newBoxType != BoxType::CRATE && newBoxType != BoxType::LIGHT) {
		return true;
	}
	if (currentBoxType == BoxType::BOMB && newBoxType == BoxType::BALLOON) {
		return true;
	}
	return false;
}

bool getBoxPickupCollision(int gx, int gy, int gw, int gh, int bx, int by, int bw, int bh, BoxType boxType) {
	if (boxType == BoxType::CRATE || boxType == BoxType::LIGHT || boxType == BoxType::BOMB) {
		// Crates can only be picked up if their base is above the feet of the Guy.
		// This prevents crates being passed up cliffs.
		return (gx <= bx + bw) && (gx + gw >= bx) && (gy < by + bh) && (gy + gh >= by + bh);
	}
	if (boxType == BoxType::BALLOON) {
		return (gx <= bx + bw) && (gx + gw >= bx) && (gy <= by + bh) && (gy + gh >= by);
	}
	return false;
}

bool getBoxLimitedJump(BoxType boxType) {
	return boxType == BoxType::CRATE || boxType == BoxType::BOMB;
}

int getBoxVertAirResistence(BoxType boxType) {
	if (boxType == BoxType::BALLOON) {
		return 2600 * hg::VERT_AIR_RESISTANCE;
	}
	return hg::VERT_AIR_RESISTANCE;
}

int getBoxCarryGravity(int baseGravity, BoxType boxType, int boxHeight) {
	if (boxType == BoxType::BALLOON) {
		return baseGravity * 2000 / boxHeight;
	}
	return baseGravity;
}

int getBoxPickupState(BoxType boxType, int boxState) {
	if (boxType == BoxType::BOMB) {
		return boxState;
	}
	return 0;
}

int getBoxDropState(BoxType boxType, int boxState) {
	if (boxType == BoxType::BOMB) {
		if (boxState == 0) {
			// Activate newly dropped bomb
			return hg::BOMB_TIMER;
		}
		else {
			return boxState;
		}
	}
	return 0;
}

}//namespace hg
#endif //HG_BOX_UTILITES_H
