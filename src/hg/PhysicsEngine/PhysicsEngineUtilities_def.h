#include "hg/GlobalConst.h"
#include <tuple>
namespace hg {
template<typename RandomAccessObjectAndTypeRange>
void buildDeparturesForComplexEntities(
	RandomAccessObjectAndTypeRange const &next,
	PhysicsEngine::FrameDepartureT &newDepartures)
{
	for (typename boost::range_reference<RandomAccessObjectAndTypeRange const>::type thingAndTime : next)
	{
		newDepartures[thingAndTime.frame].add(thingAndTime.object);
	}
}

template<typename RandomAccessObjectAndTypeRange>
void buildDeparturesForComplexEntitiesWithIndexCaching(
	RandomAccessObjectAndTypeRange const &next,
	mt::std::vector<std::tuple<std::size_t, Frame *>> &departureFrames,
	PhysicsEngine::FrameDepartureT &newDepartures)
{
	for (typename boost::range_reference<RandomAccessObjectAndTypeRange const>::type thingAndTime : next)
	{
		departureFrames.emplace_back(thingAndTime.object.getIndex(), thingAndTime.frame);
		newDepartures[thingAndTime.frame].add(thingAndTime.object);
	}
}

template<typename RandomAccessGuyRange>
bool currentPlayerInArrivals(RandomAccessGuyRange const &guyArrivals, std::size_t playerInputSize)
{
	//The arrivals are sorted
	//The current player arrival can be either the last arrival, or the second last arrival.
	//If it is the second last arrival, then the last arrival must be the next player arrival.
	if (boost::size(guyArrivals) == 0) {
		return false;
	}
	else {
		std::size_t lastGuyIndex((boost::end(guyArrivals) - 1)->getIndex());
		if (lastGuyIndex < playerInputSize - 1) return false;
		if (lastGuyIndex == playerInputSize - 1) return true;
		if (boost::size(guyArrivals) == 1) return false;
		return (boost::end(guyArrivals) - 2)->getIndex() == playerInputSize - 1;
	}
}

template <
	typename RandomAccessPortalRange,
	typename RandomAccessMutatorRange,
	typename FrameT>
	void makeBoxAndTimeWithPortalsAndMutators(
		mp::std::vector<ObjectAndTime<Box, FrameT>> &nextBox,
		mp::std::vector<char> &nextBoxNormalDeparture,
		RandomAccessPortalRange const &portals,
		RandomAccessMutatorRange const &mutators,
		int x,
		int y,
		int xspeed,
		int yspeed,
		int width,
		int height,
		BoxType boxType,
		int state,
		int oldIllegalPortal,
		TimeDirection const oldTimeDirection,
		TriggerFrameState &triggerFrameState,
		FrameT frame,
		memory_pool<user_allocator_tbb_alloc> &pool)
{
	TimeDirection timeDirection(oldTimeDirection);
	int arrivalBasis = -1;
	int illegalPortal = -1;
	FrameT nextTime(nextFrame(frame, timeDirection));


	// Mutator check
	mp::std::vector<int> mutatorCollisions(pool);

	for (unsigned i = 0; i < mutators.size(); ++i)
	{
		if (IntersectingRectanglesInclusiveCollisionOverlap(x, y, width, height,
			mutators[i].getX(), mutators[i].getY(),
			mutators[i].getWidth(), mutators[i].getHeight(),
			mutators[i].getCollisionOverlap()))
		{
			mutatorCollisions.push_back(i);
		}
	}

	// send vector of collisions that occurred (if any)
	if (mutatorCollisions.size() != 0) {
		boost::optional<Box> newBox = triggerFrameState.mutateObject(
			mutatorCollisions,
			Box(
				x,
				y,
				xspeed,
				yspeed,
				width, height,
				boxType,
				state,
				oldIllegalPortal,
				-1,
				timeDirection));
		if (!newBox) {
			return; // box was destroyed, do not add
		}
		x = newBox->getX();
		y = newBox->getY();
		xspeed = newBox->getXspeed();
		yspeed = newBox->getYspeed();
		width = newBox->getWidth();
		height = newBox->getHeight();
		boxType = newBox->getBoxType();
		illegalPortal = newBox->getIllegalPortal();
		//arrivalLocation = newBox->getArrivalLocation();
		timeDirection = newBox->getTimeDirection();
	}

	bool normalDeparture = timeDirection == oldTimeDirection;

	// fall through portals
	for (unsigned i = 0; i < portals.size(); ++i)
	{
		if (portals[i].getFallable() &&
			(!portals[i].getIsLaser()) &&
			IntersectingRectanglesInclusiveCollisionOverlap(x, y, width, height,
			portals[i].getX(), portals[i].getY(),
			portals[i].getWidth(), portals[i].getHeight(),
			portals[i].getCollisionOverlap()))
		{
			if (oldIllegalPortal != -1 && portals[i].getIndex() == oldIllegalPortal)
			{
				illegalPortal = i;
			}
			else if (triggerFrameState.shouldPort(i, Box(x, y, xspeed, yspeed, width, height, boxType, 0, oldIllegalPortal, -1, timeDirection), false))
			{
				FrameT portalTime(
					portals[i].getRelativeTime() ?
					getArbitraryFrame(getUniverse(frame), getFrameNumber(frame) + portals[i].getTimeDestination()) :
					getArbitraryFrame(getUniverse(frame), portals[i].getTimeDestination()));

				if (portals[i].getRelativeDirection())
				{
					timeDirection *= portals[i].getDestinationDirection();
				}
				else
				{
					timeDirection = portals[i].getDestinationDirection();
				}

				nextTime = !isNullFrame(portalTime) ? nextFrame(portalTime, timeDirection) : FrameT();
				illegalPortal = portals[i].getIllegalDestination();
				arrivalBasis = portals[i].getDestinationIndex();
				if (arrivalBasis != -1)
				{
					x = x - portals[i].getX() + portals[i].getXdestination();
					y = y - portals[i].getY() + portals[i].getYdestination();
					if (portals[i].getTimeDirection() * timeDirection == TimeDirection::FORWARDS)
					{
						xspeed = xspeed - portals[i].getXspeed();
						yspeed = yspeed - portals[i].getYspeed();
					}
					else
					{
						xspeed = xspeed + portals[i].getXspeed();
						yspeed = yspeed + portals[i].getYspeed();
					}
				}
				else
				{
					x = x + portals[i].getXdestination();
					y = y + portals[i].getYdestination();
				}
				//std::cerr << "yspeed " << yspeed << "\n";
				normalDeparture = false;
				break;
			}
		}
	}

	nextBoxNormalDeparture.push_back(normalDeparture);

	// add box
	nextBox.push_back(
		ObjectAndTime<Box, FrameT>(
			Box(
				x,
				y,
				xspeed,
				yspeed,
				width,
				height,
				boxType,
				state,
				illegalPortal,
				arrivalBasis,
				timeDirection),
			nextTime));
}

template<
	typename RandomAccessGuyRange,
	typename RandomAccessBoxRange>
void guyMovement(
	Environment const &env,
	RandomAccessGuyRange const &guyArrivalList,
	std::vector<GuyInput> const &playerInput,
	int const i,
	mp::std::vector<int> &x,
	mp::std::vector<int> &y,
	mp::std::vector<int> &xspeed,
	mp::std::vector<int> &yspeed,
	mp::std::vector<int> &walkSpeed,
	mp::std::vector<int> &jumpHold,
	mp::std::vector<GuyAction> &action,
	mp::std::vector<char> &supported,
	mp::std::vector<int> &supportedSpeed,
	mp::std::vector<char> &finishedWith,
	mp::std::vector<FacingDirection> &facing,
	mp::std::vector<ObjectAndTime<Box, Frame *>> &nextBox,
	mp::std::vector<char> &nextBoxNormalDeparture,
	RandomAccessBoxRange const &boxArrivalList,
	mp::std::vector<Collision> const &nextPlatform,
	mt::std::vector<ExplosionEffect> &explosions,
	GuyGlitzAdder const &guyGlitzAdder)
{
	std::size_t relativeIndex(guyArrivalList[i].getIndex());
	GuyInput const &input(playerInput[relativeIndex]);

	int const width(guyArrivalList[i].getWidth());
	int const height(guyArrivalList[i].getHeight());
	int const jumpSpeed(guyArrivalList[i].getJumpSpeed());

	// Chonofrag with platforms
	for (Collision const &platform : nextPlatform)
	{
		if (platform.getFunctional()) {
			if (platform.getCollisionType() == CollisionType::PLATFORM)
			{
				int pX(platform.getX());
				int pY(platform.getY());
				int pWidth(platform.getWidth());
				int pHeight(platform.getHeight());
				TimeDirection pDirection(platform.getTimeDirection());
				if (guyArrivalList[i].getTimePaused())
				{
					if (IntersectingRectanglesExclusive(x[i], y[i], width, height, pX, pY, pWidth, pHeight))
					{
						finishedWith[i] = true;
						continue;
					}
				}
				else if (pDirection * guyArrivalList[i].getTimeDirection() == TimeDirection::FORWARDS)
				{
					pX -= platform.getXspeed();
					pY -= platform.getYspeed();
					if (IntersectingRectanglesExclusive(x[i], y[i], width, height, pX, pY, pWidth, pHeight))
					{
						finishedWith[i] = true;
						guyGlitzAdder.addDeathGlitz(x[i], y[i], width, height, guyArrivalList[i].getTimeDirection());
						continue;
					}
				}
				else
				{
					pX -= platform.getXspeed();
					pY -= platform.getYspeed();
					if (IntersectingRectanglesExclusive(x[i], y[i], width, height, pX, pY, pWidth, pHeight))
					{
						finishedWith[i] = true;
						guyGlitzAdder.addDeathGlitz(x[i], y[i], width, height, guyArrivalList[i].getTimeDirection());
						continue;
					}
				}
			}
		}
	}

	// chonofrag with walls
	if (wallAtExclusive(env, x[i], y[i], width, height))
	{
		finishedWith[i] = true;
		if (!guyArrivalList[i].getTimePaused()) {
			guyGlitzAdder.addDeathGlitz(x[i], y[i], width, height, guyArrivalList[i].getTimeDirection());
		}
		return;
	}

	bool bottom = false;
	bool top = false;
	bool left = false;
	bool right = false;

	//std::size_t boxThatIamStandingOn(std::numeric_limits<std::size_t>::max());

	// jump
	if (input.getjump() || (jumpHold[i] > 0 && jumpHold[i] < hg::GUY_JUMP_HOLD_MIN && !guyArrivalList[i].getSupported()))
	{
		if (guyArrivalList[i].getSupported())
		{
			if (guyArrivalList[i].getSupported() >= hg::GUY_SUPPORT_JUMP_REQ)
			{
				yspeed[i] = guyArrivalList[i].getSupportedSpeed() + jumpSpeed;
				jumpHold[i] = 1;
			}
		}
		else if (jumpHold[i] > 0 && jumpHold[i] < hg::GUY_JUMP_HOLD_MAX)
		{
			BoxType carryType = guyArrivalList[i].getBoxCarrying();
			if (carryType != BoxType::NONE && carryType != BoxType::BALLOON) {
				yspeed[i] += (hg::GUY_JUMP_HOLD_SPEED + jumpHold[i] * jumpHold[i] / 32) / 3;
			}
			else {
				yspeed[i] += hg::GUY_JUMP_HOLD_SPEED + jumpHold[i] * jumpHold[i] / 32;
			}
			jumpHold[i] += 1;
		}
		else {
			jumpHold[i] = 0;
		}
	}
	else {
		jumpHold[i] = 0;
	}

	// Y direction collisions
	//std::cerr << "Guy " << guyArrivalList[i].getIndex() << " Pos: " << y[i] << ", " << yspeed[i] << ", Sup: " << guyArrivalList[i].getSupported() << "\n";
	int newY(y[i] + yspeed[i]);
	
	if (guyArrivalList[i].getTimePaused()) {
		// box collision (only occurs in Y direction)
		for (std::size_t j(0), jsize(nextBox.size()); j < jsize; ++j)
		{
			if (nextBoxNormalDeparture[j] && boxCollidable(nextBox[j].object.getBoxType()))
			{
				int boxX(nextBox[j].object.getX());
				int boxY(nextBox[j].object.getY());
				int boxWidth(nextBox[j].object.getWidth());
				if (x[i] < boxX + boxWidth && x[i] + width > boxX)
				{
					if (newY + height >= boxY && newY - yspeed[i] + height <= boxY)
					{
						//boxThatIamStandingOn = j;
						newY = boxY - height;
						xspeed[i] = 0;
						supported[i] = std::min(hg::GUY_MAX_SUPPORTED, guyArrivalList[i].getSupported() + 1);
						bottom = true;
						supportedSpeed[i] = 0;
					}
				}
			}
		}
	}
	else {
		// Forwards box collision (only occurs in Y direction)
		for (std::size_t j(0), jsize(nextBox.size()); j < jsize; ++j)
		{
			if (nextBoxNormalDeparture[j] && boxCollidable(nextBox[j].object.getBoxType()) && nextBox[j].object.getTimeDirection() * guyArrivalList[i].getTimeDirection() == TimeDirection::FORWARDS)
			{
				int boxX(nextBox[j].object.getX());
				int boxY(nextBox[j].object.getY());
				int boxXspeed(nextBox[j].object.getXspeed());
				int boxYspeed(nextBox[j].object.getYspeed());

				int boxWidth(nextBox[j].object.getWidth());
				if (x[i] < boxX + boxWidth && x[i] + width > boxX && newY + height >= boxY && newY + height - yspeed[i] <= boxY - boxYspeed)
				{
					//boxThatIamStandingOn = j;
					newY = boxY - height;
					xspeed[i] = boxXspeed;
					supported[i] = std::min(hg::GUY_MAX_SUPPORTED, guyArrivalList[i].getSupported() + 1);
					bottom = true;
					supportedSpeed[i] = boxYspeed; 
				}
			}
		}

		// Reverse box collision
		for (std::size_t j(0), jsize(boxArrivalList.size()); j < jsize; ++j)
		{
			if (boxCollidable(boxArrivalList[j].getBoxType()) && boxArrivalList[j].getTimeDirection() * guyArrivalList[i].getTimeDirection() == TimeDirection::REVERSE)
			{
				int boxXspeed(boxArrivalList[j].getXspeed());
				int boxYspeed(boxArrivalList[j].getYspeed());

				int boxX(boxArrivalList[j].getX() - boxXspeed);
				int boxY(boxArrivalList[j].getY() - boxYspeed);

				int boxWidth(boxArrivalList[j].getWidth());

				// -hg::DOWN_GRAVITY feels like hax but probably isn't. The print out shows that it is a requirement
				if (x[i] < boxX + boxWidth && x[i] + width > boxX && newY + height >= boxY && newY + height - yspeed[i] - hg::DOWN_GRAVITY <= boxY + boxYspeed)
				{
					newY = boxY - height;
					xspeed[i] = -boxXspeed;
					supported[i] = std::min(hg::GUY_MAX_SUPPORTED, guyArrivalList[i].getSupported() + 1);
					bottom = true;
					supportedSpeed[i] = -boxYspeed;
				}
			}
		}
	}

	// Platform collision in Y direction
	for (Collision const &platform : nextPlatform)
	{
		if (platform.getFunctional()) {
			int pX(platform.getX());
			int pY(platform.getY());
			TimeDirection pDirection(platform.getTimeDirection());
			int pWidth(platform.getWidth());
			int pHeight(platform.getHeight());

			if (!guyArrivalList[i].getTimePaused() && pDirection != guyArrivalList[i].getTimeDirection())
			{
				pX -= platform.getXspeed() + platform.getPrevXspeed();
				pY -= platform.getYspeed() + platform.getPrevYspeed();
			}

			if (IntersectingRectanglesExclusive(
				x[i], newY, width, height,
				pX, pY, pWidth, pHeight))
			{
				if (platform.getCollisionType() == CollisionType::PLATFORM)
				{
					int colDir = RectangleIntersectionDirection(x[i], y[i], width, height, pX, pY, pWidth, pHeight);
					if (colDir == 1)
					{
						newY = pY - height;
						bottom = true;
						if (guyArrivalList[i].getTimePaused()) {
							xspeed[i] = 0;
							supported[i] = std::min(hg::GUY_MAX_SUPPORTED, guyArrivalList[i].getSupported() + 1);
							supportedSpeed[i] = 0;
						}
						else {
							xspeed[i] = static_cast<int>(pDirection * guyArrivalList[i].getTimeDirection()) * platform.getXspeed();
							supported[i] = std::min(hg::GUY_MAX_SUPPORTED, guyArrivalList[i].getSupported() + 1);
							supportedSpeed[i] = static_cast<int>(pDirection * guyArrivalList[i].getTimeDirection()) * platform.getYspeed();
						}
					}
					else if (colDir == 3)
					{
						newY = pY + pHeight;
						top = true;
					}
				}
			}
		}
	}

	//check wall collision in Y direction
	if (yspeed[i] > 0) // down
	{
		if (env.wall.at(x[i], newY + height) || (x[i] - (x[i] / env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize() - width && env.wall.at(x[i] + width, newY + height)))
		{
			newY = ((newY + height) / env.wall.segmentSize())*env.wall.segmentSize() - height;
			supported[i] = std::min(hg::GUY_MAX_SUPPORTED, guyArrivalList[i].getSupported() + 1);
			bottom = true;
			supportedSpeed[i] = 0;
			while (newY > 0 && (env.wall.at(x[i], newY + height - 1) ||
				(x[i] - (x[i] / env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize() - width && env.wall.at(x[i] + width, newY + height - 1))))
			{
				newY = newY - env.wall.segmentSize();
			}
		}
	}
	else if (yspeed[i] < 0) // up
	{
		if (env.wall.at(x[i], newY) || (x[i] - (x[i] / env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize() - width && env.wall.at(x[i] + width, newY)))
		{
			newY = (newY / env.wall.segmentSize() + 1)*env.wall.segmentSize();
			top = true;
			while (newY + height < env.wall.roomHeight() && (env.wall.at(x[i], newY + 1) ||
				(x[i] - (x[i] / env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize() - width && env.wall.at(x[i] + width, newY + 1))))
			{
				newY = newY + env.wall.segmentSize();
			}
		}
	}

	if (bottom && top)
	{
		finishedWith[i] = true;
		guyGlitzAdder.addDeathGlitz(x[i], y[i], width, height, guyArrivalList[i].getTimeDirection());
		return;
	}

	// X direction stuff
	int newX(x[i] + xspeed[i]);

	//check wall collision in X direction
	if (input.getMoveLeft()) {
		facing[i] = FacingDirection::LEFT;
		if (walkSpeed[i] > 0) {
			walkSpeed[i] += -hg::GUY_HOR_SLOW;
		}
		else {
			walkSpeed[i] = std::max(-hg::GUY_SPEED, walkSpeed[i] - hg::GUY_HOR_ACCEL);
		}
	}
	else if (input.getMoveRight()) {
		facing[i] = FacingDirection::RIGHT;
		if (walkSpeed[i] < 0) {
			walkSpeed[i] += hg::GUY_HOR_SLOW;
		}
		else {
			walkSpeed[i] = std::min(hg::GUY_SPEED, walkSpeed[i] + hg::GUY_HOR_ACCEL);
		}
	}
	else if (walkSpeed[i] > 0) {
		walkSpeed[i] = std::max(0, walkSpeed[i] - hg::GUY_HOR_SLOW);
	}
	else if (walkSpeed[i] < 0) {
		walkSpeed[i] = std::min(0, walkSpeed[i] + hg::GUY_HOR_SLOW);
	}
	newX += walkSpeed[i];

	// platform collision
	for (Collision const &platform : nextPlatform)
	{
		if (platform.getFunctional()) {
			int pX(platform.getX());
			int pY(platform.getY());
			int pWidth = platform.getWidth();
			int pHeight = platform.getHeight();

			if (!guyArrivalList[i].getTimePaused() && platform.getTimeDirection() != guyArrivalList[i].getTimeDirection())
			{
				pX -= platform.getXspeed() + platform.getPrevXspeed();
				pY -= platform.getYspeed() + platform.getPrevYspeed();
			}

			if (IntersectingRectanglesExclusive(newX, newY, width, height, pX, pY, pWidth, pHeight))
			{
				if (platform.getCollisionType() == CollisionType::PLATFORM)
				{
					if (x[i] + width / 2 < pX + pWidth / 2)
					{
						newX = pX - width;
						right = true;
					}
					else
					{
						newX = pX + pWidth;
						left = true;
					}
				}
			}
		}
	}

	if (newX - x[i] > 0) // right
	{
		if (env.wall.at(newX + width, newY) || (newY - (newY / env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize() - height && env.wall.at(newX + width, newY + height)))
		{
			newX = (newX + width) / env.wall.segmentSize()*env.wall.segmentSize() - width;
			right = true;
			while (newX > 0 && (env.wall.at(newX + width - 1, newY) ||
				(newY - (newY / env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize() - height && env.wall.at(newX + width - 1, newY + height))))
			{
				newX = newX - env.wall.segmentSize();
			}
		}
	}
	else if (newX - x[i] < 0) // left
	{
		if (env.wall.at(newX, newY) || (newY - (newY / env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize() - height && env.wall.at(newX, newY + height)))
		{
			newX = (newX / env.wall.segmentSize() + 1)*env.wall.segmentSize();
			left = true;
			while (newX + width < env.wall.roomWidth() && (env.wall.at(newX + 1, newY) ||
				(newY - (newY / env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize() - height && env.wall.at(newX + 1, newY + height))))
			{
				newX = newX + env.wall.segmentSize();
			}
		}
	}

	if (left && right)
	{
		finishedWith[i] = true;
		guyGlitzAdder.addDeathGlitz(x[i], y[i], width, height, guyArrivalList[i].getTimeDirection());
		return;
	}

	for (std::size_t j(0), jsize(std::size(explosions)); j < jsize; ++j) {
		if (DistanceToRectangle(explosions[j].x, explosions[j].y, x[i], y[i], width, height) <= explosions[j].radius) {
			//std::cerr << "ex " << explosions[j].x << ", ey " << explosions[j].y << ", x " << x[i] << ", y " << y[i] << ", w " << width << ", h " << height << ", r " << explosions[j].radius << "\n";
			finishedWith[i] = true;
			guyGlitzAdder.addDeathGlitz(x[i], y[i], width, height, guyArrivalList[i].getTimeDirection());
			return;
		}
	}

	// Apply Change
	xspeed[i] = newX - x[i];
	yspeed[i] = newY - y[i];

	x[i] = newX;
	y[i] = newY;

	//std::cerr << "Speed: " << xspeed[i] << ", Pos: " << newX << "\n";
}

template<
	typename RandomAccessGuyRange,
	typename RandomAccessBoxRange>
void guyStep(
	Environment const &env,
	RandomAccessGuyRange const &guyArrivalList,
	Frame *frame,
	std::vector<GuyInput> const &playerInput,
	mp::std::vector<ObjectAndTime<Guy, Frame *>> &nextGuy,
	mp::std::vector<ObjectAndTime<Box, Frame *>> &nextBox,
	mp::std::vector<char> &nextBoxNormalDeparture,
	RandomAccessBoxRange const &boxArrivalList,
	mp::std::vector<Collision> const &nextPlatform,
	mp::std::vector<PortalArea> const &nextPortal,
	mp::std::vector<ArrivalLocation> const &arrivalLocations,
	mp::std::vector<MutatorArea> const &mutators,
	mt::std::vector<ExplosionEffect> &explosions,
	TriggerFrameState &triggerFrameState,
	GuyGlitzAdder const &guyGlitzAdder,
	bool &winFrame,
	memory_pool<user_allocator_tbb_alloc> &pool)
{
	mp::std::vector<int> x(pool);
	mp::std::vector<int> y(pool);
	mp::std::vector<int> xspeed(pool);
	mp::std::vector<int> yspeed(pool);
	mp::std::vector<int> walkSpeed(pool);
	mp::std::vector<int> jumpHold(pool);
	mp::std::vector<GuyAction> action(pool);
	mp::std::vector<char> supported(pool);
	mp::std::vector<int> supportedSpeed(pool);
	mp::std::vector<char> finishedWith(pool);
	mp::std::vector<FacingDirection> facing(pool);

	x.reserve(boost::size(guyArrivalList));
	y.reserve(boost::size(guyArrivalList));
	xspeed.reserve(boost::size(guyArrivalList));
	yspeed.reserve(boost::size(guyArrivalList));
	walkSpeed.reserve(boost::size(guyArrivalList));
	jumpHold.reserve(boost::size(guyArrivalList));
	action.reserve(boost::size(guyArrivalList));
	supported.reserve(boost::size(guyArrivalList));
	supportedSpeed.reserve(boost::size(guyArrivalList));
	finishedWith.reserve(boost::size(guyArrivalList));
	facing.reserve(boost::size(guyArrivalList));

	// position, velocity, collisions
	// check collisions in Y direction then do the same in X direction
	for (std::size_t i(0), isize(boost::size(guyArrivalList)); i < isize; ++i)
	{
		walkSpeed.push_back(guyArrivalList[i].getWalkSpeed());
		jumpHold.push_back(guyArrivalList[i].getJumpHold());
		action.push_back(guyArrivalList[i].getAction());
		// initialise positions with arrivalBasis
		if (guyArrivalList[i].getArrivalBasis() == -1)
		{
			x.push_back(guyArrivalList[i].getX());
			y.push_back(guyArrivalList[i].getY());
			xspeed.push_back(0);
			yspeed.push_back(guyArrivalList[i].getYspeed());
		}
		else
		{
			ArrivalLocation const &relativePortal(arrivalLocations[guyArrivalList[i].getArrivalBasis()]);
			x.push_back(relativePortal.getX() + guyArrivalList[i].getX());
			y.push_back(relativePortal.getY() + guyArrivalList[i].getY());
			xspeed.push_back(0);
			if (guyArrivalList[i].getTimePaused())
			{
				yspeed.push_back(guyArrivalList[i].getYspeed());
			}
			else
			{
				//std::cerr << "Arrival Loc Speed " << getFrameNumber(frame) << ": " << relativePortal.getYspeed() << "\n";
				if (relativePortal.getTimeDirection() * guyArrivalList[i].getTimeDirection() == TimeDirection::FORWARDS)
				{
					yspeed.push_back(guyArrivalList[i].getYspeed() + relativePortal.getYspeed());
					y[i] = y[i] - relativePortal.getYspeed();
				}
				else
				{
					yspeed.push_back(guyArrivalList[i].getYspeed() - relativePortal.getYspeed());
					y[i] = y[i] + relativePortal.getYspeed();
				}
			}
		}

		if (yspeed[i] > 0) {
			yspeed[i] += getBoxCarryGravity(hg::UP_GRAVITY, guyArrivalList[i].getBoxCarrying(), guyArrivalList[i].getBoxCarryHeight());
		}
		else {
			yspeed[i] += getBoxCarryGravity(hg::DOWN_GRAVITY, guyArrivalList[i].getBoxCarrying(), guyArrivalList[i].getBoxCarryHeight());
		}

		// Floaty behaviour
		//if (yspeed[i] > 0 && yspeed[i] < 12 * env.gravity) {
		//    yspeed[i] -= env.gravity * (12 * env.gravity - yspeed[i]) / (18 * env.gravity);
		//}
		//else if (-yspeed[i] > 0 && -yspeed[i] < 12 * env.gravity) {
		//    yspeed[i] += env.gravity * (12 * env.gravity + yspeed[i]) / (18 * env.gravity);
		//}
		//std::cerr << jumpHold[i] << ", " << yspeed[i] << "\n";

		// Soft speed limit
		yspeed[i] += -guyArrivalList[i].getYspeed() * guyArrivalList[i].getYspeed() * guyArrivalList[i].getYspeed() / hg::VERT_AIR_RESISTANCE;

		supported.push_back(0);
		supportedSpeed.push_back(0);
		finishedWith.push_back(false);
		facing.push_back(guyArrivalList[i].getFacing());

		//std::cerr << "Pre Physics Speed " << getFrameNumber(frame) << ": " << yspeed[i] << "\n";

		// Check with triggers if guy should affect frame
		if (!triggerFrameState.shouldArrive(guyArrivalList[i]))
		{
			finishedWith[i] = true;
			continue;
		}

		// Collision algo
		if (guyArrivalList[i].getIndex() < playerInput.size() && !guyArrivalList[i].getTimePaused())
		{
			guyMovement(env, guyArrivalList, playerInput,
				i, x, y, xspeed, yspeed, walkSpeed, jumpHold, action,
				supported, supportedSpeed, finishedWith, facing,
				nextBox, nextBoxNormalDeparture, boxArrivalList,
				nextPlatform, explosions, guyGlitzAdder);
		}
	}
	
	assert(boost::size(x) == boost::size(guyArrivalList));
	assert(boost::size(y) == boost::size(guyArrivalList));
	assert(boost::size(xspeed) == boost::size(guyArrivalList));
	assert(boost::size(yspeed) == boost::size(guyArrivalList));
	assert(boost::size(walkSpeed) == boost::size(guyArrivalList));
	assert(boost::size(jumpHold) == boost::size(guyArrivalList));
	assert(boost::size(action) == boost::size(guyArrivalList));
	assert(boost::size(supported) == boost::size(guyArrivalList));
	assert(boost::size(supportedSpeed) == boost::size(guyArrivalList));
	assert(boost::size(finishedWith) == boost::size(guyArrivalList));
	assert(boost::size(facing) == boost::size(guyArrivalList));

	mp::std::vector<BoxType> carry(guyArrivalList.size(), pool);
	mp::std::vector<int> carryWidth(guyArrivalList.size(), pool);
	mp::std::vector<int> carryHeight(guyArrivalList.size(), pool);
	mp::std::vector<TimeDirection> carryDirection(guyArrivalList.size(), pool);
	mp::std::vector<char> justPickedUpBox(guyArrivalList.size(), pool);

	// Do movement for pause guys. Do box manipulation for all guys.
	// This is to make pause guys not affect their past selves with box manipulation.
	for (std::size_t i(0), isize(boost::size(guyArrivalList)); i < isize; ++i)
	{
		if (guyArrivalList[i].getIndex() < playerInput.size() && guyArrivalList[i].getTimePaused())
		{
			guyMovement(env, guyArrivalList, playerInput,
				i, x, y, xspeed, yspeed, walkSpeed, jumpHold, action,
				supported, supportedSpeed, finishedWith, facing,
				nextBox, nextBoxNormalDeparture, boxArrivalList,
				nextPlatform, explosions, guyGlitzAdder);
		}

		// Box Manipulation
		if (guyArrivalList[i].getIndex() < playerInput.size() && !finishedWith[i])
		{
			carry[i] = guyArrivalList[i].getBoxCarrying();
			carryWidth[i] = 0;
			carryHeight[i] = 0;
			carryDirection[i] = TimeDirection::INVALID;

			std::size_t const relativeIndex(guyArrivalList[i].getIndex());
			GuyInput const &input(playerInput[relativeIndex]);

			if (carry[i] != BoxType::NONE)
			{
				bool droppable(false);
				if (input.getBoxAction())
				{
					int width(guyArrivalList[i].getWidth());
					int height(guyArrivalList[i].getHeight());
					int dropWidth(guyArrivalList[i].getBoxCarryWidth());
					int dropHeight(guyArrivalList[i].getBoxCarryHeight());

					int gX(x[i]);
					int gY(y[i]);
					int startDropHeight = gY + height - 1; // -1 prevents jumping off dropped boxes while falling.
					if (guyArrivalList[i].getBoxCarryDirection()*guyArrivalList[i].getTimeDirection() == TimeDirection::REVERSE) {
						gX = guyArrivalList[i].getX() - guyArrivalList[i].getXspeed();
						gY = guyArrivalList[i].getY() - guyArrivalList[i].getYspeed();
						startDropHeight = gY + height; // -1 causes paradox issues when dropping off cliffs..
					}
					if (carry[i] == BoxType::BALLOON) {
						startDropHeight = startDropHeight - height;
					}
					int dropY = startDropHeight;
					if (carry[i] == BoxType::BALLOON) {
						if (dropY + dropHeight > gY + height/2) {
							dropY = gY - dropHeight + height/2;
						}
					}
					//std::cerr << "== Guy Dropping Box == " << gX << ", " << gY << "\n";

					while (dropY >= startDropHeight - dropHeight && !droppable)
					{
						//std::cerr << "Try: " << gX << ", " << gY << "\n";
						// Track next attempted drop height
						int nextDropY = startDropHeight - dropHeight - 1;

						// Initialize bounds on drops based on movement direction
						int leftBound, rightBound;

						if (dropWidth < width)
						{
							leftBound = gX;
							rightBound = gX - dropWidth + width;
						}
						else
						{
							leftBound = gX - dropWidth + width;
							rightBound = gX;
						}

						if (facing[i] == FacingDirection::LEFT) {
							leftBound = gX - dropWidth + width / 2;
						}
						else if (facing[i] == FacingDirection::RIGHT) {
							rightBound = gX + width / 2;
						}

						//std::cerr << "Initial Bound " << leftBound << ", " << rightBound << "\n";
						//std::cerr << "Drop Y " << dropY << ", " << nextDropY << "\n";

						int initial_cy = (dropY + dropHeight - 1) - (dropY + dropHeight - 1) % env.wall.segmentSize(); // Top of lowest wall

						// Narrow drop bounds with wall collision
						int cx = (rightBound) - (rightBound) % env.wall.segmentSize();
						int bound_cx = (rightBound + dropWidth - 1) - ((rightBound + dropWidth - 1) % env.wall.segmentSize());
						
						//std::cerr << "Check X " << cx << "\n";
						while (cx <= bound_cx)
						{
							int cy = initial_cy; // Top of lowest wall
							while (cy >= dropY - env.wall.segmentSize() + 1)
							{
								if (env.wall.at(cx, cy))
								{
									rightBound = cx - dropWidth;
									int topOfWall = cy - dropHeight;
									if (topOfWall > nextDropY) {
										nextDropY = topOfWall;
									}
									goto rightBoundCheckDoubleBreak;
								}
								cy -= env.wall.segmentSize();
							}
							cx += env.wall.segmentSize();
						}
					rightBoundCheckDoubleBreak:;

						//std::cerr << "After Wall Right " << leftBound << ", " << rightBound << "\n";
						//std::cerr << "Drop Y " << dropY << ", " << nextDropY << "\n";

						cx = (leftBound + dropWidth - 1) - ((leftBound + dropWidth - 1) % env.wall.segmentSize());
						bound_cx = (leftBound) - ((leftBound) % env.wall.segmentSize());
						
						//std::cerr << "Check X " << cx << "\n";
						while (cx >= bound_cx)
						{
							int cy = initial_cy;
							while (cy >= dropY - env.wall.segmentSize() + 1)
							{
								if (env.wall.at(cx, cy))
								{
									leftBound = cx + env.wall.segmentSize();
									int topOfWall = cy - dropHeight;
									if (topOfWall > nextDropY) {
										nextDropY = topOfWall;
									}
									goto leftBoundCheckDoubleBreak;
								}
								cy -= env.wall.segmentSize();
							}
							cx -= env.wall.segmentSize();
						}
					leftBoundCheckDoubleBreak:;

						//std::cerr << "After Wall Left " << leftBound << ", " << rightBound << "\n";
						//std::cerr << "Drop Y " << dropY << ", " << nextDropY << "\n";

						// Check bounds imposed by platforms
						if (rightBound >= leftBound)
						{
							for (std::size_t j(0), jsize(nextPlatform.size()); j < jsize; ++j)
							{
								if (nextPlatform[j].getFunctional()) {
									int px = nextPlatform[j].getX();
									int py = nextPlatform[j].getY();
									int pw = nextPlatform[j].getWidth();
									int ph = nextPlatform[j].getHeight();

									if (guyArrivalList[i].getBoxCarryDirection()*nextPlatform[j].getTimeDirection() == TimeDirection::REVERSE)
									{
										px -= nextPlatform[j].getXspeed() + nextPlatform[j].getPrevXspeed();
										py -= nextPlatform[j].getYspeed() + nextPlatform[j].getPrevYspeed();
									}

									if (IntersectingRectanglesExclusive(
										px, py, pw, ph,
										leftBound, dropY, rightBound - leftBound + dropWidth, dropHeight))
									{
										if (nextPlatform[j].getCollisionType() == CollisionType::PLATFORM)
										{
											if (px + pw > leftBound + dropWidth && px < rightBound + dropWidth)
											{
												rightBound = px - dropWidth;
												if (py - dropHeight > nextDropY) {
													nextDropY = py - dropHeight;
												}
											}
											if (px < rightBound && px + pw > leftBound)
											{
												leftBound = px + pw;
												if (py - dropHeight > nextDropY) {
													nextDropY = py - dropHeight;
												}
											}
											if (rightBound < leftBound)
											{
												break;
											}
										}
									}
								}
							}
						}
						//std::cerr << "After Plat " << leftBound << ", " << rightBound << "\n";
						//std::cerr << "Drop Y " << dropY << ", " << nextDropY << "\n";

						// Check bounds imposed by boxes
						if (rightBound >= leftBound)
						{
							for (std::size_t j(0), jsize(nextBox.size()); j < jsize; ++j)
							{
								if (nextBoxNormalDeparture[j] && boxCollidable(carry[i]) && boxCollidable(nextBox[j].object.getBoxType()))
								{
									int bx = nextBox[j].object.getX();
									int by = nextBox[j].object.getY();
									int bw = nextBox[j].object.getWidth();
									int bh = nextBox[j].object.getHeight();

									if (guyArrivalList[i].getBoxCarryDirection()*nextBox[j].object.getTimeDirection() == TimeDirection::REVERSE)
									{
										bx -= nextBox[j].object.getXspeed();
										by -= nextBox[j].object.getYspeed();
									}

									//std::cerr << "Index: " << j <<  ", x: " << bx << ", y: " << by << ", w: " << bw << ", h: " << bh << "\n";
									//std::cerr << "Dropper l: " << leftBound << ", r: " << rightBound << ", y: " << dropY << ", w: " << dropWidth <<  ", h: " << dropHeight << "\n";
									if (IntersectingRectanglesExclusive(
										bx, by, bw, bh,
										leftBound, dropY, rightBound - leftBound + dropWidth, dropHeight))
									{
										//std::cerr << "rightChange: " << (bx < rightBound + dropWidth && (bx + bw > rightBound + dropWidth || (rightBound + dropWidth - (bx + bw)) < dropWidth)) << "\n";
										if (bx < rightBound + dropWidth && (bx + bw > rightBound + dropWidth || (rightBound + dropWidth - (bx + bw)) < dropWidth))
										{
											rightBound = bx - dropWidth;
											if (by - dropHeight > nextDropY) {
												nextDropY = by - dropHeight;
											}
										}
										//std::cerr << "leftChange: " << (bx + bw > leftBound && (leftBound > bx || (bx - leftBound) < dropWidth)) << "\n";
										if (bx + bw > leftBound && (leftBound > bx || (bx - leftBound) < dropWidth))
										{
											leftBound = bx + bw;
											if (by - dropHeight > nextDropY) {
												nextDropY = by - dropHeight;
											}
										}
										if (rightBound < leftBound)
										{
											break;
										}
									}
								}
							}
						}

						//std::cerr << "After Box " << leftBound << ", " << rightBound << " dropY " << nextDropY << "\n";

						droppable = rightBound >= leftBound;

						if (droppable)
						{
							// Choose where to drop it within bound
							int dropX;
							if (facing[i] == FacingDirection::LEFT) {
								dropX = leftBound;
							}
							else if (facing[i] == FacingDirection::RIGHT) {
								dropX = rightBound;
							}
							else {
								// Never seen?
								assert(false);
								int midX = gX + width / 2 - dropWidth / 2;
								if (leftBound <= midX) {
									if (midX <= rightBound)
									{
										dropX = midX;
									}
									else
									{
										dropX = rightBound;
									}
								}
								else
								{
									dropX = leftBound;
								}
							}
							//std::cerr << "Dropped Box " << dropX << ", " << dropY << "\n";

							// Add box
							makeBoxAndTimeWithPortalsAndMutators(
								nextBox,
								nextBoxNormalDeparture,
								nextPortal,
								mutators,
								dropX,
								dropY,
								0,
								guyArrivalList[i].getSupportedSpeed(),
								dropWidth,
								dropHeight,
								carry[i],
								getBoxDropState(carry[i]),
								-1,
								guyArrivalList[i].getBoxCarryDirection(),
								triggerFrameState,
								frame,
								pool);

							carry[i] = BoxType::NONE;
							carryWidth[i] = 0;
							carryHeight[i] = 0;
							carryDirection[i] = TimeDirection::INVALID;
						}
						else // !droppable
						{
							dropY = nextDropY;
						}
					}
				}

				if (!droppable)
				{
					carryWidth[i] = guyArrivalList[i].getBoxCarryWidth();
					carryHeight[i] = guyArrivalList[i].getBoxCarryHeight();
					carryDirection[i] = guyArrivalList[i].getBoxCarryDirection();
				}
			}
			else
			{
				if (input.getBoxAction())
				{

					int width = guyArrivalList[i].getWidth();
					int height = guyArrivalList[i].getHeight();
					//CAREFUL - loop modifies nextBox
					auto nextBoxIt(nextBox.begin()), nextBoxEnd(nextBox.end());
					auto nextBoxNormalDepartureIt(nextBoxNormalDeparture.begin());
					
					BoxType foundBoxType = BoxType::NONE;
					int foundBoxDistance = 10000000;
					auto foundBoxIt(nextBox.begin());
					auto foundNextBoxNormalDepartureIt(nextBoxNormalDeparture.begin());
					
					for (; nextBoxIt != nextBoxEnd; ++nextBoxIt, ++nextBoxNormalDepartureIt)
					{
						if (*nextBoxNormalDepartureIt)
						{
							int boxX = nextBoxIt->object.getX();
							int boxY = nextBoxIt->object.getY();

							if (nextBoxIt->object.getTimeDirection()*guyArrivalList[i].getTimeDirection() == TimeDirection::REVERSE)
							{
								boxX += -nextBoxIt->object.getXspeed() + x[i] - guyArrivalList[i].getX();
								boxY += -nextBoxIt->object.getYspeed() + y[i] - guyArrivalList[i].getY();
							}

							int boxWidth = nextBoxIt->object.getWidth();
							int boxHeight = nextBoxIt->object.getHeight();
							if (getBoxPickupCollision(x[i], y[i], width, height, boxX, boxY, boxWidth, boxHeight, nextBoxIt->object.getBoxType()))
							{
								int dist = ManhattanDistanceToRectangle(x[i] + width/2, y[i] + height, boxX, boxY, boxWidth, boxHeight);
								//std::cerr << "Dist Box " << dist << ", boxWidth " << boxWidth << "\n";
								if (dist < foundBoxDistance || getBoxPickupPriority(foundBoxType, nextBoxIt->object.getBoxType())) {
									foundBoxType = nextBoxIt->object.getBoxType();
									foundBoxDistance = dist;
									foundBoxIt = nextBoxIt;
									foundNextBoxNormalDepartureIt = nextBoxNormalDepartureIt;
								}
							}
						}
					}

					if (foundBoxType != BoxType::NONE) {
						carry[i] = foundBoxIt->object.getBoxType();
						carryWidth[i] = foundBoxIt->object.getWidth();
						carryHeight[i] = foundBoxIt->object.getHeight();
						carryDirection[i] = foundBoxIt->object.getTimeDirection();
						justPickedUpBox[i] = true;

						nextBox.erase(foundBoxIt);
						nextBoxNormalDeparture.erase(foundNextBoxNormalDepartureIt);
					}
				}
				else
				{
					carryWidth[i] = 0;
					carryHeight[i] = 0;
					carryDirection[i] = TimeDirection::INVALID;
				}
			}
		}
	}

	assert(boost::size(carry) == boost::size(guyArrivalList));
	assert(boost::size(carryWidth) == boost::size(guyArrivalList));
	assert(boost::size(carryHeight) == boost::size(guyArrivalList));
	assert(boost::size(carryDirection) == boost::size(guyArrivalList));

	mp::std::vector<int> newWidth(guyArrivalList.size(), pool);
	mp::std::vector<int> newHeight(guyArrivalList.size(), pool);
	mp::std::vector<int> newJumpSpeed(guyArrivalList.size(), pool);
	mp::std::vector<Pickups> newPickups(guyArrivalList.size(), pool);
	mp::std::vector<int> illegalPortal(guyArrivalList.size(), pool);
	mp::std::vector<int> newTimePaused(guyArrivalList.size(), pool);
	// arrivalBasis is always -1 for normalDeparture

	// time travel, mutator and portal collision, item use
	for (std::size_t i(0), size(guyArrivalList.size()); i != size; ++i)
	{
		if (finishedWith[i])
		{
			assert(guyArrivalList[i].getIndex() != playerInput.size()
				&& "nextPlayerFrame must always be set for guys with"
				" index == playerInput.size()");
			continue;
		}

		if (guyArrivalList[i].getIndex() < playerInput.size())
		{
			std::size_t const relativeIndex(guyArrivalList[i].getIndex());
			GuyInput const &input = playerInput[relativeIndex];

			int arrivalBasis = -1;
			illegalPortal[i] = -1;

			newWidth[i] = guyArrivalList[i].getWidth();
			newHeight[i] = guyArrivalList[i].getHeight();
			newJumpSpeed[i] = guyArrivalList[i].getJumpSpeed();
			newPickups[i] = guyArrivalList[i].getPickups();
			newTimePaused[i] = guyArrivalList[i].getTimePaused();

			TimeDirection nextTimeDirection = guyArrivalList[i].getTimeDirection();
			Frame *nextTime(nextFrame(frame, nextTimeDirection));
			assert(!isNullFrame(frame));

			// Mutators
			// Mutator and falling must occur first due to exact frame effects.
			mp::std::vector<int> mutatorCollisions(pool);
			for (unsigned j = 0; j < mutators.size(); ++j)
			{
				if (IntersectingRectanglesInclusiveCollisionOverlap(
					x[i], y[i], newWidth[i], newHeight[i],
					mutators[j].getX(), mutators[j].getY(),
					mutators[j].getWidth(), mutators[j].getHeight(),
					mutators[j].getCollisionOverlap()))
				{
					mutatorCollisions.push_back(j);
				}
			}

			if (mutatorCollisions.size() != 0)
			{
				boost::optional<Guy> newGuy = triggerFrameState.mutateObject(
					mutatorCollisions,
					Guy(relativeIndex,
						x[i], y[i],
						xspeed[i], yspeed[i],
						walkSpeed[i], jumpHold[i], action[i],
						newWidth[i], newHeight[i],
						newJumpSpeed[i],
						guyArrivalList[i].getIllegalPortal(),
						-1,
						supported[i],
						supportedSpeed[i],
						newPickups[i],
						facing[i],
						carry[i],
						carryWidth[i],
						carryHeight[i],
						carryDirection[i],
						nextTimeDirection,
						newTimePaused[i]
						));
				if (!newGuy)
				{
					finishedWith[i] = true;
					continue;
				}
				x[i] = newGuy->getX();
				y[i] = newGuy->getY();
				xspeed[i] = newGuy->getXspeed();
				yspeed[i] = newGuy->getYspeed();
				walkSpeed[i] = newGuy->getWalkSpeed();
				jumpHold[i] = newGuy->getJumpHold();
				action[i] = newGuy->getAction();
				newWidth[i] = newGuy->getWidth();
				newHeight[i] = newGuy->getHeight();
				newJumpSpeed[i] = newGuy->getJumpSpeed();
				illegalPortal[i] = newGuy->getIllegalPortal();
				// arrivalBasis is missing with good reason
				supported[i] = newGuy->getSupported();
				supportedSpeed[i] = newGuy->getSupportedSpeed();
				newPickups[i] = newGuy->getPickups();
				facing[i] = newGuy->getFacing();
				carry[i] = newGuy->getBoxCarrying();
				carryWidth[i] = newGuy->getBoxCarryWidth();
				carryHeight[i] = newGuy->getBoxCarryHeight();
				carryDirection[i] = newGuy->getBoxCarryDirection();
				nextTimeDirection = newGuy->getTimeDirection();
				newTimePaused[i] = newGuy->getTimePaused();
				// relativeIndex is missing for obvious reasons
			}

			int glitzX = x[i] - xspeed[i];
			int glitzY = y[i] - yspeed[i];

			// Things that do time travel
			// The occurrence of one thing precludes subsequent ones
			bool normalDeparture = true;

			// falling through portals
			if (normalDeparture)
			{
				for (unsigned int j = 0; j < nextPortal.size(); ++j)
				{
					if (nextPortal[j].getFallable() && 
						(!nextPortal[j].getIsLaser()) &&
						IntersectingRectanglesInclusiveCollisionOverlap(
						x[i], y[i], newWidth[i], newHeight[i],
						nextPortal[j].getX(), nextPortal[j].getY(),
						nextPortal[j].getWidth(), nextPortal[j].getHeight(),
						nextPortal[j].getCollisionOverlap()))
					{
						if (guyArrivalList[i].getIllegalPortal() != -1 && nextPortal[j].getIndex() == guyArrivalList[i].getIllegalPortal())
						{
							illegalPortal[i] = j;
						}
						else if (triggerFrameState.shouldPort(
							j,
							Guy(relativeIndex, x[i], y[i], xspeed[i], yspeed[i],
								walkSpeed[i], jumpHold[i], action[i], newWidth[i], newHeight[i],
								newJumpSpeed[i],
								illegalPortal[i], -1,
								supported[i], supportedSpeed[i], newPickups[i], facing[i],
								carry[i], carryWidth[i], carryHeight[i], carryDirection[i], nextTimeDirection, newTimePaused[i]),
							false))
						{
							if (nextPortal[j].getWinner())
							{
								winFrame = true;
								nextTime = nullptr;
								normalDeparture = false;
								break;
							}
							Frame *portalTime(
								nextPortal[j].getRelativeTime() ?
								getArbitraryFrame(
									getUniverse(frame),
									getFrameNumber(frame) + nextPortal[j].getTimeDestination()) :
								getArbitraryFrame(
									getUniverse(frame),
									nextPortal[j].getTimeDestination()));

							if (nextPortal[j].getRelativeDirection())
							{
								nextTimeDirection *= nextPortal[j].getDestinationDirection();
							}
							else
							{
								nextTimeDirection = nextPortal[j].getDestinationDirection();
							}

							nextTime = portalTime ? nextFrame(portalTime, nextTimeDirection) : nullptr;
							normalDeparture = false;
							illegalPortal[i] = nextPortal[j].getIllegalDestination();
							arrivalBasis = nextPortal[j].getDestinationIndex();
							if (arrivalBasis != -1)
							{
								x[i] = x[i] - nextPortal[j].getX() + nextPortal[j].getXdestination();
								y[i] = y[i] - nextPortal[j].getY() + nextPortal[j].getYdestination();
								if (nextPortal[j].getTimeDirection() * nextTimeDirection == TimeDirection::FORWARDS)
								{
									xspeed[i] = xspeed[i] - nextPortal[j].getXspeed();
									yspeed[i] = yspeed[i] - nextPortal[j].getYspeed();
								}
								else
								{
									xspeed[i] = xspeed[i] + nextPortal[j].getXspeed();
									yspeed[i] = yspeed[i] + nextPortal[j].getYspeed();
								}
							}
							else
							{
								x[i] = x[i] + nextPortal[j].getXdestination();
								y[i] = y[i] + nextPortal[j].getYdestination();
							}
							break;
						}
					}
				}
			}

			// for single frame effects
			// "forced" departures occur before those due to input
			if (normalDeparture)
			{
				auto timeJump(newPickups[i].find(Ability::TIME_JUMP));
				auto timeReverse(newPickups[i].find(Ability::TIME_REVERSE));
				auto timePause(newPickups[i].find(Ability::TIME_PAUSE));

				if (input.getAbilityUsed()) {
					Ability abilityCursor = input.getAbilityCursor();
					if (abilityCursor == Ability::TIME_JUMP && timeJump != newPickups[i].end() && timeJump->second != 0)
					{
						nextTime = getArbitraryFrame(getUniverse(frame), getFrameNumber(input.getTimeCursor()));
						normalDeparture = false;
						if (timeJump->second > 0)
						{
							newPickups[i][Ability::TIME_JUMP] = timeJump->second - 1;
						}
					}
					else if (abilityCursor == Ability::TIME_REVERSE && timeReverse != newPickups[i].end() && timeReverse->second != 0)
					{
						normalDeparture = false;
						nextTimeDirection *= -1;
						nextTime = newTimePaused[i] ? frame : nextFrame(frame, nextTimeDirection);
						carryDirection[i] *= -1;
						if (timeReverse->second > 0)
						{
							newPickups[i][Ability::TIME_REVERSE] = timeReverse->second - 1;
						}
					}
					else if (abilityCursor == Ability::TIME_PAUSE && timePause != newPickups[i].end() && timePause->second != 0)
					{
						nextTime = frame;
						newTimePaused[i] = !newTimePaused[i];
						normalDeparture = false;
						if (timePause->second > 0)
						{
							newPickups[i][Ability::TIME_PAUSE] = timePause->second - 1;
						}
					}
				}
				else if (input.getPortalUsed())
				{
					for (unsigned int j = 0; j < nextPortal.size(); ++j)
					{
						if ((!nextPortal[j].getIsLaser()) &&
							IntersectingRectanglesInclusiveCollisionOverlap(
							x[i], y[i], newWidth[i], newHeight[i],
							nextPortal[j].getX(), nextPortal[j].getY(),
							nextPortal[j].getWidth(), nextPortal[j].getHeight(),
							nextPortal[j].getCollisionOverlap())
							&& (triggerFrameState.shouldPort(j,
								Guy(relativeIndex, x[i], y[i], xspeed[i], yspeed[i],
									walkSpeed[i], jumpHold[i], action[i], newWidth[i], newHeight[i],
									newJumpSpeed[i],
									illegalPortal[i], -1,
									supported[i], supportedSpeed[i], newPickups[i], facing[i],
									carry[i], carryWidth[i], carryHeight[i], carryDirection[i], nextTimeDirection, newTimePaused[i]), true)))
						{
							if (nextPortal[j].getWinner())
							{
								winFrame = true;
								nextTime = nullptr;
								normalDeparture = false;
								break;
							}
							Frame *portalTime(
								nextPortal[j].getRelativeTime() ?
								getArbitraryFrame(
									getUniverse(frame),
									getFrameNumber(frame) + nextPortal[j].getTimeDestination()) :
								getArbitraryFrame(
									getUniverse(frame),
									nextPortal[j].getTimeDestination()));

							if (nextPortal[j].getRelativeDirection())
							{
								nextTimeDirection *= nextPortal[j].getDestinationDirection();
							}
							else
							{
								nextTimeDirection = nextPortal[j].getDestinationDirection();
							}

							nextTime = portalTime ? nextFrame(portalTime, nextTimeDirection) : nullptr;
							normalDeparture = false;
							illegalPortal[i] = nextPortal[j].getIllegalDestination();
							arrivalBasis = nextPortal[j].getDestinationIndex();
							if (arrivalBasis != -1)
							{
								x[i] = x[i] - nextPortal[j].getX() + nextPortal[j].getXdestination();
								y[i] = y[i] - nextPortal[j].getY() + nextPortal[j].getYdestination();
								if (nextPortal[j].getTimeDirection() * nextTimeDirection == TimeDirection::FORWARDS)
								{
									xspeed[i] = xspeed[i] - nextPortal[j].getXspeed();
									yspeed[i] = yspeed[i] - nextPortal[j].getYspeed();
								}
								else
								{
									xspeed[i] = xspeed[i] + nextPortal[j].getXspeed();
									yspeed[i] = yspeed[i] + nextPortal[j].getYspeed();
								}
							}
							else
							{
								x[i] = x[i] + nextPortal[j].getXdestination();
								y[i] = y[i] + nextPortal[j].getYdestination();
							}
							break;
						}
					}
				}
			}

			guyGlitzAdder.addGlitzForGuy(
				vec2<int>{glitzX, glitzY},
				vec2<int>{newWidth[i], newHeight[i]},
				nextTimeDirection,
				facing[i],
				carry[i],
				carryWidth[i],
				carryHeight[i],
				carryDirection[i],
				newTimePaused[i],
				guyArrivalList[i].getIndex(),
				justPickedUpBox[i]);

			if (!normalDeparture)
			{
				nextGuy.push_back(
					ObjectAndTime<Guy, Frame *>(
						Guy(
							relativeIndex + 1,
							x[i], y[i],
							xspeed[i], yspeed[i],
							walkSpeed[i], jumpHold[i], action[i],
							newWidth[i], newHeight[i],
							newJumpSpeed[i],

							illegalPortal[i],
							arrivalBasis,
							supported[i],
							supportedSpeed[i],

							newPickups[i],
							facing[i],

							carry[i],
							carryWidth[i],
							carryHeight[i],
							carryDirection[i],

							nextTimeDirection,
							newTimePaused[i]
							),
						nextTime
						)
					);
				finishedWith[i] = true;
			}
		}
		else
		{
			assert(guyArrivalList[i].getIndex() == playerInput.size());
			finishedWith[i] = true;
		}
	}

	// Get shot by laser portals
	for (unsigned int i = 0; i < nextPortal.size(); ++i)
	{
		std::size_t numberOfGuys = guyArrivalList.size();
		if (nextPortal[i].getIsLaser())
		{
			// Make map of guys which are legal to shoot. Illegal ones are invisible to raytrace
			mp::std::vector<char> shootable(pool);
			shootable.reserve(boost::size(guyArrivalList));

			for (std::size_t j(0); j != numberOfGuys; ++j)
			{
				shootable.push_back(!finishedWith[j] && !newTimePaused[j]);
			}

			// Return values from doGunRaytrace
			//PhysicsObjectType targetType = NONE;
			//std::size_t targetId = std::numeric_limits<std::size_t>::max();

			int const sx = nextPortal[i].getX();
			int const sy = nextPortal[i].getY();
			//int px = nextPortal[i].getXaim();
			//int py = nextPortal[i].getYaim();

			auto const shot = doGunRaytrace(
				sx, sy, nextPortal[i].getXaim(), nextPortal[i].getYaim(),
				env.wall,
				nextPlatform,
				nextBox, nextBoxNormalDeparture,
				x, y, newWidth, newHeight,
				shootable,
				pool);

			// Should be a different glitz adder?
			guyGlitzAdder.addLaserGlitz(sx, sy, shot.px, shot.py, nextPortal[i].getXaim(), nextPortal[i].getYaim(), nextPortal[i].getTimeDirection());

			if (shot.targetType == PhysicsObjectType::GUY)
			{
				assert(shot.targetId != std::numeric_limits<std::size_t>::max());
				assert(shot.targetId >= 0 && shot.targetId < boost::size(guyArrivalList));

				TimeDirection nextTimeDirection = guyArrivalList[shot.targetId].getTimeDirection();
				Frame *nextTime(nextFrame(frame, nextTimeDirection));

				if (nextPortal[i].getWinner())
				{
					winFrame = true;
					nextTime = nullptr;
				}
				else
				{
					Frame *portalTime(
						nextPortal[i].getRelativeTime() ?
						getArbitraryFrame(
							getUniverse(frame),
							getFrameNumber(frame) + nextPortal[i].getTimeDestination()) :
						getArbitraryFrame(
							getUniverse(frame),
							nextPortal[i].getTimeDestination()));


					if (nextPortal[i].getRelativeDirection())
					{
						nextTimeDirection *= nextPortal[i].getDestinationDirection();
					}
					else
					{
						nextTimeDirection = nextPortal[i].getDestinationDirection();
					}

					nextTime = portalTime ? nextFrame(portalTime, nextTimeDirection) : nullptr;
					illegalPortal[shot.targetId] = nextPortal[i].getIllegalDestination();
					int arrivalBasis = nextPortal[i].getDestinationIndex();
					if (arrivalBasis != -1)
					{
						x[shot.targetId] = x[shot.targetId] - nextPortal[i].getX() + nextPortal[i].getXdestination();
						y[shot.targetId] = y[shot.targetId] - nextPortal[i].getY() + nextPortal[i].getYdestination();
						if (nextPortal[i].getTimeDirection() * nextTimeDirection == TimeDirection::FORWARDS)
						{
							xspeed[shot.targetId] = xspeed[shot.targetId] - nextPortal[i].getXspeed();
							yspeed[shot.targetId] = yspeed[shot.targetId] - nextPortal[i].getYspeed();
						}
						else
						{
							xspeed[shot.targetId] = xspeed[shot.targetId] + nextPortal[i].getXspeed();
							yspeed[shot.targetId] = yspeed[shot.targetId] + nextPortal[i].getYspeed();
						}
					}
					else
					{
						x[shot.targetId] = x[shot.targetId] + nextPortal[i].getXdestination();
						y[shot.targetId] = y[shot.targetId] + nextPortal[i].getYdestination();
					}
				}

				nextGuy.push_back(
					ObjectAndTime<Guy, Frame *>(
						Guy(
							guyArrivalList[shot.targetId].getIndex() + 1,
							x[shot.targetId], y[shot.targetId],
							xspeed[shot.targetId], yspeed[shot.targetId],
							walkSpeed[shot.targetId], jumpHold[shot.targetId], action[shot.targetId],
							newWidth[shot.targetId], newHeight[shot.targetId],
							newJumpSpeed[shot.targetId],

							illegalPortal[shot.targetId],
							-1,
							supported[shot.targetId],
							supportedSpeed[shot.targetId],

							newPickups[shot.targetId],
							facing[shot.targetId],

							carry[shot.targetId],
							carryWidth[shot.targetId],
							carryHeight[shot.targetId],
							carryDirection[shot.targetId],

							guyArrivalList[shot.targetId].getTimeDirection(),
							newTimePaused[shot.targetId]
							),
						nextTime
						)
					);

				finishedWith[shot.targetId] = true;
			}
			else if (shot.targetType == PhysicsObjectType::BOX)
			{
				assert(shot.targetId != std::numeric_limits<std::size_t>::max());
				assert(shot.targetId >= 0 && shot.targetId < boost::size(nextBox));

				int boxX = nextBox[shot.targetId].object.getX();
				int boxY = nextBox[shot.targetId].object.getY();
				int boxXspeed = nextBox[shot.targetId].object.getXspeed();
				int boxYspeed = nextBox[shot.targetId].object.getYspeed();
				int boxWidth = nextBox[shot.targetId].object.getWidth();
				int boxHeight = nextBox[shot.targetId].object.getHeight();
				BoxType boxType = nextBox[shot.targetId].object.getBoxType();

				TimeDirection timeDirection = nextBox[shot.targetId].object.getTimeDirection();

				Frame *portalTime(
					nextPortal[i].getRelativeTime() ?
					getArbitraryFrame(getUniverse(frame), getFrameNumber(frame) + nextPortal[i].getTimeDestination()) :
					getArbitraryFrame(getUniverse(frame), nextPortal[i].getTimeDestination()));

				if (nextPortal[i].getRelativeDirection())
				{
					timeDirection *= nextPortal[i].getDestinationDirection();
				}
				else
				{
					timeDirection = nextPortal[i].getDestinationDirection();
				}

				Frame *nextTime = !isNullFrame(portalTime) ? nextFrame(portalTime, timeDirection) : nullptr;
				int arrivalBasis = nextPortal[i].getDestinationIndex();

				if (arrivalBasis != -1)
				{
					boxX = boxX - nextPortal[i].getX() + nextPortal[i].getXdestination();
					boxY = boxY - nextPortal[i].getY() + nextPortal[i].getYdestination();
					if (nextPortal[i].getTimeDirection() * timeDirection == TimeDirection::FORWARDS)
					{
						boxXspeed = boxXspeed - nextPortal[i].getXspeed();
						boxYspeed = boxYspeed - nextPortal[i].getYspeed();
					}
					else
					{
						boxXspeed = boxXspeed + nextPortal[i].getXspeed();
						boxYspeed = boxYspeed + nextPortal[i].getYspeed();
					}
				}
				else
				{
					boxX = boxX + nextPortal[i].getXdestination();
					boxY = boxY + nextPortal[i].getYdestination();
				}

				// Replace box
				assert(shot.targetId < nextBox.size());

				nextBox[shot.targetId] = ObjectAndTime<Box, Frame *>(
						Box(
							boxX,
							boxY,
							boxXspeed,
							boxYspeed,
							boxWidth,
							boxHeight,
							boxType,
							nextBox[shot.targetId].object.getState(),
							nextPortal[i].getIllegalDestination(),
							arrivalBasis,
							timeDirection),
						nextTime);
				nextBoxNormalDeparture[shot.targetId] = false;
			}
			else
			{
				assert(shot.targetType == PhysicsObjectType::NONE);
				assert(shot.targetId == std::numeric_limits<std::size_t>::max());
			}
		}
	}

	// Only normal departures make it this far, time for inter-guy communication (eg guns)
	// Do not want time gun shots to hit before forced things such as spikey death or fallable portals
	// This can now occur because NO POSITION CHANGE IS POSSIBLE PAST THIS POINT
	// Order is important here because we don't want two guys to shoot each other at exactly the same time
	for (std::size_t i(0), size(guyArrivalList.size()); i != size; ++i)
	{
		if (finishedWith[i])
		{
			continue;
		}

		std::size_t const relativeIndex(guyArrivalList[i].getIndex());
		GuyInput const &input = playerInput[relativeIndex];

		auto timeGun(newPickups[i].find(Ability::TIME_GUN));

		if (input.getAbilityUsed()
			&& input.getAbilityCursor() == Ability::TIME_GUN
			&& timeGun != newPickups[i].end()
			&& timeGun->second != 0)
		{
			// Make map of guys which are legal to shoot. Illegal ones are invisible to raytrace
			mp::std::vector<char> shootable(pool);
			shootable.reserve(boost::size(guyArrivalList));

			for (std::size_t j(0); j != size; ++j)
			{
				shootable.push_back(!finishedWith[j] && i != j && !newTimePaused[i]);
			}

			// Parameters for ability. sx, sy, px and py are returned as line glitz params
			Frame * const targetTime = getArbitraryFrame(getUniverse(frame), getFrameNumber(input.getTimeCursor()));
			int const sx = x[i] + newWidth[i] / 2;
			int const sy = y[i] + newHeight[i] / 4;
			//int px = input.getXCursor();
			//int py = input.getYCursor();

			// Return values from doGunRaytrace
			//PhysicsObjectType targetType = NONE;
			//std::size_t targetId = std::numeric_limits<std::size_t>::max();

			auto const shot = doGunRaytrace(
				sx, sy, input.getXCursor(), input.getYCursor(),
				env.wall,
				nextPlatform,
				nextBox, nextBoxNormalDeparture,
				x, y, newWidth, newHeight,
				shootable,
				pool);

			guyGlitzAdder.addLaserGlitz(sx, sy, shot.px, shot.py, input.getXCursor(), input.getYCursor(), guyArrivalList[i].getTimeDirection());

			if (shot.targetType == PhysicsObjectType::GUY)
			{
				assert(shot.targetId != std::numeric_limits<std::size_t>::max());
				assert(shot.targetId >= 0 && shot.targetId < boost::size(guyArrivalList));
				nextGuy.push_back(
					ObjectAndTime<Guy, Frame *>(
						Guy(
							guyArrivalList[shot.targetId].getIndex() + 1,
							x[shot.targetId], y[shot.targetId],
							xspeed[shot.targetId], yspeed[shot.targetId],
							walkSpeed[shot.targetId], jumpHold[shot.targetId], action[shot.targetId],
							newWidth[shot.targetId], newHeight[shot.targetId],
							newJumpSpeed[shot.targetId],

							illegalPortal[shot.targetId],
							-1,
							supported[shot.targetId],
							supportedSpeed[shot.targetId],

							newPickups[shot.targetId],
							facing[shot.targetId],

							carry[shot.targetId],
							carryWidth[shot.targetId],
							carryHeight[shot.targetId],
							carryDirection[shot.targetId],

							guyArrivalList[shot.targetId].getTimeDirection(),
							newTimePaused[shot.targetId]
							),
						targetTime
						)
					);
				finishedWith[shot.targetId] = true;
			}
			else if (shot.targetType == PhysicsObjectType::BOX)
			{
				assert(shot.targetId != std::numeric_limits<std::size_t>::max());
				assert(shot.targetId >= 0 && shot.targetId < boost::size(nextBox));
				nextBox[shot.targetId].frame = targetTime;
				nextBoxNormalDeparture[shot.targetId] = false;
			}
			else
			{
				assert(shot.targetType == PhysicsObjectType::NONE);
				assert(shot.targetId == std::numeric_limits<std::size_t>::max());
			}

			if (timeGun->second > 0)
			{
				newPickups[i][Ability::TIME_GUN] = timeGun->second - 1;
			}
		}
	}

	// Guys which depart normally
	for (std::size_t i(0), size(guyArrivalList.size()); i != size; ++i)
	{
		if (finishedWith[i])
		{
			continue;
		}
		nextGuy.push_back(
			ObjectAndTime<Guy, Frame *>(
				Guy(
					guyArrivalList[i].getIndex() + 1,
					x[i], y[i],
					xspeed[i], yspeed[i],
					walkSpeed[i], jumpHold[i], action[i],
					newWidth[i], newHeight[i],
					newJumpSpeed[i],

					illegalPortal[i],
					-1,
					supported[i],
					supportedSpeed[i],

					newPickups[i],
					facing[i],

					carry[i],
					carryWidth[i],
					carryHeight[i],
					carryDirection[i],

					guyArrivalList[i].getTimeDirection(),
					newTimePaused[i]
					),
				newTimePaused[i] ? frame : nextFrame(frame, guyArrivalList[i].getTimeDirection())
				)
			);
	}
}

template <
	typename RandomAccessPlatformRange>
	void boxInteractionBoundLoop(
		TimeDirection const boxDirection,
		Environment const &env,
		mp::std::vector<int> &x,
		mp::std::vector<int> &y,
		mp::std::vector<int> &xTemp,
		mp::std::vector<int> &yTemp,
		mp::std::vector<char> &squished,
		mp::std::vector<int> const &width,
		mp::std::vector<int> const &height,
		mp::std::vector<BoxType> const &boxType,
		mp::std::vector<Box> const &oldBoxList,
		RandomAccessPlatformRange const &nextPlatform,
		BoxGlitzAdder const &boxGlitzAdder,
		memory_pool<user_allocator_tbb_alloc> &pool)
{
	mp::std::vector<int> xPreBox(oldBoxList.size(), pool);
	mp::std::vector<int> yPreBox(oldBoxList.size(), pool);

	bool thereAreStillThingsToDo(true); // if a box moves thereAreStillThingsToDo
	bool firstTimeThrough(true);

	//unsigned int count(0);
	//if (boost::size(oldBoxList) > 1) {
	//	for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
	//	{
	//		std::cerr << "Box(" << oldBoxList[i].getX() << ", " << oldBoxList[i].getY() << ", " << oldBoxList[i].getXspeed()
	//			<< ", " << oldBoxList[i].getYspeed() << ", " << oldBoxList[i].getWidth() << ", " << oldBoxList[i].getHeight()
	//			<< ", -1, " << oldBoxList[i].getArrivalBasis() << ", FORWARDS),\n";
	//	}
	//}

	while (thereAreStillThingsToDo) {
		//if (boost::size(oldBoxList) > 1) {
		//	std::cerr << "Count " << count << "\n";
		//	++count;
		//}

		mp::std::vector<std::pair<bool, int>> top(oldBoxList.size(), pool);
		mp::std::vector<std::pair<bool, int>> bottom(oldBoxList.size(), pool);
		mp::std::vector<std::pair<bool, int>> left(oldBoxList.size(), pool);
		mp::std::vector<std::pair<bool, int>> right(oldBoxList.size(), pool);

		//TODO: Use scoped allocator?
		mp::std::vector<mp::std::vector<std::size_t>> topLinks(oldBoxList.size(), mp::std::vector<std::size_t>(pool), pool);
		mp::std::vector<mp::std::vector<std::size_t>> bottomLinks(oldBoxList.size(), mp::std::vector<std::size_t>(pool), pool);
		mp::std::vector<mp::std::vector<std::size_t>> rightLinks(oldBoxList.size(), mp::std::vector<std::size_t>(pool), pool);
		mp::std::vector<mp::std::vector<std::size_t>> leftLinks(oldBoxList.size(), mp::std::vector<std::size_t>(pool), pool);

		thereAreStillThingsToDo = false; 

		//*** collide boxes with platforms and walls to discover the hard bounds on the system ***//
		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i) {
			if (!squished[i] && oldBoxList[i].getTimeDirection() == boxDirection) {

				//if (boost::size(oldBoxList) > 1) std::cerr << "Box Start " << i << ": " << x[i] << ", " << xTemp[i] << ", " << y[i] << ", " << yTemp[i] << "\n";
				//** Check inside a wall, velocity independent which is why it is so complex **//
				// intial keep-it-inside-the-level step
				if (x[i] <= 0 || y[i] <= 0 || x[i] + width[i] > env.wall.roomWidth() || y[i] + height[i] > env.wall.roomHeight())
				{
					int xOut = x[i] + width[i] - env.wall.roomWidth();
					int yOut = y[i] + height[i] - env.wall.roomHeight();
					if (x[i] < 0 && (y[i] > 0 || x[i] < y[i]) && (yOut <= 0 || -x[i] > yOut)) {
						y[i] = y[i] - (x[i] - env.wall.segmentSize() / 2)*(y[i] - yTemp[i]) / std::abs(x[i] - xTemp[i]);
						x[i] = env.wall.segmentSize() / 2;
					}
					else if (y[i] < 0 && (x[i] > 0 || y[i] < x[i]) && (xOut <= 0 || -y[i] > xOut)) {
						x[i] = x[i] - (y[i] - env.wall.segmentSize() / 2)*(x[i] - xTemp[i]) / std::abs(y[i] - yTemp[i]);
						y[i] = env.wall.segmentSize() / 2;
					}
					else if (xOut > 0 && (y[i] > 0 || xOut > -y[i]) && (yOut <= 0 || xOut > yOut)) {
						y[i] = y[i] - (xOut + env.wall.segmentSize() / 2 + width[i])*(y[i] - yTemp[i]) / std::abs(x[i] - xTemp[i]);
						x[i] = env.wall.roomWidth() - env.wall.segmentSize() / 2 - width[i];
					}
					else if (yOut > 0 && (x[i] > 0 || yOut > -x[i]) && (xOut <= 0 || yOut > xOut)) {
						x[i] = x[i] - (yOut + env.wall.segmentSize() / 2 + width[i])*(x[i] - xTemp[i]) / std::abs(y[i] - yTemp[i]);
						y[i] = env.wall.roomHeight() - env.wall.segmentSize() / 2 - height[i];
					}
				}

				// TryAgainWithMoreInterpolation is only to be triggered when moving
				// the box by size[i] will ensure that the box still collides with the wall
				// it is attempted to be moved out of.
				if (false) {
				TryAgainWithMoreInterpolation:
					//if (boost::size(oldBoxList) > 1) std::cerr << "Interpolate " << i << ": " << x[i] << ", " << xTemp[i] << ", " << y[i] << ", " << yTemp[i] << "\n";
					if (std::abs(x[i] - xTemp[i]) < std::abs(y[i] - yTemp[i])) {
						int newY;
						if (y[i] < yTemp[i])
						{
							newY = y[i] - y[i] % env.wall.segmentSize() + env.wall.segmentSize();
						}
						else
						{
							newY = y[i] - y[i] % env.wall.segmentSize() - height[i];
						}
						x[i] = x[i] - std::abs(y[i] - newY)*(x[i] - xTemp[i]) / std::abs(y[i] - yTemp[i]);
						y[i] = newY;
					}
					else {
						int newX;
						if (x[i] < xTemp[i])
						{
							newX = x[i] - x[i] % env.wall.segmentSize() + env.wall.segmentSize();
						}
						else
						{
							newX = x[i] - x[i] % env.wall.segmentSize() - width[i];
						}
						y[i] = y[i] - std::abs(x[i] - newX)*(y[i] - yTemp[i]) / std::abs(x[i] - xTemp[i]);
						x[i] = newX;
					}
				}

				top[i] = std::make_pair(false, 0);
				bottom[i] = std::make_pair(false, 0);
				left[i] = std::make_pair(false, 0);
				right[i] = std::make_pair(false, 0);

				// Normal case; box does not have 2 or more sides lined up on the wallmap.
				bool w00, w10, w01, w11;

				if (width[i] <= env.wall.segmentSize() && height[i] <= env.wall.segmentSize()) {
					// purely a speedup for this case
					w00 = env.wall.at(x[i], y[i]);
					w10 = env.wall.at(x[i] + width[i] - 1, y[i]);
					w01 = env.wall.at(x[i], y[i] + height[i] - 1);
					w11 = env.wall.at(x[i] + width[i] - 1, y[i] + height[i] - 1);
				}
				else {
					// Extra collision for box size greater than wall size (would handle other case too)
					w00 = false;
					w10 = false;
					w01 = false;
					w11 = false;

					int xOff = 0;
					while (xOff < width[i] - 1) {
						xOff += env.wall.segmentSize();
						if (xOff > width[i] - 1) {
							xOff = width[i] - 1;
						}
						int yOff = 0;
						while (yOff < height[i] - 1) {
							yOff += env.wall.segmentSize();
							if (yOff > height[i] - 1) {
								yOff = height[i] - 1;
							}
							// the collision test bit
							if (!w00) {
								w00 = env.wall.at(x[i] + width[i] - xOff, y[i] + height[i] - yOff);
							}
							if (!w10) {
								w10 = env.wall.at(x[i] + xOff, y[i] + height[i] - yOff);
							}
							if (!w01) {
								w01 = env.wall.at(x[i] + width[i] - xOff, y[i] + yOff);
							}
							if (!w11) {
								w11 = env.wall.at(x[i] + xOff, y[i] + yOff);
							}
						}
					}
					//std::cerr << "w00 " << w00 << ", w10 " << w10 << ", w01 " << w01 << ", w11 " << w11 << "\n";
				}

				// collide with walls based on corner status
				if (w00) {
					// x.
					// ..
					if (w11) {
						// x.
						// .x
						if (w10) {
							// xx
							// .x
							if (w01) {
								// xx
								// xx
								if (x[i] == xTemp[i] && y[i] == yTemp[i]) {
									// Cannot interpolate a static box.
									boxGlitzAdder.addDeathGlitz(xTemp[i], yTemp[i], width[i], height[i], oldBoxList[i].getTimeDirection());
									squished[i] = true;
								}
								else {
									goto TryAgainWithMoreInterpolation;
								}
							}
							else {
								// xx
								// ox
								x[i] = ((x[i] + width[i] - 1) / env.wall.segmentSize())*env.wall.segmentSize() - width[i];
								right[i] = std::make_pair(true, x[i]);
								y[i] = (y[i] / env.wall.segmentSize() + 1)*env.wall.segmentSize();
								top[i] = std::make_pair(true, y[i]);
							}
						}
						//This triangle check needs improvement for rectangles
						else if (w01 || !env.wall.inTopRightTriangle(x[i], y[i], xTemp[i], yTemp[i])) {
							// xo
							// .x
							x[i] = (x[i] / env.wall.segmentSize() + 1)*env.wall.segmentSize();
							left[i] = std::make_pair(true, x[i]);
							y[i] = ((y[i] + height[i] - 1) / env.wall.segmentSize())*env.wall.segmentSize() - height[i];
							bottom[i] = std::make_pair(true, y[i]);
						}
						else {
							// xo
							// ox
							x[i] = ((x[i] + width[i] - 1) / env.wall.segmentSize())*env.wall.segmentSize() - width[i];
							right[i] = std::make_pair(true, x[i]);
							y[i] = (y[i] / env.wall.segmentSize() + 1)*env.wall.segmentSize();
							top[i] = std::make_pair(true, y[i]);
						}
					}
					else if (w10) {
						// xx
						// .o
						if (w01) {
							// xx
							// xo
							x[i] = (x[i] / env.wall.segmentSize() + 1)*env.wall.segmentSize();
							left[i] = std::make_pair(true, x[i]);
							y[i] = (y[i] / env.wall.segmentSize() + 1)*env.wall.segmentSize();
							top[i] = std::make_pair(true, y[i]);
						}
						else {
							// xx
							// oo
							y[i] = (y[i] / env.wall.segmentSize() + 1)*env.wall.segmentSize();
							top[i] = std::make_pair(true, y[i]);
						}
					}
					else if (w01 || env.wall.inTopRightTriangle(x[i], y[i], xTemp[i], yTemp[i]))
					{
						// xo
						// .o
						x[i] = (x[i] / env.wall.segmentSize() + 1)*env.wall.segmentSize();
						left[i] = std::make_pair(true, x[i]);
					}
					else
					{
						// xo
						// oo
						y[i] = (y[i] / env.wall.segmentSize() + 1)*env.wall.segmentSize();
						top[i] = std::make_pair(true, y[i]);
					}
				}
				else if (w10)
				{
					// ox
					// ..
					if (w01)
					{
						// ox
						// x.
						if (w11 || env.wall.inTopLeftTriangle(x[i] + width[i] - 1, y[i], xTemp[i] + height[i] - 1, yTemp[i])) // this triangle check needs improvement for rectangles
						{
							// ox
							// x.
							x[i] = ((x[i] + width[i] - 1) / env.wall.segmentSize())*env.wall.segmentSize() - width[i];
							right[i] = std::make_pair(true, x[i]);
							y[i] = ((y[i] + height[i] - 1) / env.wall.segmentSize())*env.wall.segmentSize() - height[i];
							bottom[i] = std::make_pair(true, y[i]);
						}
						else
						{
							// ox
							// xo
							x[i] = (x[i] / env.wall.segmentSize() + 1)*env.wall.segmentSize();
							left[i] = std::make_pair(true, x[i]);
							y[i] = (y[i] / env.wall.segmentSize() + 1)*env.wall.segmentSize();
							top[i] = std::make_pair(true, y[i]);
						}
					}
					else if (w11 || env.wall.inTopLeftTriangle(x[i] + width[i] - 1, y[i], xTemp[i] + width[i] - 1, yTemp[i]))
					{
						// ox
						// o.
						x[i] = ((x[i] + width[i] - 1) / env.wall.segmentSize())*env.wall.segmentSize() - width[i];
						right[i] = std::make_pair(true, x[i]);
					}
					else
					{
						// ox
						// oo
						y[i] = (y[i] / env.wall.segmentSize() + 1)*env.wall.segmentSize();
						top[i] = std::make_pair(true, y[i]);
					}
				}
				else if (w01)
				{
					// oo
					// x.
					if (w11 || env.wall.inTopLeftTriangle(x[i], y[i] + height[i] - 1, xTemp[i], yTemp[i] + height[i] - 1))
					{
						// oo
						// x.
						y[i] = ((y[i] + height[i] - 1) / env.wall.segmentSize())*env.wall.segmentSize() - height[i];
						bottom[i] = std::make_pair(true, y[i]);
					}
					else
					{
						// oo
						// xo
						x[i] = (x[i] / env.wall.segmentSize() + 1)*env.wall.segmentSize();
						left[i] = std::make_pair(true, x[i]);
					}
				}
				else if (w11)
				{
					// oo
					// ox
					if (env.wall.inTopRightTriangle(x[i] + width[i] - 1, y[i] + height[i] - 1, xTemp[i] + width[i] - 1, yTemp[i] + height[i] - 1))
					{
						y[i] = ((y[i] + height[i] - 1) / env.wall.segmentSize())*env.wall.segmentSize() - height[i];
						bottom[i] = std::make_pair(true, y[i]);
					}
					else
					{
						x[i] = ((x[i] + width[i] - 1) / env.wall.segmentSize())*env.wall.segmentSize() - width[i];
						right[i] = std::make_pair(true, x[i]);
					}
				}

				// Stop sideways movement if the floor was hit (ceiling for balloons)
				if (bottom[i].first && boxType[i] != BoxType::BALLOON) {
					x[i] = xTemp[i];
				}
				if (top[i].first && boxType[i] == BoxType::BALLOON) {
					x[i] = xTemp[i];
				}

				// Check inside a platform
				for (Collision const &platform : nextPlatform)
				{
					if (platform.getFunctional()) {
						int pX(platform.getX());
						int pY(platform.getY());
						TimeDirection pDirection(platform.getTimeDirection());
						int pWidth(platform.getWidth());
						int pHeight(platform.getHeight());

						if (pDirection != boxDirection)
						{
							pX -= platform.getXspeed() + platform.getPrevXspeed();
							pY -= platform.getYspeed() + platform.getPrevYspeed();
						}
						//std::cerr << "Platform y - height: " <<  (pY - height[i]) << "\n";

						if (IntersectingRectanglesInclusive(x[i], y[i], width[i], height[i], pX, pY, pWidth, pHeight))
						{
							if (platform.getCollisionType() == CollisionType::PLATFORM)
							{
								if (IsRectangleRelationVertical(xTemp[i], yTemp[i], width[i], height[i], pX, pY, pWidth, pHeight, false))
								{
									if (yTemp[i] + height[i] / 2 < pY + pHeight / 2) // box above platform
									{
										y[i] = pY - height[i];
										bottom[i] = std::make_pair(true, y[i]);
										//std::cerr << "Box platform hit " << i << ": " << y[i] << "\n";
										if (firstTimeThrough && boxType[i] != BoxType::BALLOON)
										{
											x[i] = xTemp[i] + static_cast<int>(pDirection * oldBoxList[i].getTimeDirection()) * platform.getXspeed();
										}
									}
									else
									{
										y[i] = pY + pHeight;
										top[i] = std::make_pair(true, y[i]);
										if (firstTimeThrough && boxType[i] == BoxType::BALLOON)
										{
											x[i] = xTemp[i] + static_cast<int>(pDirection * oldBoxList[i].getTimeDirection()) * platform.getXspeed();
										}
									}
								}
								else // left or right
								{
									if (xTemp[i] + width[i] / 2 < pX + pWidth / 2) // box left of platform
									{
										x[i] = pX - width[i];
										right[i] = std::make_pair(true, x[i]);
									}
									else
									{
										x[i] = pX + pWidth;
										left[i] = std::make_pair(true, x[i]);
									}
								}
							}
						}
					}
				}

				// Collide with reverse boxes
				for (std::size_t j(0), jsize(boost::size(oldBoxList)); j < jsize; ++j)
				{
					if (boxCollidable(boxType[i]) && boxCollidable(oldBoxList[j].getBoxType())
						&& oldBoxList[j].getTimeDirection() != boxDirection)
					{
						int bX = oldBoxList[j].getX() - oldBoxList[j].getXspeed();
						int bY = oldBoxList[j].getY() - oldBoxList[j].getYspeed();
						int bWidth = oldBoxList[j].getWidth();
						int bHeight = oldBoxList[j].getHeight();
						if (IntersectingRectanglesInclusive(x[i], y[i], width[i], height[i], bX, bY, bWidth, bHeight))
						{
							if (IsRectangleRelationVertical(x[i], y[i], width[i], height[i], bX, bY, bWidth, bHeight, false))
							{
								if (yTemp[i] + height[i] / 2 < bY + bHeight / 2) // on top of reverse box
								{
									y[i] = bY - height[i];
									bottom[i] = std::make_pair(true, y[i]);
									if (firstTimeThrough)
									{
										x[i] = xTemp[i] - oldBoxList[j].getXspeed();
									}
								}
								else
								{
									y[i] = bY + bHeight;
									top[i] = std::make_pair(true, y[i]);
								}
							}
							else // left or right
							{
								if (xTemp[i] + width[i] / 2 < bX + bWidth / 2) // box left of reverse box
								{
									x[i] = bX - width[i];
									right[i] = std::make_pair(true, x[i]);
								}
								else
								{
									x[i] = bX + bWidth;
									left[i] = std::make_pair(true, x[i]);
								}
							}
						}
					}
				}
			}
		}

		// Store position before box collision
		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i] && oldBoxList[i].getTimeDirection() == boxDirection)
			{
				//std::cerr << "Wall " << i << ": " << x[i] << ", " << y[i] << " " << (right[i].first ? "> " : "  ") << (top[i].first ? "^ " : "  ") << (left[i].first ? "< " : "  ") << (bottom[i].first ? "v\n" : " \n");
				xPreBox[i] = x[i];
				yPreBox[i] = y[i];
				//if (boost::size(oldBoxList) > 1) std::cerr << "Box Store " << i << ": " << x[i] << ", " << xTemp[i] << ", " << y[i] << ", " << yTemp[i] << "\n";
			}
		}

		// Now make the map of vertical collisions
		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i] && boxCollidable(boxType[i]) && oldBoxList[i].getTimeDirection() == boxDirection)
			{
				for (std::size_t j(0); j < i; ++j)
				{
					if (j != i && !squished[j] && boxCollidable(boxType[j])
							&& oldBoxList[j].getTimeDirection() == boxDirection
							&& IntersectingRectanglesInclusive(x[i], y[i], width[i], height[i], x[j], y[j], width[j], height[j])) {
						if (IsRectangleRelationVertical(x[i], y[i], width[i], height[i], x[j], y[j], width[j], height[j], false))
						{
							if (y[i] + height[i]/2 < y[j] + height[j]/2) // i above j
							{
								bottomLinks[i].push_back(j);
								topLinks[j].push_back(i);
								//if (boost::size(oldBoxList) > 1) std::cerr << "Vert link " << i << " <-> " << j << "\n";
							}
							else // i below j
							{
								topLinks[i].push_back(j);
								bottomLinks[j].push_back(i);
								//if (boost::size(oldBoxList) > 1) std::cerr << "Vert link " << j << " <-> " << i << "\n";
							}
						}
					}
				}
			}
		}
		//for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		//{
		//    if (boost::size(oldBoxList) > 1) std::cerr << "Box post Vertlink " << i << ": " << x[i] << ", " << xTemp[i] << ", " << y[i] << ", " << yTemp[i] << "\n";
		//}

		// propagate through vertical collision links to reposition and explode
		mp::std::vector<char> toBeSquished(oldBoxList.size(), pool);

		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i] && boxCollidable(boxType[i]) && oldBoxList[i].getTimeDirection() == boxDirection)
			{
				if (bottom[i].first) // Push boxes up
				{
					//explodeBoxes(y, size, topLinks, toBeSquished, top, i, bottom[i].second, -1);
					explodeBoxesUpwards(x, xTemp, y, height, topLinks, firstTimeThrough, toBeSquished, top, i, bottom[i].second);
				}
				if (top[i].first) // Push boxes down
				{
					//std::cerr << "explodeBoxes" << "\n";
					explodeBoxes(y, height, bottomLinks, toBeSquished, bottom, i, top[i].second, 1);
				}
			}
		}

