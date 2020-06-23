#ifndef HG_BOX_UTILITES_H
#define HG_BOX_UTILITES_H

#include "hg/GlobalConst.h"
#include "hg/TimeEngine/ArrivalDepartures/Box.h"

namespace hg {

bool boxCollidable(BoxType boxType) {
	return (boxType == BoxType::CRATE || boxType == BoxType::BOMB);
}

bool getBoxPickupPriority(BoxType currentBoxType, BoxType newBoxType) {
	if (currentBoxType == BoxType::NONE) {
		return true;
	}
	if (currentBoxType == BoxType::CRATE && newBoxType != BoxType::CRATE) {
		return true;
	}
	if (currentBoxType == BoxType::BOMB && newBoxType == BoxType::BALLOON) {
		return true;
	}
	return false;
}

bool getBoxPickupCollision(int gx, int gy, int gw, int gh, int bx, int by, int bw, int bh, BoxType boxType) {
	if (boxType == BoxType::CRATE || boxType == BoxType::BOMB) {
		// Crates can only be picked up if their base is above the feet of the Guy.
		// This prevents crates being passed up cliffs.
		return (gx <= bx + bw) && (gx + gw >= bx) && (gy < by + bh) && (gy + gh >= by + bh);
	}
	if (boxType == BoxType::BALLOON) {
		return (gx <= bx + bw) && (gx + gw >= bx) && (gy <= by + bh) && (gy + gh >= by);
	}
	return false;
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

int getBoxDropState(BoxType boxType) {
	if (boxType == BoxType::BOMB) {
		return 5 * hg::FRAMERATE;
	}
	return 0;
}

}//namespace hg
#endif //HG_BOX_UTILITES_H
