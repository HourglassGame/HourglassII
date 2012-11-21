namespace hg {
template<typename RandomAccessObjectAndTypeRange>
void buildDeparturesForComplexEntities(
    RandomAccessObjectAndTypeRange const& next,
    PhysicsEngine::FrameDepartureT& newDepartures)
{
	foreach (typename boost::range_reference<RandomAccessObjectAndTypeRange const>::type thingAndTime, next)
    {
        newDepartures[thingAndTime.frame].add(thingAndTime.object);
    }
}

template<typename RandomAccessGuyRange>
bool currentPlayerInArrivals(RandomAccessGuyRange const& guyArrivals, std::size_t playerInputSize)
{
    //The arrivals are sorted
    //The current player arrival can be either the last arrival, or the second last arrival.
    //If it is the second last arrival, then the last arrival must be the next player arrival.
    if (boost::distance(guyArrivals) == 0) {
        return false;
    }
    else {
        std::size_t lastGuyIndex((boost::end(guyArrivals) - 1)->getIndex());
        if (lastGuyIndex < playerInputSize - 1) return false;
        if (lastGuyIndex == playerInputSize - 1) return true;
        if (boost::distance(guyArrivals) == 1) return false;
        return (boost::end(guyArrivals) - 2)->getIndex() == playerInputSize;
    }
}


template <
    typename RandomAccessPortalRange,
    typename RandomAccessMutatorRange,
    typename FrameT>
void makeBoxAndTimeWithPortalsAndMutators(
    typename mt::std::vector<ObjectAndTime<Box, FrameT> >::type& nextBox,
    mt::std::vector<char>::type& nextBoxNormalDeparture,
    const RandomAccessPortalRange& portals,
    const RandomAccessMutatorRange& mutators,
    int x,
    int y,
    int xspeed,
    int yspeed,
    int size,
    int oldIllegalPortal,
    TimeDirection const oldTimeDirection,
    TriggerFrameState& triggerFrameState,
    BoxGlitzAdder const& boxGlitzAdder,
    FrameT frame)
{
    TimeDirection timeDirection(oldTimeDirection);
	int arrivalBasis = -1;
	int illegalPortal = -1;
	FrameT nextTime(nextFrame(frame, timeDirection));


	// Mutator check
	mt::std::vector<int>::type mutatorCollisions;

	for (unsigned i = 0; i < mutators.size(); ++i)
	{
		if (IntersectingRectanglesInclusiveCollisionOverlap(x, y, size, size,
					mutators[i].getX(), mutators[i].getY(),
					mutators[i].getWidth(), mutators[i].getHeight(),
					mutators[i].getCollisionOverlap()))
		{
			mutatorCollisions.push_back(i);
		}
	}

	// send vector of collisions that occured (if any)
	if (mutatorCollisions.size() != 0)
	{

		boost::optional<Box> newBox = triggerFrameState.mutateObject(
			mutatorCollisions,
			Box(
				x,
				y,
				xspeed,
				yspeed,
				size,
				oldIllegalPortal,
				-1,
				timeDirection));
		if (!newBox)
		{
			return; // box was destroyed, do not add
		}
		x = newBox->getX();
		y = newBox->getY();
		xspeed = newBox->getXspeed();
		yspeed = newBox->getYspeed();
		size = newBox->getSize();
		illegalPortal = newBox->getIllegalPortal();
		//arrivalLocation = newBox->getArrivalLocation();
		timeDirection = newBox->getTimeDirection();
	}
    bool normalDeparture = true;
    if (timeDirection == oldTimeDirection && arrivalBasis == -1 /*&& nextTime == oldNextTime*/) {
        boxGlitzAdder.addGlitzForBox(vector2<int>(x, y), vector2<int>(xspeed, yspeed), size, timeDirection);
    }
    else {
        normalDeparture = false;
    }


	// fall through portals
    for (unsigned i = 0; i < portals.size(); ++i)
    {
        if (IntersectingRectanglesInclusiveCollisionOverlap(x, y, size, size,
        		portals[i].getX(), portals[i].getY(),
        		portals[i].getWidth(), portals[i].getHeight(),
        		portals[i].getCollisionOverlap())
        		&& portals[i].getFallable())
        {
            if (oldIllegalPortal != -1 && portals[i].getIndex() == oldIllegalPortal)
            {
                illegalPortal = i;
            }
            else if (triggerFrameState.shouldPort(i, Box(x,y,xspeed,yspeed,size,oldIllegalPortal,-1,timeDirection), false))
            {
                FrameT portalTime(
                		portals[i].getRelativeTime() ?
                    getArbitraryFrame(getUniverse(frame), getFrameNumber(frame) + portals[i].getTimeDestination()):
                    getArbitraryFrame(getUniverse(frame), portals[i].getTimeDestination()));
                nextTime = !isNullFrame(portalTime) ? nextFrame(portalTime, timeDirection) : FrameT();
                illegalPortal = portals[i].getIllegalDestination();
                arrivalBasis = portals[i].getDestinationIndex();
                x = x - portals[i].getX() + portals[i].getXdestination();
                y = y - portals[i].getY() + portals[i].getYdestination();
                xspeed = xspeed - portals[i].getXspeed();
                yspeed = yspeed - portals[i].getXspeed();
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
				size,
				illegalPortal,
				arrivalBasis,
				timeDirection),
			nextTime));
}



template<typename RandomAccessGuyRange>
void guyStep(
    Environment const& env,
    RandomAccessGuyRange const& guyArrivalList,
    Frame* frame,
    std::vector<InputList> const& playerInput,
    mt::std::vector<ObjectAndTime<Guy, Frame*> >::type& nextGuy,
    mt::std::vector<ObjectAndTime<Box, Frame*> >::type& nextBox,
    mt::std::vector<char>::type& nextBoxNormalDeparture,
    mt::std::vector<Collision>::type const& nextPlatform,
    mt::std::vector<PortalArea>::type const& nextPortal,
    mt::std::vector<ArrivalLocation>::type const& arrivalLocations,
    mt::std::vector<MutatorArea>::type const& mutators,
    TriggerFrameState& triggerFrameState,
    GuyGlitzAdder const& guyGlitzAdder,
    BoxGlitzAdder const& boxGlitzAdder,
    bool& nextPlayerFrame,
    bool& winFrame)
{
    mt::std::vector<int>::type x;
    mt::std::vector<int>::type y;
    mt::std::vector<int>::type xspeed;
    mt::std::vector<int>::type yspeed;
    mt::std::vector<char>::type supported;
    mt::std::vector<int>::type supportedSpeed;
    mt::std::vector<char>::type finishedWith;
    mt::std::vector<FacingDirection::FacingDirection>::type facing;

    x.reserve(boost::distance(guyArrivalList));
    y.reserve(boost::distance(guyArrivalList));
    xspeed.reserve(boost::distance(guyArrivalList));
    yspeed.reserve(boost::distance(guyArrivalList));
    supported.reserve(boost::distance(guyArrivalList));
    supportedSpeed.reserve(boost::distance(guyArrivalList));
    finishedWith.reserve(boost::distance(guyArrivalList));
    facing.reserve(boost::distance(guyArrivalList));

    // position, velocity, collisions
    // check collisions in Y direction then do the same in X direction
    for (std::size_t i(0), isize(boost::distance(guyArrivalList)); i < isize; ++i)
    {
    	// initialise positions with arrivalBasis
        if (guyArrivalList[i].getArrivalBasis() == -1)
        {
            x.push_back(guyArrivalList[i].getX());
            y.push_back(guyArrivalList[i].getY());
            xspeed.push_back(0);
			yspeed.push_back(guyArrivalList[i].getYspeed() + env.gravity);
        }
        else
        {
            ArrivalLocation const& relativePortal(arrivalLocations[guyArrivalList[i].getArrivalBasis()]);
            x.push_back(relativePortal.getX() + guyArrivalList[i].getX());
            y.push_back(relativePortal.getY() + guyArrivalList[i].getY());
            xspeed.push_back(0);
			yspeed.push_back(guyArrivalList[i].getYspeed() + relativePortal.getYspeed() + env.gravity);
        }
        supported.push_back(false);
        supportedSpeed.push_back(0);
        finishedWith.push_back(false);
        facing.push_back(guyArrivalList[i].getFacing());

        // Check with triggers if guy should affect frame
		if (not triggerFrameState.shouldArrive(guyArrivalList[i]))
		{
			finishedWith[i] = true;
			continue;
		}

		// Collision algo
        if (guyArrivalList[i].getIndex() < playerInput.size())
        {
            std::size_t relativeIndex(guyArrivalList[i].getIndex());
            InputList const& input(playerInput[relativeIndex]);

            int const width(guyArrivalList[i].getWidth());
            int const height(guyArrivalList[i].getHeight());
            int const jumpSpeed(guyArrivalList[i].getJumpSpeed());

            // chonofrag with platforms
			foreach (Collision const& platform, nextPlatform)
			{
				int pX(platform.getX());
				int pY(platform.getY());
				TimeDirection pDirection(platform.getTimeDirection());
				if (pDirection * guyArrivalList[i].getTimeDirection() == hg::FORWARDS)
				{
					pX -= platform.getXspeed();
					pY -= platform.getYspeed();
				}
				int pWidth(platform.getWidth());
				int pHeight(platform.getHeight());

				if (IntersectingRectanglesExclusive(x[i], y[i], width, height, pX, pY, pWidth, pHeight))
				{
					finishedWith[i] = true;
					continue;
				}
			}

			// chonofrag with walls
			if (wallAtExclusive(env, x[i], y[i], width, height))
			{
				finishedWith[i] = true;
				continue;
			}

            bool bottom = false;
            bool top = false;
            bool left = false;
            bool right = false;

            std::size_t boxThatIamStandingOn(std::numeric_limits<std::size_t>::max());

            // jump
            if (guyArrivalList[i].getSupported() && input.getUp())
            {
                yspeed[i] = guyArrivalList[i].getSupportedSpeed() + jumpSpeed;
            }

            // Y direction collisions
            int newY(y[i] + yspeed[i]);

            // box collision (only occurs in Y direction)
            for (std::size_t j(0), jsize(nextBox.size()); j < jsize; ++j)
			{
            	if (nextBoxNormalDeparture[j])
            	{
            		int boxX(nextBox[j].object.getX());
					int boxY(nextBox[j].object.getY());
					int boxXspeed(nextBox[j].object.getXspeed());
					int boxYspeed(nextBox[j].object.getYspeed());
					int boxSize(nextBox[j].object.getSize());
					TimeDirection boxDirection(nextBox[j].object.getTimeDirection());
					if (x[i] < boxX+boxSize && x[i]+width > boxX)
					{
						if (boxDirection * guyArrivalList[i].getTimeDirection() == hg::REVERSE)
						{
							if (newY + height >= boxY-boxYspeed && newY + height-yspeed[i] <= boxY)
							{
								boxThatIamStandingOn = j;
								newY = boxY-height-boxYspeed;
								xspeed[i] = -boxXspeed;
								supported[i] = true;
								bottom = true;
								supportedSpeed[i] = -boxYspeed;
							}

						}
						else
						{
							if (newY+height >= boxY && newY-yspeed[i]+height <= boxY-boxYspeed)
							{
								boxThatIamStandingOn = j;
								newY = boxY-height;
								xspeed[i] = boxXspeed;
								supported[i] = true;
								bottom = true;
								supportedSpeed[i] = boxYspeed;
							}
						}
					}
            	}
			}

            // check platform collision in Y direction
            foreach (Collision const& platform, nextPlatform)
            {
                int pX(platform.getX());
                int pY(platform.getY());
                TimeDirection pDirection(platform.getTimeDirection());
                if (pDirection * guyArrivalList[i].getTimeDirection() == hg::REVERSE)
                {
                    pX -= platform.getXspeed();
                    pY -= platform.getYspeed();
                }
                int pWidth(platform.getWidth());
                int pHeight(platform.getHeight());

                if (IntersectingRectanglesExclusive(
                        x[i], newY, width, height,
                		pX - pDirection * guyArrivalList[i].getTimeDirection() * platform.getXspeed(), pY, pWidth, pHeight))
                {
                    if (newY+height/2 < pY+pHeight/2)
                    {
                        newY = pY-height;
                        xspeed[i] = pDirection * guyArrivalList[i].getTimeDirection() * platform.getXspeed();
                        supported[i] = true;
                        bottom = true;
                        supportedSpeed[i] = pDirection * guyArrivalList[i].getTimeDirection() * platform.getYspeed();
                    }
                    else
                    {
                        newY = pY + pHeight;
                        top = true;
                    }
                }
            }

            //check wall collision in Y direction
			if (yspeed[i] > 0) // down
			{
				if (env.wall.at(x[i], newY+height) || (x[i] - (x[i]/env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize()-width && env.wall.at(x[i]+width, newY+height)))
				{
					newY = ((newY+height)/env.wall.segmentSize())*env.wall.segmentSize() - height;
					supported[i] = true;
					bottom = true;
					supportedSpeed[i] = 0;
				}
			}
			else if (yspeed[i] < 0) // up
			{
				if  (env.wall.at(x[i], newY) || (x[i] - (x[i]/env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize()-width && env.wall.at(x[i]+width, newY)))
				{
					newY = (newY/env.wall.segmentSize() + 1)*env.wall.segmentSize();
					top = true;
				}
			}

            if (bottom && top)
            {
            	finishedWith[i] = true;
            	continue;
            }

            // X direction stuff
            int newX(x[i] + xspeed[i]);

            //check wall collision in X direction
            if (input.getLeft())
            {
            	facing[i] = FacingDirection::LEFT;
                newX += -250;
            }
            else if (input.getRight())
            {
            	facing[i] = FacingDirection::RIGHT;
                newX += 250;
            }

            // platform collision
            foreach (Collision const& platform, nextPlatform)
            {
                int pX(platform.getX());
                int pY(platform.getY());
                TimeDirection pDirection(platform.getTimeDirection());
                if (pDirection*guyArrivalList[i].getTimeDirection() == hg::REVERSE)
                {
                    pX -= platform.getXspeed();
                    pY -= platform.getYspeed();
                }
                int pWidth = platform.getWidth();
                int pHeight = platform.getHeight();

                if (IntersectingRectanglesExclusive(newX, newY, width, height, pX, pY, pWidth, pHeight))
                {
                    if (newX+width/2 < pX+pWidth/2)
                    {
                        newX = pX-width;
                        right = true;
                    }
                    else
                    {
                        newX = pX+pWidth;
                        left = true;
                    }
                }
            }

            if (newX-x[i] > 0) // right
			{
				if (env.wall.at(newX+width, newY) || (newY - (newY/env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize()-height && env.wall.at(newX+width, newY+height)))
				{
					newX = (newX+width)/env.wall.segmentSize()*env.wall.segmentSize() - width;
					right = true;
				}
			}
			else if (newX-x[i] < 0) // left
			{
				if (env.wall.at(newX, newY) || (newY - (newY/env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize()-height && env.wall.at(newX, newY+height)))
				{
					newX = (newX/env.wall.segmentSize() + 1)*env.wall.segmentSize();
					left = true;
				}
			}

            if (left && right)
			{
				finishedWith[i] = true;
				continue;
			}

			// Apply Change
            xspeed[i] = newX-x[i];
            yspeed[i] = newY-y[i];

            x[i] = newX;
            y[i] = newY;
        }
    }
    assert(boost::distance(x) == boost::distance(guyArrivalList));
    assert(boost::distance(y) == boost::distance(guyArrivalList));
    assert(boost::distance(xspeed) == boost::distance(guyArrivalList));
    assert(boost::distance(yspeed) == boost::distance(guyArrivalList));
    assert(boost::distance(supported) == boost::distance(guyArrivalList));
    assert(boost::distance(supportedSpeed) == boost::distance(guyArrivalList));
    assert(boost::distance(finishedWith) == boost::distance(guyArrivalList));
    assert(boost::distance(facing) == boost::distance(guyArrivalList));

    mt::std::vector<char>::type carry(guyArrivalList.size());
    mt::std::vector<int>::type carrySize(guyArrivalList.size());
    mt::std::vector<TimeDirection>::type carryDirection(guyArrivalList.size());

    // box carrying
    for (std::size_t i(0), isize(boost::distance(guyArrivalList)); i < isize; ++i)
    {
        if (guyArrivalList[i].getIndex() < playerInput.size() && !finishedWith[i])
        {
        	carry[i] = guyArrivalList[i].getBoxCarrying();
			carrySize[i] = 0;
			carryDirection[i] = hg::INVALID;

            std::size_t const relativeIndex(guyArrivalList[i].getIndex());
            InputList const& input(playerInput[relativeIndex]);

            if (carry[i])
            {
                bool droppable(false);
                if (input.getDown() && supported[i])
                {
                    int width(guyArrivalList[i].getWidth());
                    int height(guyArrivalList[i].getHeight());
                    int dropSize(guyArrivalList[i].getBoxCarrySize());
                    //int dropX(facing[i] ? x[i] + width : x[i] - guyArrivalList[i].getBoxCarrySize());

                    // Initialise bounds on drops based on movement direction
                    int dropY(y[i] + height - dropSize);
                    int leftBound, rightBound;
                    if (input.getLeft())
					{
                    	leftBound = x[i] - dropSize;
                    	rightBound = x[i];
					}
                    else if (input.getRight())
                    {
                    	leftBound = x[i] - dropSize + width;
                    	rightBound = x[i] + width;
                    }
                    else
                    {
                    	if (dropSize < width)
                    	{
                    		leftBound = x[i];
							rightBound = x[i] - dropSize + width;
                    	}
                    	else
                    	{
							leftBound = x[i] - dropSize + width;
							rightBound = x[i];
                    	}
                    }

                    //std::cerr << "Initial Bound " << leftBound << ", " << rightBound << "\n";

                    // Narrow drop bounds with wall collision
                    int cx = rightBound - rightBound % env.wall.segmentSize();
                    int initial_cy = dropY - dropY % env.wall.segmentSize();
                    while (cx < rightBound + dropSize)
                    {
                    	int cy = initial_cy;
                    	while (cy < dropY + dropSize)
                    	{
							if (env.wall.at(cx,cy))
							{
								rightBound = cx - dropSize;
								goto rightBoundCheckDoubleBreak;
							}
							cy += env.wall.segmentSize();
                    	}
                    	cx += env.wall.segmentSize();
                    }
                    rightBoundCheckDoubleBreak:

                    //std::cerr << "After Wall Right " << leftBound << ", " << rightBound << "\n";

                    cx = (leftBound + dropSize) - (leftBound + dropSize) % env.wall.segmentSize() - 1;
                    while (cx > leftBound)
					{
                    	int cy = initial_cy;
                    	while (cy < dropY + dropSize)
						{
                    		if (env.wall.at(cx,cy))
							{
                    			leftBound = cx + 1;
								goto leftBoundCheckDoubleBreak;
							}
							cy += env.wall.segmentSize();
						}
						cx -= env.wall.segmentSize();
					}
                    leftBoundCheckDoubleBreak:

                    //std::cerr << "After Wall Left " << leftBound << ", " << rightBound << "\n";

                    // Check bounds imposed by platforms
                    if (rightBound >= leftBound)
                    {
                    	for (std::size_t j(0), jsize(nextPlatform.size()); j < jsize; ++j)
						{
                    		int px = nextPlatform[j].getX();
                    		int py = nextPlatform[j].getY();
                    		int pw = nextPlatform[j].getWidth();
                    		int ph = nextPlatform[j].getHeight();
							if (IntersectingRectanglesExclusive(
									px, py, pw, ph,
									leftBound, dropY, rightBound - leftBound + dropSize, dropSize))
							{
								if (px + pw > leftBound + dropSize && px < rightBound + dropSize)
								{
									rightBound = px - dropSize;
								}
								if (px < rightBound && px + pw > leftBound)
								{
									leftBound = px + pw;
								}
								if (rightBound < leftBound)
								{
									break;
								}
							}
						}
                    }
                    //std::cerr << "After Plat " << leftBound << ", " << rightBound << "\n";

                    // Check bounds imposed by boxes
					if (rightBound >= leftBound)
					{
						for (std::size_t j(0), jsize(nextBox.size()); j < jsize; ++j)
						{
							int bx = nextBox[j].object.getX();
							int by = nextBox[j].object.getY();
							int bs = nextBox[j].object.getSize();
							//std::cerr << "x: " << bx << ", y: " << by << ", w: " << bs << ", h: " << bs << "\n";
							//std::cerr << "x: " << leftBound << ", y: " << dropY << ", w: " << rightBound - leftBound + dropSize <<  ", w: " << dropSize << "\n";
							if (IntersectingRectanglesExclusive(
									bx, by, bs, bs,
									leftBound, dropY, rightBound - leftBound + dropSize, dropSize))
							{
								if (bx + bs >= leftBound + dropSize && bx <= rightBound + dropSize)
								{
									rightBound = bx - dropSize;
								}
								if (bx <= rightBound && bx + bs >= leftBound)
								{
									leftBound = bx + bs;
								}
								if (rightBound < leftBound)
								{
									break;
								}
							}
						}
					}

					//std::cerr << "After Box " << leftBound << ", " << rightBound << "\n";

                    droppable = rightBound >= leftBound;

					if (droppable)
					{

						// Choose where to drop it within bound
						int dropX;
						if (input.getLeft())
						{
							dropX = leftBound;
						}
						else if (input.getRight())
						{
							dropX = rightBound;
						}
						else
						{
							int midX = x[i]+width/2-dropSize/2;
							if (leftBound <= midX)
							{
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

						// Add box
						makeBoxAndTimeWithPortalsAndMutators(
							nextBox,
							nextBoxNormalDeparture,
							nextPortal,
							mutators,
							dropX,
							dropY,
							0,
							yspeed[i],
							dropSize,
							-1,
							guyArrivalList[i].getBoxCarryDirection(),
							triggerFrameState,
							boxGlitzAdder,
							frame);

						carry[i] = false;
						carrySize[i] = 0;
						carryDirection[i] = hg::INVALID;
					}
                }

                if (!droppable)
                {
                    carrySize[i] = guyArrivalList[i].getBoxCarrySize();
                    carryDirection[i] = guyArrivalList[i].getBoxCarryDirection();
                }
            }
            else
            {
                if (input.getDown())
                {

                    int width = guyArrivalList[i].getWidth();
                    int height = guyArrivalList[i].getHeight();
                    //CAREFUL - loop modifies nextBox
                    mt::std::vector<ObjectAndTime<Box, Frame*> >::type::iterator nextBoxIt(nextBox.begin()),nextBoxEnd(nextBox.end());
					mt::std::vector<char>::type::iterator nextBoxNormalDepartureIt(nextBoxNormalDeparture.begin());
                    for (;nextBoxIt != nextBoxEnd; ++nextBoxIt, ++nextBoxNormalDepartureIt)
                    {
                    	if (*nextBoxNormalDepartureIt)
						{
							int boxX = nextBoxIt->object.getX();
							int boxY = nextBoxIt->object.getY();
							int boxSize = nextBoxIt->object.getSize();
							if ((x[i] < boxX+boxSize) && (x[i]+width > boxX) && (y[i] < boxY+boxSize)&& (y[i]+height > boxY))
							{
								carry[i] = true;
								carrySize[i] = boxSize;
								carryDirection[i] = nextBoxIt->object.getTimeDirection();

								nextBoxIt = nextBox.erase(nextBoxIt);
								nextBoxNormalDepartureIt = nextBoxNormalDeparture.erase(nextBoxNormalDepartureIt);
								nextBoxEnd = nextBox.end();
								break;
							}
						}
                    }
                }
                else
                {
                    carrySize[i] = 0;
                    carryDirection[i] = hg::INVALID;
                }
            }
        }
    }
    assert(boost::distance(carry) == boost::distance(guyArrivalList));
    assert(boost::distance(carrySize) == boost::distance(guyArrivalList));
    assert(boost::distance(carryDirection) == boost::distance(guyArrivalList));

    mt::std::vector<int>::type newWidth(guyArrivalList.size());
    mt::std::vector<int>::type newHeight(guyArrivalList.size());
    mt::std::vector<int>::type newJumpSpeed(guyArrivalList.size());
    mt::std::vector<mt::std::map<Ability, int>::type >::type newPickups(guyArrivalList.size());
    mt::std::vector<int>::type illegalPortal(guyArrivalList.size());
	mt::std::vector<int>::type newTimePaused(guyArrivalList.size());
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
            const std::size_t relativeIndex(guyArrivalList[i].getIndex());
            const InputList& input = playerInput[relativeIndex];

            int arrivalBasis = -1;
            illegalPortal[i] = -1;

            newWidth[i] = guyArrivalList[i].getWidth();
            newHeight[i] = guyArrivalList[i].getHeight();
            newJumpSpeed[i] = guyArrivalList[i].getJumpSpeed();
            newPickups[i] = mt::std::map<Ability, int>::type(guyArrivalList[i].getPickups());
			newTimePaused[i] = guyArrivalList[i].getTimePaused();

            TimeDirection nextTimeDirection = guyArrivalList[i].getTimeDirection();
            Frame* nextTime(nextFrame(frame, nextTimeDirection));
            assert(!isNullFrame(frame));

            // Mutators
            // Mutator and falling must occur first due to exact frame effects.
            mt::std::vector<int>::type mutatorCollisions;
			for (unsigned j = 0; j < mutators.size(); ++j)
			{
				if (IntersectingRectanglesInclusiveCollisionOverlap(
						x[i], y[i], newWidth[i], newHeight[i],
						mutators[j].getX(), mutators[j].getY(),
						mutators[j].getWidth(), mutators[j].getHeight(),
						mutators[j].getCollisionOverlap()))
				{
					mutatorCollisions.push_back(i);
				}
			}

			if (mutatorCollisions.size() != 0)
			{
				boost::optional<Guy> newGuy = triggerFrameState.mutateObject(
					mutatorCollisions,
					Guy(relativeIndex,
                        x[i], y[i],
						xspeed[i], yspeed[i],
						newWidth[i], newHeight[i],
						newJumpSpeed[i],
						guyArrivalList[i].getIllegalPortal(),
						-1,
						supported[i],
						supportedSpeed[i],
						newPickups[i],
						facing[i],
						carry[i],
						carrySize[i],
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
				carrySize[i] = newGuy->getBoxCarrySize();
				carryDirection[i] = newGuy->getBoxCarryDirection();
				nextTimeDirection = newGuy->getTimeDirection();
				newTimePaused[i] = newGuy->getTimePaused();
				// relativeIndex is missing for obvious reasons
			}

            guyGlitzAdder.addGlitzForGuy(
                vector2<int>(x[i], y[i]),
                vector2<int>(xspeed[i], yspeed[i]),
                vector2<int>(newWidth[i], newHeight[i]),
                nextTimeDirection,
                facing[i],
                carry[i],
                carrySize[i],
                carryDirection[i]);

			// Things that do time travel
			// The occurrence of one thing precludes subsequent ones
            bool normalDeparture = true;

            // falling through portals
			if (normalDeparture)
			{
				for (unsigned int j = 0; j < nextPortal.size(); ++j)
				{
					if (IntersectingRectanglesInclusiveCollisionOverlap(
							x[i], y[i], newWidth[i], newHeight[i],
							nextPortal[j].getX(), nextPortal[j].getY(),
							nextPortal[j].getWidth(), nextPortal[j].getHeight(),
							nextPortal[j].getCollisionOverlap())
						&& nextPortal[j].getFallable())
					{
						if (guyArrivalList[i].getIllegalPortal() != -1 && nextPortal[j].getIndex() == guyArrivalList[i].getIllegalPortal())
						{
							illegalPortal[i] = j;
						}
						else if (triggerFrameState.shouldPort(j,
								Guy(relativeIndex,x[i], y[i],xspeed[i], yspeed[i], newWidth[i], newHeight[i],
						        newJumpSpeed[i],
								illegalPortal[i],-1,
								supported[i],supportedSpeed[i], newPickups[i], facing[i],
								carry[i],carrySize[i], carryDirection[i],nextTimeDirection,newTimePaused[i]),false))
						{
							if (nextPortal[j].getWinner())
							{
								winFrame = true;
								nextTime = 0;
								normalDeparture = false;
								break;
							}
							Frame* portalTime(
								nextPortal[j].getRelativeTime() ?
                                    getArbitraryFrameClamped(
                                        getUniverse(frame),
                                        getFrameNumber(frame) + nextPortal[j].getTimeDestination()):
                                    getArbitraryFrame(
                                        getUniverse(frame),
                                        nextPortal[j].getTimeDestination()));
							nextTime = portalTime ? nextFrame(portalTime, nextTimeDirection) : 0;
							normalDeparture = false;
							illegalPortal[i] = nextPortal[j].getIllegalDestination();
							arrivalBasis = nextPortal[j].getDestinationIndex();
							x[i] = x[i] - nextPortal[j].getX() + nextPortal[j].getXdestination() - nextPortal[j].getXspeed();
							y[i] = y[i] - nextPortal[j].getY() + nextPortal[j].getYdestination() - nextPortal[j].getYspeed();
							break;
						}
					}
				}
			}

			// for single frame effects
			// "forced" departures occur before those due to input
			if (normalDeparture)
			{
				mt::std::map<Ability, int>::type::iterator timeJump(newPickups[i].find(TIME_JUMP));
				mt::std::map<Ability, int>::type::iterator timeReverse(newPickups[i].find(TIME_REVERSE));
				mt::std::map<Ability, int>::type::iterator timePause(newPickups[i].find(TIME_PAUSE));

				Ability inputAbility = input.getAbility();

				if (inputAbility == hg::TIME_JUMP && timeJump != newPickups[i].end() && timeJump->second != 0)
				{
					nextTime = getArbitraryFrame(getUniverse(frame), getFrameNumber(input.getTimeParam()));
					normalDeparture = false;
					if (timeJump->second > 0)
					{
						newPickups[i][hg::TIME_JUMP] = timeJump->second - 1;
					}
				}
				else if (inputAbility == hg::TIME_REVERSE && timeReverse != newPickups[i].end() && timeReverse->second != 0)
				{
					normalDeparture = false;
					nextTimeDirection *= -1;
					nextTime = nextFrame(frame, nextTimeDirection);
					carryDirection[i] *= -1;
					if (timeReverse->second > 0)
					{
						newPickups[i][hg::TIME_REVERSE] = timeReverse->second - 1;
					}
				}
				else if (inputAbility == hg::TIME_PAUSE && timePause != newPickups[i].end() && timePause->second != 0)
				{
					nextTime = frame;
					newTimePaused[i] = !newTimePaused[i];
					normalDeparture = false;
					if (timePause->second > 0)
					{
						newPickups[i][hg::TIME_PAUSE] = timePause->second - 1;
					}
				}
				else if (input.getUse() == true)
				{
					for (unsigned int j = 0; j < nextPortal.size(); ++j)
					{
						if (IntersectingRectanglesInclusiveCollisionOverlap(
								x[i], y[i], newWidth[i], newHeight[i],
								nextPortal[j].getX(), nextPortal[j].getY(),
								nextPortal[j].getWidth(), nextPortal[j].getHeight(),
								nextPortal[j].getCollisionOverlap())
							&& (triggerFrameState.shouldPort(j,
									Guy(relativeIndex,x[i], y[i],xspeed[i], yspeed[i], newWidth[i], newHeight[i],
								    newJumpSpeed[i],
									illegalPortal[i],-1,
									supported[i],supportedSpeed[i], newPickups[i], facing[i],
									carry[i],carrySize[i], carryDirection[i],nextTimeDirection,newTimePaused[i]),true)))
						{
							if (nextPortal[j].getWinner())
							{
								winFrame = true;
								nextTime = 0;
								normalDeparture = false;
								break;
							}
							Frame* portalTime(
								nextPortal[j].getRelativeTime() ?
									getArbitraryFrameClamped(
										getUniverse(frame),
										getFrameNumber(frame) + nextPortal[j].getTimeDestination()) :
									getArbitraryFrame(
										getUniverse(frame),
										nextPortal[j].getTimeDestination()));
							nextTime = portalTime ? nextFrame(portalTime, nextTimeDirection) : 0;
							normalDeparture = false;
							illegalPortal[i] = nextPortal[j].getIllegalDestination();
							arrivalBasis = nextPortal[j].getDestinationIndex();
							x[i] = x[i] - nextPortal[j].getX() + nextPortal[j].getXdestination() - nextPortal[j].getXspeed();
							y[i] = y[i] - nextPortal[j].getY() + nextPortal[j].getYdestination() - nextPortal[j].getYspeed();
							break;
						}
					}
				}
			}

            if (not normalDeparture)
            {
            	nextGuy.push_back(
					ObjectAndTime<Guy, Frame*>(
						Guy(
                            relativeIndex + 1,
							x[i], y[i],
							xspeed[i], yspeed[i],
							newWidth[i], newHeight[i],
							newJumpSpeed[i],

							illegalPortal[i],
							arrivalBasis,
							supported[i],
							supportedSpeed[i],

							newPickups[i],
							facing[i],

							carry[i],
							carrySize[i],
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
            nextPlayerFrame = true;
            finishedWith[i] = true;
        }
    }

    // Only normal departures make it this far, time for inter-guy communication (eg guns)
    // This can now occur because NO POSITION CHANGE IS POSSIBLE PAST THIS POINT
	for (std::size_t i(0), size(guyArrivalList.size()); i != size; ++i)
	{
		if (finishedWith[i])
		{
			continue;
		}
		nextGuy.push_back(
			ObjectAndTime<Guy, Frame*>(
				Guy(
					guyArrivalList[i].getIndex() + 1,
					x[i], y[i],
					xspeed[i], yspeed[i],
					newWidth[i], newHeight[i],
					newJumpSpeed[i],

					illegalPortal[i],
					-1,
					supported[i],
					supportedSpeed[i],

					newPickups[i],
					facing[i],

					carry[i],
					carrySize[i],
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
    typename RandomAccessBoxRange,
    typename RandomAccessPortalRange,
    typename RandomAccessPlatformRange,
    typename RandomAccessArrivalLocationRange,
    typename RandomAccessMutatorRange,
    typename FrameT>
void boxCollisionAlogorithm(
    Environment const& env,
    RandomAccessBoxRange const& boxArrivalList,
    mt::std::vector<Box>::type const& additionalBox,
    typename mt::std::vector<ObjectAndTime<Box, FrameT> >::type& nextBox,
    mt::std::vector<char>::type& nextBoxNormalDeparture,
    BoxGlitzAdder const& boxGlitzAdder,
    RandomAccessPlatformRange const& nextPlatform,
    RandomAccessPortalRange const& nextPortal,
    RandomAccessArrivalLocationRange const& arrivalLocations,
    RandomAccessMutatorRange const& mutators,
    TriggerFrameState& triggerFrameState,
    FrameT const& frame)
{
	mt::std::vector<Box>::type oldBoxList;

	boost::push_back(oldBoxList, boxArrivalList);
	boost::push_back(oldBoxList, additionalBox);

	boost::sort(oldBoxList);


	//std::cerr << "*** New Step ***\n";
	/*
	foreach (Collision const& platform, nextPlatform)
	{
		int pX(platform.getX());
		int pY(platform.getY());
		int pWidth(platform.getWidth());
		int pHeight(platform.getHeight());
		std::cerr << "Platform " << pX << ", " << pY << ", " << pWidth << ", " << pHeight << "\n";
	}
	*/

	mt::std::vector<int>::type x(oldBoxList.size());
	mt::std::vector<int>::type y(oldBoxList.size());
	mt::std::vector<int>::type xTemp(oldBoxList.size());
	mt::std::vector<int>::type yTemp(oldBoxList.size());
	mt::std::vector<int>::type xPreBox(oldBoxList.size());
	mt::std::vector<int>::type yPreBox(oldBoxList.size());
	mt::std::vector<int>::type size(oldBoxList.size());
	mt::std::vector<char>::type squished(oldBoxList.size());

	// Check with triggers if the box should arrive at all
	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i) {
		if (not triggerFrameState.shouldArrive(oldBoxList[i])) {
			squished[i] = true;
		}
	}

	// Inititalise box location with arrival basis
	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i) {
        if (oldBoxList[i].getArrivalBasis() == -1) {
            xTemp[i] = oldBoxList[i].getX();
            yTemp[i] = oldBoxList[i].getY();
            x[i] = xTemp[i] + oldBoxList[i].getXspeed();
			y[i] = yTemp[i] + oldBoxList[i].getYspeed() + env.gravity;
        }
        else
        {
            ArrivalLocation const& relativePortal(arrivalLocations[oldBoxList[i].getArrivalBasis()]);
            xTemp[i] = relativePortal.getX() + oldBoxList[i].getX();
            yTemp[i] = relativePortal.getY() + oldBoxList[i].getY();
            x[i] = xTemp[i] + oldBoxList[i].getXspeed() + relativePortal.getXspeed();
			y[i] = yTemp[i] + oldBoxList[i].getYspeed() + relativePortal.getYspeed() + env.gravity;
        }
        size[i] = oldBoxList[i].getSize();
	}

	// Destroy boxes that are overlapping with platforms and walls
	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i) {
		if (not squished[i]) {
			if (env.wall.at(xTemp[i], yTemp[i]) && env.wall.at(xTemp[i]+size[i]-1, yTemp[i])
					&& env.wall.at(xTemp[i], yTemp[i]+size[i]-1) && env.wall.at(xTemp[i]+size[i], yTemp[i]+size[i]-1))
			{
				squished[i] = true;
				continue;
			}

			foreach (Collision const& platform, nextPlatform) {
				int pX(platform.getX());
				int pY(platform.getY());
				TimeDirection pDirection(platform.getTimeDirection());
				if (pDirection * oldBoxList[i].getTimeDirection() == hg::FORWARDS) {
					pX -= platform.getXspeed();
					pY -= platform.getYspeed();
				}
				int pWidth(platform.getWidth());
				int pHeight(platform.getHeight());

				if (IntersectingRectanglesExclusive(xTemp[i], yTemp[i], size[i], size[i], pX, pY, pWidth, pHeight))
				{
					squished[i] = true;
				}
			}
		}
	}

	// Destroy boxes that are overlapping, deals with chronofrag (maybe too strictly?)
	mt::std::vector<char>::type toBeSquished(oldBoxList.size());

	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i) {
		if (!squished[i]) {
			for (std::size_t j(0); j < i; ++j) {
				if (!squished[j]) {
					if (IntersectingRectanglesExclusive(
                            xTemp[i], yTemp[i], size[i], size[i],
                            xTemp[j], yTemp[j], size[j], size[j]))
					{
						toBeSquished[i] = true;
						toBeSquished[j] = true;
					}
				}
			}
		}
	}

	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i) {
		if (toBeSquished[i]) squished[i] = true;
	}

	// do all the other things until there are no more things to do
	bool thereAreStillThingsToDo(true); // if a box moves thereAreStillThingsToDo
	bool firstTimeThrough(true);
	//unsigned int count(0);
	while (thereAreStillThingsToDo) {
		/*
		++count;
		if (count > 10)
		{
			std::cerr << count << "\n";
			for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
			{
				std::cerr << "Box(" << oldBoxList[i].getX() << ", " << oldBoxList[i].getY() << ", " << oldBoxList[i].getXspeed()
						<< ", " << oldBoxList[i].getYspeed() << ", " << oldBoxList[i].getSize() << ", -1, " << oldBoxList[i].getArrivalBasis() << ", FORWARDS),\n";
			}
			int bla = 1/0;
		}
		*/

		mt::std::vector<std::pair<bool,int> >::type top(oldBoxList.size());
		mt::std::vector<std::pair<bool,int> >::type bottom(oldBoxList.size());
		mt::std::vector<std::pair<bool,int> >::type left(oldBoxList.size());
		mt::std::vector<std::pair<bool,int> >::type right(oldBoxList.size());

		mt::std::vector<mt::std::vector<std::size_t>::type >::type topLinks(oldBoxList.size());
		mt::std::vector<mt::std::vector<std::size_t>::type >::type bottomLinks(oldBoxList.size());
		mt::std::vector<mt::std::vector<std::size_t>::type >::type rightLinks(oldBoxList.size());
		mt::std::vector<mt::std::vector<std::size_t>::type >::type leftLinks(oldBoxList.size());

		thereAreStillThingsToDo = false;

		//*** collide boxes with platforms and walls to discover the hard bounds on the system ***//
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i) {
			if (!squished[i]) {

				//std::cerr << "Box " << i << ": " << x[i] << ", " << xTemp[i] << ", " << y[i] << ", " << yTemp[i] << "\n";
				//** Check inside a wall, velocity independent which is why it is so complex **//
				// intial keep-it-inside-the-level step
				if (x[i] <= 0 || y[i] <= 0 || x[i] + size[i] > env.wall.roomWidth() || y[i] + size[i] > env.wall.roomHeight())
				{
					int xOut = x[i] + size[i] - env.wall.roomWidth();
					int yOut = y[i] + size[i] - env.wall.roomHeight();
					if (x[i] < 0 && (y[i] > 0 || x[i] < y[i]) && (yOut <= 0 || -x[i] > yOut)) {
						y[i] = y[i] - (x[i]-env.wall.segmentSize()/2)*(y[i]-yTemp[i])/std::abs(x[i]-xTemp[i]);
						x[i] = env.wall.segmentSize()/2;
					}
					else if (y[i] < 0 && (x[i] > 0 || y[i] < x[i]) && (xOut <= 0 || -y[i] > xOut)) {
						x[i] = x[i] - (y[i]-env.wall.segmentSize()/2)*(x[i]-xTemp[i])/std::abs(y[i]-yTemp[i]);
						y[i] = env.wall.segmentSize()/2;
					}
					else if (xOut > 0 && (y[i] > 0 || xOut > -y[i]) && (yOut <= 0 || xOut > yOut)) {
						y[i] = y[i] - (xOut+env.wall.segmentSize()/2+size[i])*(y[i]-yTemp[i])/std::abs(x[i]-xTemp[i]);
						x[i] = env.wall.roomWidth() - env.wall.segmentSize()/2 - size[i];
					}
					else if (yOut > 0 && (x[i] > 0 || yOut > -x[i]) && (xOut <= 0 || yOut > xOut)) {
						x[i] = x[i] - (yOut+env.wall.segmentSize()/2+size[i])*(x[i]-xTemp[i])/std::abs(y[i]-yTemp[i]);
						y[i] = env.wall.roomHeight() - env.wall.segmentSize()/2 - size[i];
					}
				}

				// TryAgainWithMoreInterpolation is only to be triggered when moving
				// the box by size[i] will ensure that the box still collides with the wall
				// it is attempted to be moved out of.
				if (false) {
                    TryAgainWithMoreInterpolation:
                    //std::cerr << "Interpolate " << i << ": " << x[i] << ", " << xTemp[i] << ", " << y[i] << ", " << yTemp[i] << "\n";
                    if (std::abs(x[i]-xTemp[i]) < std::abs(y[i]-yTemp[i])) {
                        int newY;
                    	if (y[i] < yTemp[i])
                        {
                        	newY = y[i] - y[i] % env.wall.segmentSize() + env.wall.segmentSize();
                        }
                        else
                        {
                        	newY = y[i] - y[i] % env.wall.segmentSize() - size[i];
                        }
                    	x[i] = x[i] - std::abs(y[i]-newY)*(x[i]-xTemp[i])/std::abs(y[i]-yTemp[i]);
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
							newX = x[i] - x[i] % env.wall.segmentSize() - size[i];
						}
						y[i] = y[i] - std::abs(x[i]-newX)*(y[i]-yTemp[i])/std::abs(x[i]-xTemp[i]);
						x[i] = newX;
                    }
                }

				top[i] = std::make_pair(false, 0);
				bottom[i] = std::make_pair(false, 0);
				left[i] = std::make_pair(false, 0);
				right[i] = std::make_pair(false, 0);

				// Normal case; box does not have 2 or more sides lined up on the wallmap.
				bool w00,w10,w01,w11;

				if (size[i] <= env.wall.segmentSize()) {
					// purely a speedup for this case
					w00 = env.wall.at(x[i], y[i]);
					w10 = env.wall.at(x[i]+size[i]-1, y[i]);
					w01 = env.wall.at(x[i], y[i]+size[i]-1);
					w11 = env.wall.at(x[i]+size[i]-1, y[i]+size[i]-1);
				}
				else {
					// Extra collision for box size greater than wall size (would handle other case too)
					w00 = false;
					w10 = false;
					w01 = false;
					w11 = false;

					int xOff = 0;
					while (xOff < size[i]-1) {
						xOff += env.wall.segmentSize();
						if (xOff > size[i]-1) {
							xOff = size[i]-1;
						}
						int yOff = 0;
						while (yOff < size[i]-1) {
							yOff += env.wall.segmentSize();
							if (yOff > size[i]-1) {
								yOff = size[i]-1;
							}
							// the collision test bit
							if (!w00) {
								w00 = env.wall.at(x[i]+size[i]-xOff, y[i]+size[i]-yOff);
							}
							if (!w10) {
								w10 = env.wall.at(x[i]+yOff, y[i]+size[i]-yOff);
							}
							if (!w01) {
								w01 = env.wall.at(x[i]+size[i]-xOff, y[i]+yOff);
							}
							if (!w11) {
								w11 = env.wall.at(x[i]+xOff, y[i]+yOff);
							}
						}
					}
				}

				// collide with walls based on corner status
				if (w00) {
					if (w11) {
						if (w10) {
							if (w01) {
								goto TryAgainWithMoreInterpolation;
							}
							else {
								x[i] = ((x[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
								right[i] = std::make_pair(true, x[i]);
								y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
								top[i] = std::make_pair(true, y[i]);
							}
						}
                        //This triangle check needs improvement for rectangles
						else if (w01 || !env.wall.inTopRightTriangle(x[i],y[i])) {
							x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							left[i] = std::make_pair(true, x[i]);
							y[i] = ((y[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							bottom[i] = std::make_pair(true, y[i]);
						}
						else {
							x[i] = ((x[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							right[i] = std::make_pair(true, x[i]);
							y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							top[i] = std::make_pair(true, y[i]);
						}
					}
					else if (w10) {
						if (w01) {
							x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							left[i] = std::make_pair(true, x[i]);
							y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							top[i] = std::make_pair(true, y[i]);
						}
						else {
							y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							top[i] = std::make_pair(true, y[i]);
						}
					}
					else if (w01 || env.wall.inTopRightTriangle(x[i],y[i]))
					{
						x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						left[i] = std::make_pair(true, x[i]);
					}
					else
					{
						y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						top[i] = std::make_pair(true, y[i]);
					}
				}
				else if (w10)
				{
					if (w01)
					{
						if (w11 || env.wall.inTopLeftTriangle(x[i]+size[i]-1,y[i])) // this triangle check needs improvement for rectangles
						{
							x[i] = ((x[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							right[i] = std::make_pair(true, x[i]);
							y[i] = ((y[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							bottom[i] = std::make_pair(true, y[i]);
						}
						else
						{
							x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							left[i] = std::make_pair(true, x[i]);
							y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							top[i] = std::make_pair(true, y[i]);
						}
					}
					else if (w11 || env.wall.inTopLeftTriangle(x[i]+size[i]-1,y[i]))
					{
						x[i] = ((x[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						right[i] = std::make_pair(true, x[i]);
					}
					else
					{
						y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						top[i] = std::make_pair(true, y[i]);
					}
				}
				else if (w01)
				{
					if (w11 || env.wall.inTopLeftTriangle(x[i],y[i]+size[i]-1))
					{
						y[i] = ((y[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						bottom[i] = std::make_pair(true, y[i]);
						x[i] = xTemp[i];
					}
					else
					{
						x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						left[i] = std::make_pair(true, x[i]);
					}
				}
				else if (w11)
				{
					if (env.wall.inTopRightTriangle(x[i]+size[i]-1,y[i]+size[i]-1))
					{
						y[i] = ((y[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						bottom[i] = std::make_pair(true, y[i]);
						x[i] = xTemp[i];
					}
					else
					{
						x[i] = ((x[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						right[i] = std::make_pair(true, x[i]);
					}
				}

				// Check inside a platform
				foreach (Collision const& platform, nextPlatform)
				{
					int pX(platform.getX());
					int pY(platform.getY());
					TimeDirection pDirection(platform.getTimeDirection());
					if (pDirection * oldBoxList[i].getTimeDirection() == hg::REVERSE)
					{
						pX -= platform.getXspeed();
						pY -= platform.getYspeed();
					}
					int pWidth(platform.getWidth());
					int pHeight(platform.getHeight());

					if (IntersectingRectanglesInclusive(x[i], y[i], size[i], size[i], pX, pY, pWidth, pHeight))
					{
						if (IsPointInVerticalQuadrant(x[i] + size[i]/2, y[i] + size[i]/2, pX, pY, pWidth, pHeight))
						{
							if (y[i] + size[i]/2 < pY + pHeight/2) // box above platform
							{
								y[i] = pY - size[i];
								bottom[i] = std::make_pair(true, y[i]);
								if (firstTimeThrough)
								{
									x[i] = xTemp[i] + pDirection * oldBoxList[i].getTimeDirection() * platform.getXspeed();
								}
							}
							else
							{
								y[i] = pY + pHeight;
								top[i] = std::make_pair(true, y[i]);
							}
						}
						else // left or right
						{
							if (x[i] + size[i]/2 < pX + pWidth/2) // box left of platform
							{
								x[i] = pX - size[i];
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

		// Store position before box collision
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				//std::cerr << "Wall " << i << ": " << x[i] << ", " << y[i] << " " << (right[i].first ? "> " : "  ") << (top[i].first ? "^ " : "  ") << (left[i].first ? "< " : "  ") << (bottom[i].first ? "v\n" : " \n");
				xPreBox[i] = x[i];
				yPreBox[i] = y[i];
			}
		}

		// Now make the map of vertical collisions
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				for (std::size_t j(0); j < i; ++j)
				{
					if (j != i && !squished[j])
					{
						if (IntersectingRectanglesInclusive(x[i], y[i], size[i], size[i], x[j], y[j], size[j], size[j]))
						{
							if (std::abs(x[i] + size[i]/2 - x[j] - size[j]/2) < std::abs(y[i] + size[i]/2 - y[j] - size[j]/2)) // top or bot
							{
								if (y[i] < y[j]) // i above j
								{
									bottomLinks[i].push_back(j);
									topLinks[j].push_back(i);
									//std::cerr << "Vert link " << i << " <-> " << j << "\n";
								}
								else // i below j
								{
									topLinks[i].push_back(j);
									bottomLinks[j].push_back(i);
									//std::cerr << "Vert link " << j << " <-> " << i << "\n";
								}
							}
						}
					}
				}
			}
		}

		// propagate through vertical collision links to reposition and explode
		mt::std::vector<char>::type toBeSquished(oldBoxList.size());

		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				if (bottom[i].first) // Push boxes up
				{
					//explodeBoxes(y, size, topLinks, toBeSquished, top, i, bottom[i].second, -1);
					explodeBoxesUpwards(x, xTemp, y, size, topLinks, firstTimeThrough, toBeSquished, top, i, bottom[i].second);
				}
				if (top[i].first) // Push boxes down
				{
					explodeBoxes(y, size, bottomLinks, toBeSquished, bottom, i, top[i].second, 1);
				}
			}
		}

		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (toBeSquished[i])
			{
				squished[i] = true;
			}
		}

		// Now make the map of horizontal collisions
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				for (unsigned j = 0; j < i; ++j)
				{
					if (j != i
                        && !squished[j]
                        && IntersectingRectanglesInclusive(x[i], y[i], size[i], size[i], x[j], y[j], size[j], size[j]))
					{
                        if (std::abs(x[i] + size[i]/2 - x[j] - size[j]/2) >= std::abs(y[i] + size[i]/2 - y[j] - size[j]/2)) // left or right
                        {
                            if (x[i] < x[j]) // i left of j
                            {
                                rightLinks[i].push_back(j);
                                leftLinks[j].push_back(i);
                                //std::cerr << "Hori link " << i << " <-> " << j << "\n";
                            }
                            else // i right of j
                            {
                                leftLinks[i].push_back(j);
                                rightLinks[j].push_back(i);
                                //std::cerr << "Hori link " << j << " <-> " << i << "\n";
                            }
                        }
					}
				}
			}
		}

		// propagate through horizontal collision links to reposition and explode
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			toBeSquished[i] = false;
		}

		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				if (right[i].first)
				{
					explodeBoxes(x, size, leftLinks, toBeSquished, left, i, right[i].second, -1);
				}
				if (left[i].first)
				{
					explodeBoxes(x, size, rightLinks, toBeSquished, right, i, left[i].second, 1);
				}
			}
		}

		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (toBeSquished[i])
			{
				squished[i] = true;
			}
		}

		// Collide boxes vertically which are still overlapping. These will be the unbounded ones
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
			    mt::std::vector<std::size_t>::type pass;
				recursiveBoxCollision(y, x, size, squished, pass, i, 0);
			}
		}

		// Collide them horizontally
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
			    mt::std::vector<std::size_t>::type pass;
				recursiveBoxCollision(x, y, size, squished, pass, i, 1);
			}
		}

		// If anything moved then rerun box collision until nothing moves.
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
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

	// Send boxes
	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
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
                    size[i],
                    oldBoxList[i].getIllegalPortal(),
                    oldBoxList[i].getTimeDirection(),
                    triggerFrameState,
                    boxGlitzAdder,
                    frame);
			}
			else
			{
				ArrivalLocation const& relativePortal(arrivalLocations[oldBoxList[i].getArrivalBasis()]);
				makeBoxAndTimeWithPortalsAndMutators(
                    nextBox,
                    nextBoxNormalDeparture,
                    nextPortal,
                    mutators,
                    x[i],
                    y[i],
                    x[i] - relativePortal.getX() - oldBoxList[i].getX(),
                    y[i] - relativePortal.getY() - oldBoxList[i].getY(),
                    size[i],
                    oldBoxList[i].getIllegalPortal(),
                    oldBoxList[i].getTimeDirection(),
                    triggerFrameState,
                    boxGlitzAdder,
                    frame);
			}
		}
	}
}

}//namespace hg