		//for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		//{
		//    if (boost::size(oldBoxList) > 1) std::cerr << "Box postExplode vert " << i << ": " << x[i] << ", " << xTemp[i] << ", " << y[i] << ", " << yTemp[i] << "\n";
		//}
		
		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		{
			if (toBeSquished[i] && oldBoxList[i].getTimeDirection() == boxDirection)
			{
				boxGlitzAdder.addDeathGlitz(xTemp[i], yTemp[i], width[i], height[i], oldBoxList[i].getTimeDirection());
				squished[i] = true;
			}
		}

		// Now make the map of horizontal collisions
		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i] && boxCollidable(boxType[i]) && oldBoxList[i].getTimeDirection() == boxDirection)
			{
				for (unsigned j = 0; j < i; ++j)
				{
					if (j != i && !squished[j] && boxCollidable(boxType[j])
						&& oldBoxList[j].getTimeDirection() == boxDirection
						&& IntersectingRectanglesInclusive(x[i], y[i], width[i], height[i], x[j], y[j], width[j], height[j]))
					{
						if (!IsRectangleRelationVertical(x[i], y[i], width[i], height[i], x[j], y[j], width[j], height[j], false))
						{
							if (x[i] + width[i]/2 < x[j] + width[j]/2) // i left of j
							{
								rightLinks[i].push_back(j);
								leftLinks[j].push_back(i);
								//if (boost::size(oldBoxList) > 1) std::cerr << "Hori link " << i << " <-> " << j << "\n";
							}
							else // i right of j
							{
								leftLinks[i].push_back(j);
								rightLinks[j].push_back(i);
								//if (boost::size(oldBoxList) > 1) std::cerr << "Hori link " << j << " <-> " << i << "\n";
							}
						}
					}
				}
			}
		}

		// propagate through horizontal collision links to reposition and explode
		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		{
			if (oldBoxList[i].getTimeDirection() == boxDirection)
			{
				toBeSquished[i] = false;
			}
		}

		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i] && boxCollidable(boxType[i]) && oldBoxList[i].getTimeDirection() == boxDirection)
			{
				if (right[i].first)
				{
					explodeBoxes(x, width, leftLinks, toBeSquished, left, i, right[i].second, -1);
				}
				if (left[i].first)
				{
					explodeBoxes(x, width, rightLinks, toBeSquished, right, i, left[i].second, 1);
				}
			}
		}

		//for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		//{
		//    if (boost::size(oldBoxList) > 1) std::cerr << "Box postExplode hor " << i << ": " << x[i] << ", " << xTemp[i] << ", " << y[i] << ", " << yTemp[i] << "\n";
		//}

		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		{
			if (toBeSquished[i] && oldBoxList[i].getTimeDirection() == boxDirection)
			{
				boxGlitzAdder.addDeathGlitz(xTemp[i], yTemp[i], width[i], height[i], oldBoxList[i].getTimeDirection());
				squished[i] = true;
			}
		}

		// Collide boxes vertically which are still overlapping. These will be the unbounded ones
		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i] && boxCollidable(boxType[i]) && oldBoxList[i].getTimeDirection() == boxDirection)
			{
				mp::std::vector<std::size_t> pass(pool);
				recursiveBoxCollision(y, x, height, width, squished, boxType, pass, i, false, boxDirection, oldBoxList);
			}
		}

		//for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		//{
		//    if (boost::size(oldBoxList) > 1) std::cerr << "Box postVert " << i << ": " << x[i] << ", " << xTemp[i] << ", " << y[i] << ", " << yTemp[i] << "\n";
		//}

		// Collide them horizontally
		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i] && boxCollidable(boxType[i]) && oldBoxList[i].getTimeDirection() == boxDirection)
			{
				mp::std::vector<std::size_t> pass(pool);
				recursiveBoxCollision(x, y, width, height, squished, boxType, pass, i, true, boxDirection, oldBoxList);
			}
		}

		//for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		//{
		//    if (boost::size(oldBoxList) > 1) std::cerr << "Box postHor " << i << ": " << x[i] << ", " << xTemp[i] << ", " << y[i] << ", " << yTemp[i] << "\n";
		//}

		// If anything moved then rerun box collision until nothing moves.
		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i] && oldBoxList[i].getTimeDirection() == boxDirection)
			{
				if (x[i] != xTemp[i] || y[i] != yTemp[i])
				{
					thereAreStillThingsToDo = true;
					xTemp[i] = xPreBox[i];
					yTemp[i] = yPreBox[i];
				}
			}
		}
		firstTimeThrough = false;
	}
}

