#include "PhysicsEngineUtilities.h"
#include <boost/range/algorithm_ext/push_back.hpp>
#include "RectangleGlitz.h"
#include "multi_thread_allocator.h"
namespace hg {

void buildDepartures(
    mt::std::vector<ObjectAndTime<Box, Frame*> >::type const& nextBox,
    mt::std::vector<ObjectAndTime<Guy, Frame*> >::type const& nextGuy,
    PhysicsEngine::FrameDepartureT& newDepartures,
    Frame* frame)
{
    (void)frame;
    buildDeparturesForComplexEntities(nextBox, newDepartures);
    buildDeparturesForComplexEntities(nextGuy, newDepartures);
}


template<typename T>
    void pushBidirectional(
        T const& obj,
        unsigned forwardsColour, unsigned reverseColour,
        mt::std::vector<Glitz>::type& forwardsGlitz,
        mt::std::vector<Glitz>::type& reverseGlitz)
{
    Glitz sameDirectionGlitz(
        multi_thread_new<RectangleGlitz>(
            obj.getX(), obj.getY(),
            obj.getWidth(), obj.getHeight(),
            forwardsColour));

    Glitz oppositeDirectionGlitz(
        multi_thread_new<RectangleGlitz>(
            obj.getX() - obj.getXspeed(), obj.getY() - obj.getYspeed(),
            obj.getWidth(), obj.getHeight(),
            reverseColour));

    forwardsGlitz.push_back(
        obj.getTimeDirection() == FORWARDS ?
        sameDirectionGlitz:oppositeDirectionGlitz);

    reverseGlitz.push_back(
        obj.getTimeDirection() == REVERSE ?
        sameDirectionGlitz:oppositeDirectionGlitz);
}

FrameView makeFrameView(
    mt::std::vector<GuyOutputInfo>::type const& guyInfo,
    mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type const& backgroundTriggerGlitz,
    mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type const& foregroundTriggerGlitz,
    mt::std::vector<Glitz>::type const& forwardsGlitz,
    mt::std::vector<Glitz>::type const& reverseGlitz)
{
    mt::std::vector<Glitz>::type finalForwardsGlitz;
    mt::std::vector<Glitz>::type finalReverseGlitz;

    foreach (RetardedNotActuallyAGlitzGlitz const& glitz, backgroundTriggerGlitz) {
        pushBidirectional(glitz, glitz.getForwardsColour(), glitz.getReverseColour(), finalForwardsGlitz, finalReverseGlitz);
    }

    boost::push_back(finalForwardsGlitz, forwardsGlitz);
    boost::push_back(finalReverseGlitz, reverseGlitz);

    foreach (RetardedNotActuallyAGlitzGlitz const& glitz, foregroundTriggerGlitz) {
        pushBidirectional(glitz, glitz.getForwardsColour(), glitz.getReverseColour(), finalForwardsGlitz, finalReverseGlitz);
    }

    return FrameView(finalForwardsGlitz, finalReverseGlitz, guyInfo);
}

struct Collidables
{
    Collidables(
        Wall const* nwalls,
        mt::std::vector<Collision>::type const* nplatforms,
        mt::std::vector<ObjectAndTime<Box, Frame*> >::type const* nboxes) :
        walls(nwalls), platforms(nplatforms), boxes(nboxes) {}

