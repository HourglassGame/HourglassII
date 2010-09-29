#include "PhysicsEngine.h"

#include "TimeObjectListList.h"
#include "ObjectList.h"
#include "TimeDirection.h"

#include <iostream>
#include <map>
#include <vector>

#include <cassert>
#include <cmath>

using namespace ::std;
namespace hg {

PhysicsEngine::PhysicsEngine(vector<vector<bool> > newWallmap,
                             int newWallSize,
                             int newGravity,
                             AttachmentMap nAttachmentMap,
                             TriggerSystem nTriggerSystem) :
wallmap(newWallmap),
gravity(newGravity),
wallSize(newWallSize),
attachmentMap(nAttachmentMap),
triggerSystem(nTriggerSystem)
{
}

TimeObjectListList PhysicsEngine::executeFrame(const ObjectList& arrivals,
                                               const NewFrameID time,
                                               const std::vector<InputList>& playerInput,
                                               NewFrameID& currentPlayerFrame,
                                               TimeDirection& currentPlayerDirection,
                                               NewFrameID& nextPlayerFrame,
                                               NewFrameID& winFrame) const
{
    std::vector<BoxInfo> nextBox;
    std::vector<GuyInfo> nextGuy;

    std::vector<Platform> nextPlatform;
    nextPlatform.reserve(triggerSystem.getPlatformCount());

    std::vector<Button> nextButton;
    nextButton.reserve(triggerSystem.getButtonCount());

    std::vector<PlatformDestination> platformDesinations;

    std::vector<bool> nextButtonState = std::vector<bool>(triggerSystem.getButtonCount(), false);

    std::vector<PauseInitiatorID> pauseTimes;

    map<NewFrameID, MutableObjectList> newDepartures;

    if (time.parentFrame() == NewFrameID())
    {
        // button state update
        buttonChecks(arrivals.getPlatformListRef(), arrivals.getBoxListRef(), arrivals.getGuyListRef(), arrivals.getButtonListRef(), nextButtonState, time);

        // Trigger system execution
        triggerSystem.getPlatformDestinations(nextButtonState, platformDesinations);
    }

	// platforms set their new location and velocity from trigger system data (and ofc their physical data)
	platformStep(arrivals.getPlatformListRef(), nextPlatform, platformDesinations, time);

	// pickup position update from platform

	// boxes do their crazy wizz-bang collision algorithm
    crappyBoxCollisionAlogorithm(arrivals.getBoxListRef(), nextBox, nextPlatform);

	// item simple collision algorithm

	// guys simple collision algorithm
	guyStep(arrivals.getGuyListRef(), time, playerInput,
            nextGuy, nextBox, nextPlatform, newDepartures, currentPlayerFrame, nextPlayerFrame, currentPlayerDirection, pauseTimes);

    // button position update
    buttonPositionUpdate(nextPlatform, nextButtonState, arrivals.getButtonListRef(), nextButton, time);



    buildDepartures(nextBox, nextPlatform, nextButton, nextGuy,
                    arrivals.getGuyThiefListRef(), arrivals.getBoxThiefListRef(),
                    arrivals.getGuyExtraListRef(), arrivals.getBoxExtraListRef(),
                    newDepartures, time, pauseTimes);

    // compile all departures
    TimeObjectListList returnDepartures;

    for (map<NewFrameID, MutableObjectList>::iterator it(newDepartures.begin()), end(newDepartures.end()); it != end; ++it) {
        //Consider adding insertObjectList overload which can take aim for massively increased efficiency
        returnDepartures.insertObjectList(it->first, ObjectList(it->second));
    }
	// add data to departures
	return returnDepartures;
}

void PhysicsEngine::buildDepartures(const vector<BoxInfo>& nextBox,
                                    const vector<Platform>& nextPlatform,
                                    const vector<Button>& nextButton,
                                    const vector<GuyInfo>& nextGuy,
                                    const vector<RemoteDepartureEdit<Guy> >& guyThief,
                                    const vector<RemoteDepartureEdit<Box> >& boxThief,
                                    const vector<RemoteDepartureEdit<Guy> >& guyExtra,
                                    const vector<RemoteDepartureEdit<Box> >& boxExtra,
                                    map<NewFrameID, MutableObjectList>& newDepartures,
                                    const NewFrameID time,
                                    std::vector<PauseInitiatorID>& pauseTimes
                                    ) const
{

    // pause times initiated in the frame must be sorted
    sort(pauseTimes.begin(), pauseTimes.end());

    // build departures for boxes - boxes that were paused or unpaused and in the frame
	for (size_t i = 0; i < nextBox.size(); ++i)
	{
	    Box box = nextBox[i].box;

	    // the index of the next normal departure for a box
		NewFrameID nextTime(time.nextFrame(box.getTimeDirection()));

        // check if the departure is to be stolen
		for (size_t t = 0; t < boxThief.size(); ++t)
		{
		    if (boxThief[t].getDeparture() == box)
		    {
		        // by now the departure is known to be stolen
		        // if the departure is a pause departure the departure a level up must also be stolen
                if (box.getPauseLevel() != 0)
                {
                    newDepartures[time.parentFrame()].addBoxThief
                    (
                        RemoteDepartureEdit<Box>
                        (
                            time.universe().initiatorID(),
                            Box
                            (
                                box.getX(), box.getY(), box.getXspeed(), box.getYspeed(),
                                box.getSize(), box.getTimeDirection(), box.getPauseLevel()-1
                            ),
                            true
                        )
                    );
                }

                // adds pause time departures to pause times before this one
                // also adds pause time departure to the pause time that stole the departue
                for (size_t j = 0; j < pauseTimes.size(); ++j)
                {
                    newDepartures[time.entryChildFrame(pauseTimes[j], box.getTimeDirection())].addBox
                    (
                        Box(box.getX(), box.getY(), box.getXspeed(), box.getYspeed(),
                           box.getSize(), box.getTimeDirection(), box.getPauseLevel()+1)
                    );

                    if (pauseTimes[j] == boxThief[t].getOrigin())
                    {
                        // the box is finished, goto next one
                        goto buildNextBox;
                    }
                }
                assert(false && "pauseTimes must have a element that is equal to thief origin");
		    }
		}

        // the depature is not stolen for this box

        // if the box is a pause time box and this is the end of the universe do not depart
        if (box.getPauseLevel() != 0 && time.nextFrameInUniverse(box.getTimeDirection()) != 0)
        {
            // adds an extra box to parent frame so that pause times in the same parent frame but after this one
            // see this potentially new pause time box location in their pause time
            newDepartures[time.parentFrame()].addBoxExtra
            (
                RemoteDepartureEdit<Box>
                (
                    time.universe().initiatorID(),
                    Box
                    (
                        box.getX(), box.getY(), box.getXspeed(), box.getYspeed(),
                        box.getSize(), box.getTimeDirection(), box.getPauseLevel()-1
                    ),
                    false
                )
            );
        }
		else if (nextTime.isValidFrame())
		{
		    // simply depart to next frame
			newDepartures[nextTime].addBox(box);
		}

        // add pause time departure to all spawned pause times
		for (size_t j = 0; j < pauseTimes.size(); ++j)
		{
		    newDepartures[time.entryChildFrame(pauseTimes[j], box.getTimeDirection())].addBox
            (
                Box(box.getX(), box.getY(), box.getXspeed(), box.getYspeed(),
                   box.getSize(), box.getTimeDirection(), box.getPauseLevel()+1)
            );
		}
        buildNextBox:
        ;
	}

	// special departures for boxes, from pause time
	// these boxes are pause time boxes that have changed location in pause time
	for (size_t i = 0; i < boxExtra.size(); ++i)
	{
	    Box box = boxExtra[i].getDeparture();

	    // the index of the next normal departure for a box
		NewFrameID nextTime(time.nextFrame(box.getTimeDirection()));

        // check if the departure is to be stolen
		for (size_t t = 0; t < boxThief.size(); ++t)
		{
		    if (boxThief[t].getDeparture() == box)
		    {
		        // by now the departure is known to be stolen
		        // if the departure is a pause departure the departure a level up must also be stolen
                if (box.getPauseLevel() != 0)
                {
                    newDepartures[time.parentFrame()].addBoxThief
                    (
                        RemoteDepartureEdit<Box>
                        (
                            time.universe().initiatorID(),
                            Box
                            (
                                box.getX(), box.getY(), box.getXspeed(), box.getYspeed(),
                                box.getSize(), box.getTimeDirection(), box.getPauseLevel()-1
                            ),
                            true
                        )
                    );
                }

                // adds pause time departures to pause times before this one
                // also adds pause time departure to the pause time that stole the departue
                // CHANGE FROM NORMAL BOX: only add to pause times that occur after the extra box pause time
                for (size_t j = 0; j < pauseTimes.size(); ++j)
                {
                    if (pauseTimes[j] < boxExtra[i].getOrigin())
                    {
                        newDepartures[time.entryChildFrame(pauseTimes[j], box.getTimeDirection())].addBox
                        (
                            Box(box.getX(), box.getY(), box.getXspeed(), box.getYspeed(),
                               box.getSize(), box.getTimeDirection(), box.getPauseLevel()+1)
                        );
                    }

                    if (pauseTimes[j] == boxThief[t].getOrigin())
                    {
                        // the box is finished, goto next one
                        goto buildNextExtraBox;
                    }
                }
                assert(false && "pauseTimes must have a element that is equal to thief origin");
		    }
		}

        // the depature is not stolen for this box

        // if the box is a pause time box and this is the end of the universe do not depart
        if (box.getPauseLevel() != 0 && time.nextFrameInUniverse(box.getTimeDirection()) != 0)
        {
            // adds an extra box to parent frame so that pause times in the same parent frame but after this one
            // see this potentially new pause time box location in their pause time
            newDepartures[time.parentFrame()].addBoxExtra
            (
                RemoteDepartureEdit<Box>
                (
                    time.universe().initiatorID(),
                    Box
                    (
                        box.getX(), box.getY(), box.getXspeed(), box.getYspeed(),
                        box.getSize(), box.getTimeDirection(), box.getPauseLevel()-1
                    ),
                    false
                )
            );
        }
		else if (nextTime.isValidFrame() && boxExtra[i].getPropIntoNormal())
		{
		    // simply depart to next frame
			newDepartures[nextTime].addBox(box);
		}

        // CHANGE FROM NORMAL BOX: add pause time departure to pause times after the current one
		for (size_t j = 0; j < pauseTimes.size(); ++j)
		{
		    if (pauseTimes[j] < boxExtra[i].getOrigin())
            {
                newDepartures[time.entryChildFrame(pauseTimes[j], box.getTimeDirection())].addBox
                (
                    Box(box.getX(), box.getY(), box.getXspeed(), box.getYspeed(),
                       box.getSize(), box.getTimeDirection(), box.getPauseLevel()+1)
                );
            }
		}
        buildNextExtraBox:
        ;
	}

	// build departures for platforms
	for (size_t i = 0; i < nextPlatform.size(); ++i)
	{
		NewFrameID nextTime(time.nextFrame(nextPlatform[i].getTimeDirection()));

		if (nextTime.isValidFrame() && (nextPlatform[i].getPauseLevel() == 0 || time.nextFrameInUniverse(nextPlatform[i].getTimeDirection()) == 0))
		{
			newDepartures[nextTime].addPlatform(nextPlatform[i]);
		}

		for (size_t j = 0; j < pauseTimes.size(); ++j)
		{
		    newDepartures[time.entryChildFrame(pauseTimes[j], nextPlatform[i].getTimeDirection())].addPlatform
            (
                Platform(nextPlatform[i].getX(), nextPlatform[i].getY(), nextPlatform[i].getXspeed(), nextPlatform[i].getYspeed(),
                         nextPlatform[i].getWidth(), nextPlatform[i].getHeight(), nextPlatform[i].getIndex(),
                         nextPlatform[i].getTimeDirection(), nextPlatform[i].getPauseLevel()+1)
            );
		}
	}

	// build departures for buttons
	for (size_t i = 0; i < nextButton.size(); ++i)
	{
		NewFrameID nextTime(time.nextFrame(nextButton[i].getTimeDirection()));

		if (nextTime.isValidFrame() && (nextButton[i].getPauseLevel() == 0 || time.nextFrameInUniverse(nextButton[i].getTimeDirection()) == 0))
		{
			newDepartures[nextTime].addButton(nextButton[i]);
		}

		for (size_t j = 0; j < pauseTimes.size(); ++j)
		{
		    newDepartures[time.entryChildFrame(pauseTimes[j], nextButton[i].getTimeDirection())].addButton
            (
                Button(nextButton[i].getX(), nextButton[i].getY(), nextButton[i].getXspeed(), nextButton[i].getYspeed(),
                        nextButton[i].getIndex(), nextButton[i].getState(),
                        nextButton[i].getTimeDirection(), nextButton[i].getPauseLevel()+1)
            );
		}
	}

	if (nextGuy.size() != 0)
    {
        //cout << "** " << time.frame() << endl;
    }

	// build departures for guys
	for (size_t i = 0; i < nextGuy.size(); ++i)
	{
	    NewFrameID nextTime(time.nextFrame(nextGuy[i].guy.getTimeDirection()));

	    if (nextTime.isValidFrame() && (nextGuy[i].guy.getPauseLevel() == 0 || time.nextFrameInUniverse(nextGuy[i].guy.getTimeDirection()) == 0))
		{
            newDepartures[nextGuy[i].time].addGuy(nextGuy[i].guy);
            //cout << nextTime.frame() << "  " << nextGuy[i].guy.getRelativeIndex() << endl;
		}

        /*if (nextGuy[i].time.parentFrame() != time)
        {
            for (size_t j = 0; j < pauseTimes.size(); ++j)
            {
                //cout << time.entryChildFrame(pauseTimes[j], nextGuy[i].guy.getTimeDirection()).frame() << "  " << nextGuy[i].guy.getRelativeIndex() << endl;
                newDepartures[time.entryChildFrame(pauseTimes[j], nextGuy[i].guy.getTimeDirection())].addGuy
                (
                    Guy(nextGuy[i].guy.getX(), nextGuy[i].guy.getY(), nextGuy[i].guy.getXspeed(), nextGuy[i].guy.getYspeed(),
                        nextGuy[i].guy.getWidth(), nextGuy[i].guy.getHeight(), nextGuy[i].guy.getRelativeIndex(),
                        nextGuy[i].guy.getSupported(), nextGuy[i].guy.getBoxCarrySize(),
                        nextGuy[i].guy.getBoxCarryDirection(), nextGuy[i].guy.getBoxPauseLevel(),
                        nextGuy[i].guy.getTimeDirection(), nextGuy[i].guy.getPauseLevel()+1,
                        nextGuy[i].guy.getRelativeIndex(), nextGuy[i].guy.getSubimage()
                    )
                );
            }
        }*/

	}

}

void PhysicsEngine::guyStep(const vector<Guy>& oldGuyList,
                            const NewFrameID time,
                            const vector<InputList>& playerInput,
                            ::std::vector<GuyInfo>& nextGuy,
                            std::vector<BoxInfo>& nextBox,
                            const ::std::vector<Platform>& nextPlatform,
                            ::std::map<NewFrameID, MutableObjectList>& newDepartures,
                            NewFrameID& currentPlayerFrame,
                            NewFrameID& nextPlayerFrame,
                            TimeDirection& currentPlayerDirection,
                            std::vector<PauseInitiatorID>& pauseTimes) const
{
	vector<int> x;
	vector<int> y;
	vector<int> xspeed;
	vector<int> yspeed;
	vector<bool> supported;

    x.reserve(oldGuyList.size());
    y.reserve(oldGuyList.size());
    xspeed.reserve(oldGuyList.size());
    yspeed.reserve(oldGuyList.size());

	// position, velocity, collisions
	for (size_t i = 0; i < oldGuyList.size(); ++i)
	{
        if (oldGuyList[i].getRelativeIndex() < playerInput.size() && oldGuyList[i].getPauseLevel() == 0)
        {
            x.push_back(oldGuyList[i].getX());
            y.push_back(oldGuyList[i].getY());
            xspeed.push_back(0);
            yspeed.push_back(oldGuyList[i].getYspeed() + gravity);
            supported.push_back(false);

            size_t relativeIndex = oldGuyList[i].getRelativeIndex();
            const InputList& input = playerInput[relativeIndex];

            int width = oldGuyList[i].getWidth();
            int height = oldGuyList[i].getHeight();

             // jump
            if (oldGuyList[i].getSupported() && input.getUp())
            {
                yspeed[i] = -700;
            }

            //check wall collision in Y direction
            int newY = y[i] + yspeed[i];

            if (yspeed[i] > 0) // down
            {
                if (wallAt(x[i], newY+height) || (x[i] - (x[i]/wallSize)*wallSize > wallSize-width && wallAt(x[i]+width, newY+height)))
                {
                    newY = ((newY+height)/wallSize)*wallSize - height;
                    supported[i] = true;
                }
            }
            else if (yspeed[i] < 0) // up
            {
                if  (wallAt(x[i], newY) || (x[i] - (x[i]/wallSize)*wallSize > wallSize-width && wallAt(x[i]+width, newY)))
                {
                    newY = (newY/wallSize + 1)*wallSize;
                }
            }

            // platform collision
            for (unsigned int j = 0; j < nextPlatform.size(); ++j)
            {
                int pX = nextPlatform[j].getX();
                int pY = nextPlatform[j].getY();
                TimeDirection pDirection = nextPlatform[j].getTimeDirection();
                if (pDirection*oldGuyList[i].getTimeDirection() == hg::REVERSE)
                {
                    pX -= nextPlatform[j].getXspeed();
                    pY -= nextPlatform[j].getYspeed();
                }
                int pWidth = nextPlatform[j].getWidth();
                int pHeight = nextPlatform[j].getHeight();

                if (intersectingRectangles(x[i], newY, width, height, pX, pY, pWidth, pHeight, false))
                {
                    if (newY+height/2 < pY+pHeight/2)
                    {
                        newY = pY-height;
                        xspeed[i] = pDirection*oldGuyList[i].getTimeDirection()*nextPlatform[j].getXspeed();
                        supported[i] = true;
                    }
                    else
                    {
                        newY = pY+pHeight;
                    }
                }
            }

            // box collision
            for (unsigned int j = 0; j < nextBox.size(); ++j)
            {
                int boxX = nextBox[j].box.getX();
                int boxY = nextBox[j].box.getY();
                int boxXspeed = nextBox[j].box.getXspeed();
                int boxYspeed = nextBox[j].box.getYspeed();
                int boxSize = nextBox[j].box.getSize();
                TimeDirection boxDirection = nextBox[j].box.getTimeDirection();
                if (x[i] <= boxX+boxSize && x[i]+width >= boxX)
                {
                    if (boxDirection*oldGuyList[i].getTimeDirection() == hg::REVERSE)
                    {
                        if (newY+height >= boxY-boxYspeed && newY+height-yspeed[i] <= boxY)
                        {
                            newY = boxY-height-boxYspeed;
                            xspeed[i] = -boxXspeed;
                            supported[i] = true;
                        }

                    }
                    else
                    {
                        if (newY+height >= boxY && newY-yspeed[i]+height <= boxY-boxYspeed)
                        {
                            newY = boxY-height;
                            xspeed[i] = boxXspeed;
                            supported[i] = true;
                        }
                    }
                }
            }

            //check wall collision in X direction
            int newX = x[i] + xspeed[i];

            if (input.getLeft())
            {
                newX += -250;
            }
            else if (input.getRight())
            {
                newX += 250;
            }

            if (newX-x[i] > 0) // right
            {
                if (wallAt(newX+width, newY) || (newY - (newY/wallSize)*wallSize > wallSize-height && wallAt(newX+width, newY+height)))
                {
                    newX = (newX+width)/wallSize*wallSize - width;
                }
            }
            else if (newX-x[i] < 0) // left
            {
                if (wallAt(newX, newY) || (newY - (newY/wallSize)*wallSize > wallSize-height && wallAt(newX, newY+height)))
                {
                    newX = (newX/wallSize + 1)*wallSize;
                }
            }

            // platform collision
            for (unsigned int j = 0; j < nextPlatform.size(); ++j)
            {
                int pX = nextPlatform[j].getX();
                int pY = nextPlatform[j].getY();
                TimeDirection pDirection = nextPlatform[j].getTimeDirection();
                if (pDirection*oldGuyList[i].getTimeDirection() == hg::REVERSE)
                {
                    pX -= nextPlatform[j].getXspeed();
                    pY -= nextPlatform[j].getYspeed();
                }
                int pWidth = nextPlatform[j].getWidth();
                int pHeight = nextPlatform[j].getHeight();

                if (intersectingRectangles(newX, newY, width, height, pX, pY, pWidth, pHeight, false))
                {
                    if (newX+width/2 < pX+pWidth/2)
                    {
                        newX = pX-width;
                    }
                    else
                    {
                        newX = pX+pWidth;
                    }
                }
            }

            xspeed[i] = newX-x[i];
            yspeed[i] = newY-y[i];

            x[i] = newX;
            y[i] = newY;
        }
	}

	vector<bool> carry;
	vector<int> carrySize;
	vector<TimeDirection> carryDirection;
	vector<int> carryPauseLevel;

	// box carrying
	for (size_t i = 0; i < oldGuyList.size(); ++i)
	{
        if (oldGuyList[i].getRelativeIndex() < playerInput.size() && oldGuyList[i].getPauseLevel() == 0)
        {
            carry.push_back(oldGuyList[i].getBoxCarrying());
            carrySize.push_back(0);
            carryDirection.push_back(hg::INVALID);
            carryPauseLevel.push_back(0);

            size_t relativeIndex = oldGuyList[i].getRelativeIndex();
            const InputList& input = playerInput[relativeIndex];

            if (carry[i])
            {
                bool droppable = false;
                if (input.getDown())
                {
                    int width = oldGuyList[i].getWidth();
                    int dropX = x[i] + width/2 - oldGuyList[i].getBoxCarrySize()/2;
                    int dropY = y[i] - oldGuyList[i].getBoxCarrySize();
                    int dropSize = oldGuyList[i].getBoxCarrySize();

                    if (!wallAt(dropX, dropY, dropSize, dropSize))
                    {
                        droppable = true;
                        for (unsigned int j = 0; droppable && j < nextBox.size(); ++j)
                        {
                            if (intersectingRectangles(nextBox[j].box.getX(), nextBox[j].box.getY(), nextBox[j].box.getSize(), nextBox[j].box.getSize(),
                                                      dropX, dropY, dropSize, dropSize, false))
                            {
                                droppable = false;
                            }
                        }
                        for (unsigned int j = 0; droppable && j < nextPlatform.size(); ++j)
                        {
                            if (intersectingRectangles(nextPlatform[j].getX(), nextPlatform[j].getY(),
                                                       nextPlatform[j].getWidth(), nextPlatform[j].getHeight(),
                                                      dropX, dropY, dropSize, dropSize, false))
                            {
                                droppable = false;
                                break;
                            }
                        }
                        if (droppable)
                        {
                            nextBox.push_back
                            (
                                PhysicsEngine::BoxInfo
                                (
                                    Box
                                    (
                                        dropX,
                                        dropY,
                                        0,
                                        0,
                                        dropSize,
                                        oldGuyList[i].getBoxCarryDirection(),
                                        oldGuyList[i].getBoxPauseLevel()
                                    ),
                                false
                                )
                            );

                            if (oldGuyList[i].getBoxPauseLevel() != 0)
                            {
                                newDepartures[time.parentFrame()].addBoxExtra
                                (
                                    RemoteDepartureEdit<Box>
                                    (
                                        time.universe().initiatorID(),
                                        Box
                                        (
                                            dropX, dropY, 0, 0,
                                            dropSize, oldGuyList[i].getBoxCarryDirection(),
                                            oldGuyList[i].getBoxPauseLevel()-1
                                        ),
                                        true
                                    )

                                );
                            }

                            carry[i] = false;
                            carrySize[i] = 0;
                            carryDirection[i] = hg::INVALID;
                            carryPauseLevel[i] = 0;
                        }
                    }
                }

                if (!droppable)
                {
                    carrySize[i] = oldGuyList[i].getBoxCarrySize();
                    carryDirection[i] = oldGuyList[i].getBoxCarryDirection();
                    carryPauseLevel[i] = oldGuyList[i].getBoxPauseLevel();
                }
            }
            else
            {
                if (input.getDown())
                {
                    int width = oldGuyList[i].getWidth();
                    int height = oldGuyList[i].getHeight();

                    for (unsigned int j = 0; j < nextBox.size(); ++j)
                    {
                        int boxX = nextBox[j].box.getX();
                        int boxY = nextBox[j].box.getY();
                        int boxSize = nextBox[j].box.getSize();
                        if ((x[i] < boxX+boxSize) && (x[i]+width > boxX) && (y[i] < boxY+boxSize)&& (y[i]+height > boxY))
                        {
                            carry[i] = true;
                            carrySize[i] = boxSize;
                            carryDirection[i] = nextBox[j].box.getTimeDirection();
                            carryPauseLevel[i] = nextBox[j].box.getPauseLevel();
                            if (nextBox[j].box.getPauseLevel() != 0)
                            {
                                newDepartures[time.parentFrame()].addBoxThief
                                (
                                    RemoteDepartureEdit<Box>
                                    (
                                        time.universe().initiatorID(),
                                        Box
                                        (
                                            boxX, boxY, nextBox[j].box.getXspeed(), nextBox[j].box.getYspeed(),
                                            boxSize, carryDirection[i], carryPauseLevel[i]-1
                                        ),
                                        true
                                    )
                                );
                            }
                            nextBox.erase(nextBox.begin() + j);
                            break;
                        }
                    }
                }
                else
                {
                    carrySize[i] = 0;
                    carryDirection[i] = hg::INVALID;
                    carryPauseLevel[i] = 0;
                }
            }
        }
	}
	// colliding with pickups?

	// time travel
	for (size_t i = 0; i < oldGuyList.size(); ++i)
	{
        if (oldGuyList[i].getPauseLevel() != 0)
	    {
	        nextGuy.push_back
            (
                PhysicsEngine::GuyInfo
                (
                    oldGuyList[i],time.nextFrame(oldGuyList[i].getTimeDirection())
                )
            );
	    }
        else if (oldGuyList[i].getRelativeIndex() < playerInput.size())
        {
            // animation
            int nextSubimage = oldGuyList[i].getSubimage() + 1;
            if (nextSubimage > Guy::animationLength)
            {
                nextSubimage = 0;
            }

            size_t relativeIndex(oldGuyList[i].getRelativeIndex());
            const InputList& input = playerInput[relativeIndex];
            int nextCarryPauseLevel = carryPauseLevel[i];

            // add departure for guy at the appropriate time
            TimeDirection nextTimeDirection = oldGuyList[i].getTimeDirection();
            NewFrameID nextTime(time.nextFrame(nextTimeDirection));
            assert(time.isValidFrame());
            if (input.getAbility() == hg::TIME_JUMP)
            {
                nextTime = input.getFrameIdParam(0);
            }
            else if (input.getAbility() == hg::TIME_REVERSE)
            {
                nextTimeDirection *= -1;
                nextTime = time.nextFrame(nextTimeDirection);
                carryDirection[i] *= -1;

                if (time.nextFrameInUniverse(nextTimeDirection) != 0)
                {
                    nextCarryPauseLevel -= time.nextFrameInUniverse(nextTimeDirection);
                    if (nextCarryPauseLevel < 0)
                    {
                        nextCarryPauseLevel = 0;
                    }
                }
            }
            else if (input.getAbility() == hg::PAUSE_TIME)
            {
                PauseInitiatorID pauseID = PauseInitiatorID(pauseinitiatortype::GUY, relativeIndex, 500);
                nextTime = time.entryChildFrame(pauseID, oldGuyList[i].getTimeDirection());

                pauseTimes.push_back(pauseID);
            }
            else
            {
                if (time.nextFrameInUniverse(nextTimeDirection) != 0)
                {
                    nextCarryPauseLevel -= time.nextFrameInUniverse(nextTimeDirection);
                    if (nextCarryPauseLevel < 0)
                    {
                        nextCarryPauseLevel = 0;
                    }
                }
            }

            if (playerInput.size() - 1 == relativeIndex)
            {
                currentPlayerDirection = oldGuyList[i].getTimeDirection();
                currentPlayerFrame = time;
                nextPlayerFrame = nextTime;
                //cout << "nextPlayerFrame set to: " << nextPlayerFrame.frame() << "\n";
            }

            if (nextTime.isValidFrame())
            {
                nextGuy.push_back
                (
                    GuyInfo(Guy(x[i], y[i], xspeed[i], yspeed[i],
                        oldGuyList[i].getWidth(), oldGuyList[i].getHeight(),
                        supported[i],
                        carry[i], carrySize[i], carryDirection[i], nextCarryPauseLevel,
                        nextTimeDirection, 0,
                        relativeIndex+1, nextSubimage),
                    nextTime)
                );
            }

        }
        else
        {
            assert(oldGuyList[i].getRelativeIndex() == playerInput.size());
        }
    }
}

void PhysicsEngine::crappyBoxCollisionAlogorithm(const vector<Box>& oldBoxList,
                                                 std::vector<BoxInfo>& nextBox,
                                                 const ::std::vector<Platform>& nextPlatform) const
{
	for (vector<Box>::const_iterator i(oldBoxList.begin()), iend(oldBoxList.end()); i != iend; ++i)
	{

	    if (i->getPauseLevel() != 0)
	    {
	        nextBox.push_back
            (
                PhysicsEngine::BoxInfo
                (
                    *i,true
                )
            );

	        continue;
	    }

		int x = i->getX();
		int y = i->getY();
		int xspeed = i->getXspeed();
		int yspeed = i->getYspeed() + gravity;
		int size = i->getSize();

		bool supported = false;

        bool exploded = false;

        for (vector<Box>::const_iterator j(oldBoxList.begin()), jend(oldBoxList.end()); j != jend; ++j)
        {
            if (j != i)
            {
                int boxX = j->getX();
                int boxY = j->getY();
                int boxSize = j->getSize();
                if (PhysicsEngine::intersectingRectangles(x, y, size, size, boxX, boxY, boxSize, boxSize, false))
                {
                    exploded = true;
                    continue;
                }
            }
        }
        /*
        for (vector<Platform>::const_iterator j(nextPlatform.begin()), jend(nextPlatform.end()); j != jend; ++j)
        {
            if (PhysicsEngine::intersectingRectangles(x, y, size, size, j->getX(), j->getY(), j->getHeight(), j->getWidth(), false))
            {
                exploded = true;
                continue;
            }
        }
        */
        if (exploded)
        {
            continue;
        }

        // ** Y component **
        int newY = y + yspeed;

		// box collision
        for (vector<Box>::const_iterator j(oldBoxList.begin()), jend(oldBoxList.end()); j != jend; ++j)
        {
            if (j != i)
            {
                int boxX = j->getX();
                int boxY = j->getY();
                int boxSize = j->getSize();
                if (PhysicsEngine::intersectingRectangles(x, newY, size, size, boxX, boxY, boxSize, boxSize, true))
                {
                    if (newY + size/2 < boxY + boxSize/2)
                    {
                        newY = boxY - size;
                    }
                    else
                    {
                        newY = boxY + boxSize;
                    }
                }
            }
        }
        yspeed = newY - y;

		//check wall collision in Y direction
		if (yspeed > 0) // down
		{
			if (wallmap[x/wallSize][(newY+size)/wallSize] || (x - (x/wallSize)*wallSize > wallSize-size && wallmap[(x+size)/wallSize][(newY+size)/wallSize]))
			{
				newY = ((newY+size)/wallSize)*wallSize - size;
				supported = true;
				xspeed = 0;
			}
		}
		else if (yspeed < 0) // up
		{
			if  (wallmap[x/wallSize][newY/wallSize] || (x - (x/wallSize)*wallSize > wallSize-size && wallmap[(x+size)/wallSize][newY/wallSize]))
			{
				newY = (newY/wallSize + 1)*wallSize;
			}
		}

		// platform collision
        for (unsigned int j = 0; j < nextPlatform.size(); ++j)
        {
            int pX = nextPlatform[j].getX();
            int pY = nextPlatform[j].getY();
            TimeDirection pDirection = nextPlatform[j].getTimeDirection();
            if (pDirection*i->getTimeDirection() == hg::REVERSE)
            {
                pX -= nextPlatform[j].getXspeed();
                pY -= nextPlatform[j].getYspeed();
            }
            int pWidth = nextPlatform[j].getWidth();
            int pHeight = nextPlatform[j].getHeight();

            if (intersectingRectangles(x, newY, size, size, pX, pY, pWidth, pHeight, false))
            {
                if (newY+size/2 < pY+pHeight/2)
                {
                    newY = pY-size;
                    xspeed = pDirection*i->getTimeDirection()*nextPlatform[j].getXspeed();
                    supported = true;
                }
                else
                {
                    newY = pY+pHeight;
                }
            }
        }

        // ** X component **
        int newX = x + xspeed;

        for (vector<Box>::const_reverse_iterator j(oldBoxList.rbegin()), jend(oldBoxList.rend()); j != jend; ++j)
        {
            if ((j.base() - 1) != i )
            {
                {
                    int boxX = j->getX();
                    int boxY = j->getY();
                    int boxSize = j->getSize();
                    if (PhysicsEngine::intersectingRectangles(newX, newY, size, size, boxX, boxY, boxSize, boxSize, true))
                    {
                        if (newX + size/2 < boxX + boxSize/2)
                        {
                            newX = x + j->getXspeed()/10;//boxX - size;
                        }
                        else
                        {
                            newX = x + j->getXspeed()/10;//boxX + boxSize;
                        }
                    }
                }
            }
        }
        xspeed = newX - x;

		//check wall collision in X direction
		if (xspeed > 0) // right
		{
			if (wallmap[(newX+size)/wallSize][newY/wallSize] || (newY - (newY/wallSize)*wallSize > wallSize-size && wallmap[(newX+size)/wallSize][(newY+size)/wallSize]))
			{
				newX = (newX+size)/wallSize*wallSize - size;
			}
		}
		else if (xspeed < 0) // left
		{
			if (wallmap[newX/wallSize][newY/wallSize] || (newY - (newY/wallSize)*wallSize > wallSize-size && wallmap[newX/wallSize][(newY+size)/wallSize]))
			{
				newX = (newX/wallSize + 1)*wallSize;
			}
		}

		// platform collision
        for (unsigned int j = 0; j < nextPlatform.size(); ++j)
        {
            int pX = nextPlatform[j].getX();
            int pY = nextPlatform[j].getY();
            TimeDirection pDirection = nextPlatform[j].getTimeDirection();
            if (pDirection*i->getTimeDirection() == hg::REVERSE)
            {
                pX -= nextPlatform[j].getXspeed();
                pY -= nextPlatform[j].getYspeed();
            }
            int pWidth = nextPlatform[j].getWidth();
            int pHeight = nextPlatform[j].getHeight();

            if (intersectingRectangles(newX, newY, size, size, pX, pY, pWidth, pHeight, false))
            {
                if (newX+size/2 < pX+pWidth/2)
                {
                    newX = pX-size;
                }
                else
                {
                    newX = pX+pWidth;
                }
            }
        }

        xspeed = newX-x;
        yspeed = newY-y;

        x = newX;
        y = newY;

		nextBox.push_back
        (
            PhysicsEngine::BoxInfo
            (
                Box
                (
                    x,
                    y,
                    xspeed,
                    yspeed,
                    size,
                    i->getTimeDirection(),
                    0
                )
                ,supported
            )
        );

	}
}

void PhysicsEngine::platformStep(const ::std::vector<Platform>& oldPlatformList,
                                 ::std::vector<Platform>& nextPlatform,
                                 const std::vector<PlatformDestination>& pd,
                                 const NewFrameID& time) const
{

    for (unsigned int i = 0; i < oldPlatformList.size(); ++i)
    {
        if (oldPlatformList[i].getPauseLevel() != 0)
	    {
	        nextPlatform.push_back(oldPlatformList[i]);
	        continue;
	    }

        int x = oldPlatformList[i].getX();
		int y = oldPlatformList[i].getY();
		int xspeed = oldPlatformList[i].getXspeed();
		int yspeed = oldPlatformList[i].getYspeed();

        // X component
        if (pd[i].getX() != x)
        {
            if (abs(pd[i].getX() - x) <= abs(xspeed) && abs(xspeed) <= pd[i].getXdeccel())
            {
                xspeed = pd[i].getX() - x;
            }
            else
            {
                int direction = abs(x - pd[i].getX())/(x - pd[i].getX());

                if (xspeed*direction >= 0)
                {
                    xspeed -= direction*pd[i].getXdeccel();
                    if (xspeed*direction < 0)
                    {
                        xspeed = 0;
                    }
                }
                else
                {
                    // if the platform can still stop if it fully accelerates
                    if (abs(x - pd[i].getX()) > ((int)(pow((double)(xspeed - direction*pd[i].getXaccel()),2))*3/(2*pd[i].getXdeccel())))
                    {
                        // fully accelerate
                        xspeed -= direction*pd[i].getXaccel();
                    }
                    // if the platform can stop if it doesn't accelerate
                    else if (abs(x - pd[i].getX()) > ((int)(pow((double)(xspeed),2))*3/(2*pd[i].getXdeccel())))
                    {
                        // set speed to required speed
                        xspeed = -direction*(int)(floor((double)(abs(x - pd[i].getX())*pd[i].getXdeccel()*2/3)));
                    }
                    else
                    {
                        xspeed += direction*pd[i].getXdeccel();
                    }
                }
            }
        }
        else
        {
            if (abs(xspeed) <= pd[i].getXdeccel())
            {
                xspeed = 0;
            }
            else
            {
                xspeed += abs(xspeed)/xspeed*pd[i].getXdeccel();
            }
        }

        if (abs(xspeed) > pd[i].getXspeed())
        {
            xspeed = abs(xspeed)/xspeed*pd[i].getXspeed();
        }

		x += xspeed;

        // Y component
        if (pd[i].getY() != y)
        {
            if (abs(pd[i].getY() - y) <= abs(yspeed) && abs(yspeed) <= pd[i].getYdeccel())
            {
                yspeed = pd[i].getY() - y;
            }
            else
            {
                int direction = abs(y - pd[i].getY())/(y - pd[i].getY());

                if (yspeed*direction > 0)
                {
                    yspeed -= direction*pd[i].getYdeccel();
                    if (yspeed*direction < 0)
                    {
                        yspeed = 0;
                    }
                }
                else
                {
                    // if the platform can still stop if it fully accelerates
                    if (abs(y - pd[i].getY()) > ((int)pow((double)yspeed - direction*pd[i].getYaccel(),2))*3/(2*pd[i].getYdeccel()))
                    {
                        // fully accelerate
                        yspeed -= direction*pd[i].getYaccel();
                    }
                    // if the platform can stop if it doesn't accelerate
                    else if (abs(y - pd[i].getY()) > ((int)pow((double)yspeed,2))*3/(2*pd[i].getYdeccel()))
                    {
                        // set speed to required speed
                        yspeed = -direction*(int)floor(sqrt((double)abs(y - pd[i].getY())*pd[i].getYdeccel()*2/3));
                    }
                    else
                    {
                        yspeed += direction*pd[i].getYdeccel();
                    }
                }
            }
        }
        else
        {
            if (abs(yspeed) <= pd[i].getYdeccel())
            {
                yspeed = 0;
            }
            else
            {
                yspeed += abs(yspeed)/yspeed*pd[i].getYdeccel();
            }
        }

        if (abs(yspeed) > pd[i].getYspeed())
        {
            yspeed = abs(yspeed)/yspeed*pd[i].getYspeed();
        }

        y += yspeed;

		// send information
        nextPlatform.push_back(Platform(x, y, xspeed, yspeed, oldPlatformList[i].getWidth(), oldPlatformList[i].getHeight(), oldPlatformList[i].getIndex(), oldPlatformList[i].getTimeDirection(), 0));
    }
}

void PhysicsEngine::buttonPositionUpdate(
        const ::std::vector<Platform>& nextPlatform,
        const ::std::vector<bool>& nextButtonState,
        const ::std::vector<Button>& oldButtonList,
        vector<Button>& nextButton,
        NewFrameID time
    ) const
{

    ::std::vector< ::boost::tuple<int, int, int> > attachments = attachmentMap.getButtonAttachmentRef();


    for (unsigned int i = 0; i < oldButtonList.size(); ++i)
	{
	    if (oldButtonList[i].getPauseLevel() != 0)
	    {
	        nextButton.push_back(oldButtonList[i]);
	        continue;
	    }

	    int x = oldButtonList[i].getX();
	    int y = oldButtonList[i].getY();

	    int index = oldButtonList[i].getIndex();

	    if (attachments[index].get<0>() != -1)
	    {
	        int pid = attachments[index].get<0>();

	        if (nextPlatform[pid].getTimeDirection()*oldButtonList[index].getTimeDirection() == hg::FORWARDS)
	        {
	            x = nextPlatform[pid].getX()+attachments[index].get<1>();
                y = nextPlatform[pid].getY()+attachments[index].get<2>();
	        }
	        else
	        {
	            x = nextPlatform[pid].getX()-nextPlatform[pid].getXspeed()+attachments[index].get<1>();
                y = nextPlatform[pid].getY()-nextPlatform[pid].getYspeed()+attachments[index].get<2>();
	        }
	    }

        nextButton.push_back(Button(x, y, x-oldButtonList[i].getX(), y-oldButtonList[i].getY(), index, nextButtonState[i], oldButtonList[index].getTimeDirection(), 0));
	}
}


void PhysicsEngine::buttonChecks(const ::std::vector<Platform>& oldPlatformList,
                                 const ::std::vector<Box>& oldBoxList,
                                const ::std::vector<Guy>& oldGuyList,
                                const ::std::vector<Button>& oldButtonList,
                                ::std::vector<bool>& nextButton,
                                NewFrameID time) const
{

    ::std::vector< ::boost::tuple<int, int, int> > attachments = attachmentMap.getButtonAttachmentRef();

    for (unsigned int i = 0; i < oldButtonList.size(); ++i)
	{

	    int x = oldButtonList[i].getX();
	    int y = oldButtonList[i].getY();
	    int w = 3200;
	    int h = 800;
	    bool state = false;

	    int index = oldButtonList[i].getIndex();

	    if (attachments[index].get<0>() != -1)
	    {
	        int pid = attachments[index].get<0>();

	        if (oldPlatformList[pid].getTimeDirection()*oldButtonList[index].getTimeDirection() == hg::FORWARDS)
	        {
	            x = oldPlatformList[pid].getX()+attachments[index].get<1>();
                y = oldPlatformList[pid].getY()+attachments[index].get<2>();
	        }
	        else
	        {
	            x = oldPlatformList[pid].getX()-oldPlatformList[pid].getXspeed()+attachments[index].get<1>();
                y = oldPlatformList[pid].getY()-oldPlatformList[pid].getYspeed()+attachments[index].get<2>();
	        }
	    }

	    for (unsigned int j = 0; !state && j < oldBoxList.size(); ++j)
        {
            state = PhysicsEngine::intersectingRectangles(x, y, w, h, oldBoxList[j].getX(), oldBoxList[j].getY(), oldBoxList[j].getSize(), oldBoxList[j].getSize(), true);
        }

        for (unsigned int j = 0; !state && j < oldGuyList.size(); ++j)
        {
            state = PhysicsEngine::intersectingRectangles(x, y, w, h, oldGuyList[j].getX(), oldGuyList[j].getY(), oldGuyList[j].getWidth(), oldGuyList[j].getHeight(), true);
        }

        nextButton[oldButtonList[i].getIndex()] = state;
	}
}

bool PhysicsEngine::wallAt(int x, int y) const
{
	if (x < 0 || y < 0)
	{
		return true;
	}

	unsigned int aX = x/wallSize;
	unsigned int aY = y/wallSize;

	if (aX < wallmap.size() && aY < wallmap[aX].size())
	{
		return wallmap[aX][aY];
	}
	else
	{
		return true;
	}
}

bool PhysicsEngine::wallAt(int x, int y, int w, int h) const
{
	return wallAt(x, y) || wallAt(x+w, y) || wallAt(x, y+h) || wallAt(x+w, y+h);
}

bool PhysicsEngine::intersectingRectangles(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, bool inclusive) const
{
    if (inclusive)
    {
        return
        (
            (
                (x1 <= x2 && x1 + w1 >= x2)
             ||
                (x2 <= x1 && x2 + w2 >= x1)
            )
         &&
            (
                (y1 <= y2 && y1 + h1 >= y2)
             ||
                (y2 <= y1 && y2 + h2 >= y1)
            )
        );
    }
    else
    {
        return
        (
            (
                (x1 < x2 && x1 + w1 > x2)
             ||
                (x2 < x1 && x2 + w2 > x1)
             ||
                (x1 == x2)
            )
         &&
            (
                (y1 < y2 && y1 + h1 > y2)
             ||
                (y2 < y1 && y2 + h2 > y1)
             ||
                (y1 == y2)
            )
        );
    }
}
} //namespace hg