template <
	typename RandomAccessBoxRange,
	typename RandomAccessPortalRange,
	typename RandomAccessPlatformRange,
	typename RandomAccessArrivalLocationRange,
	typename RandomAccessMutatorRange,
	typename FrameT>
	void boxCollisionAlgorithm(
		Environment const &env,
		RandomAccessBoxRange const &boxArrivalList,
		mp::std::vector<Box> const &additionalBox,
		mp::std::vector<ObjectAndTime<Box, FrameT>> &nextBox,
		mp::std::vector<char> &nextBoxNormalDeparture,
		RandomAccessPlatformRange const &nextPlatform,
		RandomAccessPortalRange const &nextPortal,
		RandomAccessArrivalLocationRange const &arrivalLocations,
		RandomAccessMutatorRange const &mutators,
		mt::std::vector<ExplosionEffect> &explosions,
		TriggerFrameState &triggerFrameState,
		BoxGlitzAdder const &boxGlitzAdder,
		FrameT const &frame,
		memory_pool<user_allocator_tbb_alloc> &pool)
{
	mp::std::vector<Box> oldBoxList(pool);

	boost::push_back(oldBoxList, boxArrivalList);
	boost::push_back(oldBoxList, additionalBox);

	boost::sort(oldBoxList);


	//std::cerr << "*** New Step ***\n";
	/*
	for (Collision const &platform: nextPlatform)
	{
		int pX(platform.getX());
		int pY(platform.getY());
		int pWidth(platform.getWidth());
		int pHeight(platform.getHeight());
		std::cerr << "Platform " << pX << ", " << pY << ", " << pWidth << ", " << pHeight << "\n";
	}
	*/

	mp::std::vector<int> x(oldBoxList.size(), pool);
	mp::std::vector<int> y(oldBoxList.size(), pool);
	mp::std::vector<int> xTemp(oldBoxList.size(), pool);
	mp::std::vector<int> yTemp(oldBoxList.size(), pool);
	mp::std::vector<int> xPreBox(oldBoxList.size(), pool);
	mp::std::vector<int> yPreBox(oldBoxList.size(), pool);
	mp::std::vector<int> width(oldBoxList.size(), pool);
	mp::std::vector<int> height(oldBoxList.size(), pool);
	mp::std::vector<BoxType> boxType(oldBoxList.size(), pool);
	mp::std::vector<int> state(oldBoxList.size(), pool);
	mp::std::vector<char> squished(oldBoxList.size(), pool);

	// Check with triggers if the box should arrive at all
	for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i) {
		if (!triggerFrameState.shouldArrive(oldBoxList[i])) {
			// Triggers should add Glitz for this case as required.
			squished[i] = true;
		}
	}

	// Inititalise box location with arrival basis
	for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i) {
		if (oldBoxList[i].getArrivalBasis() == -1) {
			xTemp[i] = oldBoxList[i].getX();
			yTemp[i] = oldBoxList[i].getY();
			x[i] = xTemp[i] + oldBoxList[i].getXspeed();
			y[i] = yTemp[i] + oldBoxList[i].getYspeed();
		}
		else
		{
			ArrivalLocation const& relativePortal(arrivalLocations[oldBoxList[i].getArrivalBasis()]);
			int relx = relativePortal.getX() + oldBoxList[i].getX();
			int rely = relativePortal.getY() + oldBoxList[i].getY();
			if (relativePortal.getTimeDirection() * oldBoxList[i].getTimeDirection() == TimeDirection::FORWARDS)
			{
				xTemp[i] = relx - relativePortal.getXspeed();
				yTemp[i] = rely - relativePortal.getYspeed();
				x[i] = relx + oldBoxList[i].getXspeed();
				y[i] = rely + oldBoxList[i].getYspeed();
			}
			else
			{
				xTemp[i] = relx + relativePortal.getXspeed();
				yTemp[i] = rely + relativePortal.getYspeed();
				x[i] = relx + oldBoxList[i].getXspeed();
				y[i] = rely + oldBoxList[i].getYspeed();
			}
		}
		width[i] = oldBoxList[i].getWidth();
		height[i] = oldBoxList[i].getHeight();
		boxType[i] = oldBoxList[i].getBoxType();
		state[i] = oldBoxList[i].getState();

		if (oldBoxList[i].getBoxType() == BoxType::BALLOON) {
			y[i] -= hg::UP_GRAVITY / 2;
		}
		else {
			if (oldBoxList[i].getYspeed() > 0) {
				y[i] += hg::UP_GRAVITY;
			}
			else {
				y[i] += hg::DOWN_GRAVITY;
			}
		}
		y[i] += -oldBoxList[i].getYspeed() * oldBoxList[i].getYspeed() * oldBoxList[i].getYspeed() / getBoxVertAirResistence(oldBoxList[i].getBoxType());
	}

	// Destroy boxes that are overlapping with platforms and walls
	for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i) {
		if (!squished[i]) {
			if (env.wall.at(xTemp[i], yTemp[i]) && env.wall.at(xTemp[i] + width[i] - 1, yTemp[i])
				&& env.wall.at(xTemp[i], yTemp[i] + height[i] - 1) && env.wall.at(xTemp[i] + width[i], yTemp[i] + height[i] - 1))
			{
				boxGlitzAdder.addDeathGlitz(xTemp[i], yTemp[i], width[i], height[i], oldBoxList[i].getTimeDirection());
				squished[i] = true;
				continue;
			}

			for (Collision const &platform : nextPlatform) {
				if (platform.getFunctional()) {
					int pX(platform.getX());
					int pY(platform.getY());
					int pWidth(platform.getWidth());
					int pHeight(platform.getHeight());
					TimeDirection pDirection(platform.getTimeDirection());
					if (pDirection * oldBoxList[i].getTimeDirection() == TimeDirection::FORWARDS)
					{
						pX -= platform.getXspeed();
						pY -= platform.getYspeed();
						if (IntersectingRectanglesExclusive(xTemp[i], yTemp[i], width[i], height[i], pX, pY, pWidth, pHeight))
						{
							if (platform.getCollisionType() == CollisionType::PLATFORM)
							{
								boxGlitzAdder.addDeathGlitz(xTemp[i], yTemp[i], width[i], height[i], oldBoxList[i].getTimeDirection());
								squished[i] = true;
								continue;
							}
						}
					}
					else
					{
						pX -= platform.getXspeed();
						pY -= platform.getYspeed();
						if (IntersectingRectanglesExclusive(xTemp[i], yTemp[i], width[i], height[i], pX, pY, pWidth, pHeight))
						{
							if (platform.getCollisionType() == CollisionType::PLATFORM)
							{
								boxGlitzAdder.addDeathGlitz(xTemp[i], yTemp[i], width[i], height[i], oldBoxList[i].getTimeDirection());
								squished[i] = true;
								continue;
							}
						}
					}
				}
			}
		}
	}

	// Destroy boxes that are overlapping, deals with chronofrag (maybe too strictly?)
	{
		mp::std::vector<char> toBeSquished(oldBoxList.size(), pool);
	
		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i) {
			if (boxCollidable(boxType[i]) && !squished[i]) {
				for (std::size_t j(0); j < i; ++j) {
					if (boxCollidable(boxType[j]) && !squished[j]) {
						if (IntersectingRectanglesExclusive(
							xTemp[i], yTemp[i], width[i], height[i],
							xTemp[j], yTemp[j], width[j], height[j]))
						{
							toBeSquished[i] = true;
							toBeSquished[j] = true;
						}
					}
				}
			}
		}
	
		for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i) {
			if (toBeSquished[i]) {
				boxGlitzAdder.addDeathGlitz(xTemp[i], yTemp[i], width[i], height[i], oldBoxList[i].getTimeDirection());
				squished[i] = true;
			}
		}
	}

	boxInteractionBoundLoop(TimeDirection::FORWARDS, env, x, y, xTemp, yTemp, squished, width, height, boxType, oldBoxList, nextPlatform, boxGlitzAdder, pool);
	boxInteractionBoundLoop(TimeDirection::REVERSE, env, x, y, xTemp, yTemp, squished, width, height, boxType, oldBoxList, nextPlatform, boxGlitzAdder, pool);

	// Update box state
	for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i) {
		if (boxType[i] == BoxType::BOMB) {
			if (state[i] > 0) {
				state[i] -= 1;
				if (state[i] == 0) {
					explodeBomb(i, x, y, width, height, boxType, squished, oldBoxList, explosions, boxGlitzAdder, pool);
				}
			}
		}
	}

	// Send boxes
	for (std::size_t i(0), isize(boost::size(oldBoxList)); i < isize; ++i)
	{
		if (!squished[i])
		{
			if (oldBoxList[i].getArrivalBasis() == -1)
			{
				makeBoxAndTimeWithPortalsAndMutators(
					nextBox,
					nextBoxNormalDeparture,
					nextPortal,
					mutators,
					x[i],
					y[i],
					x[i] - oldBoxList[i].getX(),
					y[i] - oldBoxList[i].getY(),
					width[i],
					height[i],
					boxType[i],
					state[i],
					oldBoxList[i].getIllegalPortal(),
					oldBoxList[i].getTimeDirection(),
					triggerFrameState,
					frame,
					pool);
			}
			else
			{
				ArrivalLocation const &relativePortal(arrivalLocations[oldBoxList[i].getArrivalBasis()]);
				int relx = relativePortal.getX() + oldBoxList[i].getX();
				int rely = relativePortal.getY() + oldBoxList[i].getY();
				if (relativePortal.getTimeDirection() * oldBoxList[i].getTimeDirection() == TimeDirection::FORWARDS)
				{
					relx = relx - relativePortal.getXspeed();
					rely = rely - relativePortal.getYspeed();
				}
				else
				{
					relx = relx + relativePortal.getXspeed();
					rely = rely + relativePortal.getYspeed();
				}

				makeBoxAndTimeWithPortalsAndMutators(
					nextBox,
					nextBoxNormalDeparture,
					nextPortal,
					mutators,
					x[i],
					y[i],
					x[i] - relx,
					y[i] - rely,
					width[i],
					height[i],
					boxType[i],
					state[i],
					oldBoxList[i].getIllegalPortal(),
					oldBoxList[i].getTimeDirection(),
					triggerFrameState,
					frame,
					pool);
			}
		}
	}
}

}//namespace hg