    Wall const* walls;
    mt::std::vector<Collision>::type const* platforms;
    mt::std::vector<ObjectAndTime<Box, Frame*> >::type const* boxes;
};

bool explodeBoxes(
    mt::std::vector<int>::type& pos,
    mt::std::vector<int>::type const& size,
    mt::std::vector<mt::std::vector<std::size_t>::type >::type const& links,
    mt::std::vector<char>::type& toBeSquished,
    mt::std::vector<std::pair<bool, int> >::type const& bound,
    std::size_t index,
    int boundSoFar,
    int sign)
{
	// sign = 1, small to large (left to right / top to bottom)
	// sign = -1, large to small (right to left / bottom to top)
	pos[index] = boundSoFar;
	//std::cerr << "Exploding " << index << ": " << boundSoFar << ", " << size[index] << ", " << sign << "\n";

	bool subSquished = false;

	for (std::size_t i(0), isize(links[index].size()); i < isize; ++i)
	{
		//std::cerr << "New Bound " << boundSoFar + ((sign == 1) ? size[index] : size[i]) * sign << ", " << ((sign == 1) ? size[index] : size[links[index][i]]) <<  "\n";
		subSquished =
            explodeBoxes(
                pos,
                size,
                links,
                toBeSquished,
                bound,
                links[index][i],
                boundSoFar + ((sign == 1) ? size[index] : size[links[index][i]]) * sign,
                sign)
            || subSquished;
	}

	if (subSquished || (bound[index].first && bound[index].second * sign <= boundSoFar * sign))
	{
		toBeSquished[index] = true;
		return true;
    }
	return false;
}

bool explodeBoxesUpwards(
    mt::std::vector<int>::type& x,
    mt::std::vector<int>::type const& xTemp,
    mt::std::vector<int>::type& y,
    mt::std::vector<int>::type const& size,
    mt::std::vector<mt::std::vector<std::size_t>::type >::type const& links,
    bool firstTime,
    mt::std::vector<char>::type& toBeSquished,
    mt::std::vector<std::pair<bool, int> >::type const& bound,
    std::size_t index,
    int boundSoFar)
{
	//int sign = -1;
	y[index] = boundSoFar;

	bool subSquished = false;

	for (unsigned int i = 0; i < links[index].size(); ++i)
	{
		if (firstTime || x[index] != xTemp[index])
		{
            // boxes sitting on this one
			x[links[index][i]] = xTemp[links[index][i]] + x[index] - xTemp[index];
		}
		subSquished =
            explodeBoxesUpwards(
                x,
                xTemp,
                y,
                size,
                links,
                firstTime,
                toBeSquished,
                bound,
                links[index][i],
                boundSoFar - size[links[index][i]])
            || subSquished;
	}

	if (subSquished || (bound[index].first && bound[index].second >= boundSoFar))
	{
		toBeSquished[index] = true;
		return true;
	}
	return false;
}
void recursiveBoxCollision(
    mt::std::vector<int>::type& majorAxis,
    mt::std::vector<int>::type const& minorAxis,
    mt::std::vector<int>::type const& size,
    mt::std::vector<char>::type const& squished,
    mt::std::vector<std::size_t>::type& boxesSoFar,
    std::size_t index,
    int subtractionNumber) // horizontal wins a tie
{
	boxesSoFar.push_back(index);

	for (std::size_t i(0), isize(majorAxis.size()); i < isize; ++i)
	{
		if (i != index && !squished[i] &&
			IntersectingRectanglesExclusive(
                majorAxis[index], minorAxis[index], size[index], size[index],
                majorAxis[i], minorAxis[i], size[i], size[i]) &&
			std::abs(majorAxis[index] - majorAxis[i]) > std::abs(minorAxis[index] - minorAxis[i]) - subtractionNumber)
		{
			//std::cerr << "Collide link " << majorAxis[index] << ", " << size[index] << ", " << majorAxis[i] << ", " << size[i] << "\n";
			int overlap = -(majorAxis[index] + size[index] - majorAxis[i]); // index must move UP
			if (majorAxis[i] < majorAxis[index])
			{
				overlap = majorAxis[i] + size[i] - majorAxis[index];  // index must move DOWN
			}

			int indexMovement = overlap/(static_cast<int>(boxesSoFar.size()) + 1);
			int iMovement = -overlap + indexMovement;
			for (std::size_t j(0), jsize(boxesSoFar.size()); j < jsize; ++j)
			{
				majorAxis[boxesSoFar[j]] = majorAxis[boxesSoFar[j]] + indexMovement;
			}
			majorAxis[i] = majorAxis[i] + iMovement;
			recursiveBoxCollision(majorAxis, minorAxis, size, squished, boxesSoFar, i, subtractionNumber);
		}
	}
}
#if 0
bool wallAtInclusive(Environment const& env, int x, int y, int w, int h)
{
    return env.wall.at(x, y) || env.wall.at(x+w, y) || env.wall.at(x, y+h) || env.wall.at(x+w, y+h);
}
#endif
bool wallAtExclusive(Wall const& wall, int x, int y, int w, int h)
{
    return wall.at(x+1, y+1) || wall.at(x+w-1, y+1) || wall.at(x+1, y+h-1) || wall.at(x+w-1, y+h-1);
}
bool wallAtExclusive(Environment const& env, int x, int y, int w, int h)
{
    return wallAtExclusive(env.wall, x, y, w, h);
}

#if 0
unsigned int Raytrace(
		Environment const& env,
		int sx, int sy, int px, int py, // start and position of target for trace
		mt::std::vector<ObjectAndTime<Box, Frame*> >::type box,
		mt::std::vector<char>::type& nextBoxNormalDeparture,
		mt::std::vector<int>::type gx, // other guy things
		mt::std::vector<int>::type gy,
		mt::std::vector<int>::type gw,
		mt::std::vector<int>::type gh,
		mt::std::vector<char>::type shootable,
		int myIndex) // don't hit myself
{
	// starts in a wall all it can hit is a wall
	if (env.wall.at(sx,sy))
	{
		return -1;
	}

	// shoot to the right if the direction has no length
	if (sx == px && sy == py)
	{
		px = sx + 1;
	}

	int size = env.wall.segmentSize();

	if (sx < px && sy >= py) // top right, x safe
	{
		// find wall distance
		int tx = sx;
		int ty = sy;
		while (true)
		{
			int hDis = tx%size + size - sx;
			int vDis = (ty-1)%size - sy;
			if (vDis <= hDis * (py - sy)/(px - sx)) // both sides are non-positive
			{
				// next grid square on Right
				tx = tx%size + size;
				ty = tx * (py - sy)/(px - sx) + sy;
				if (env.wall.at(tx,ty))
				{
					break;
				}
			}
			else
			{
				// next grid square on Top
				ty = (ty-1)%size;
				tx = ty * (px - sx)/(py - sy) + sx; // it is known (py - sy) != 0
				if (env.wall.at(tx,ty))
				{
					break;
				}
			}
		}
		int wallDistance = tx - sx;

		// find all the boxes within wallDistance and on line
		for (std::size_t j(0), jsize(box.size()); j < jsize; ++j)
		{
			// only shoot normal departure boxes
			if (nextBoxNormalDeparture[j])
			{

			}
		}

	}
	else if (sx >= px && sy > py) // top left, y safe
	{

	}
	else if (sx > px && sy <= py) // bottom left, x safe
	{

	}
	else // (sx <= px && sy < py) // bottom right, y safe
	{
		assert(sx <= px && sy < py);

	}
}
#endif

bool PointInRectangleInclusive(int px, int py, int x, int y, int w, int h)
{
    return
        (px <= x + w && px >= x)
        &&
        (py <= y + h && py >= y)
        ;
}

bool IsPointInVerticalQuadrant(int x, int y, int x1, int y1, int w, int h)
{
	if (w > h)
	{
		if (x < x1 + h/2) // left
		{
			return std::abs(x - (x1 + h/2)) < std::abs(y - (y1 + h/2));
		}
		else if  (x < x1 + w - h/2) // middle
		{
			return true;
		}
		else // right
		{
			return std::abs(x - (x1 + w - h/2)) < std::abs(y - (y1 + h/2));
		}
	}
	else
	{
		if (y < y1 + w/2) // top
		{
			return std::abs(x - (x1 + w/2)) < std::abs(y - (y1 + w/2));
		}
		else if  (x < x1 + w - h/2) // middle
		{
			return false;
		}
		else // bottom
		{
			return std::abs(x - (x1 + w/2)) < std::abs(y - (y1 + h - w/2));
		}
	}
}

bool IntersectingRectanglesInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return
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
	);
}

bool IntersectingRectanglesExclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return
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
	);
}


bool IntersectingRectanglesInclusiveCollisionOverlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, int buffer)
{
	if (buffer < HALF_COLLISION_BUFFER_RANGE)
	{
		return IntersectingRectanglesInclusive(
			x1 + w1 * buffer/COLLISION_BUFFER_RANGE,
			y1 + h1 * buffer/COLLISION_BUFFER_RANGE,
			w1 - w1 * buffer/HALF_COLLISION_BUFFER_RANGE,
			h1 - h1 * buffer/HALF_COLLISION_BUFFER_RANGE,
			x2, y2, w2, h2
		);
	}
	else if (buffer == HALF_COLLISION_BUFFER_RANGE)
	{
		return PointInRectangleInclusive(x1 + w1/2, y1 + h1/2, x2, y2, w2, h2);
	}
	else //(buffer > HALF_COLLISION_BUFFER_RANGE)
	{
		// Yes h1 and w1 are negative
		// the intersection algorithms don't care and it's faster not to prevent negativity
		return RectangleWithinInclusive(
			x1 + w1 * buffer/COLLISION_BUFFER_RANGE,
			y1 + h1 * buffer/COLLISION_BUFFER_RANGE,
			w1 - w1 * buffer/HALF_COLLISION_BUFFER_RANGE,
			h1 - h1 * buffer/HALF_COLLISION_BUFFER_RANGE,
			x2, y2, w2, h2
		);
	}
}

bool RectangleWithinInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return
		(x1 >= x2 && x1 + w1 <= x2 + w2)
		&&
		(y1 >= y2 && y1 + h1 <= y2 + h2);
}


}//namespace hg
