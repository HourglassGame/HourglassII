#include "PhysicsEngineUtilities.h"
#include <boost/range/algorithm_ext/push_back.hpp>
#include "RectangleGlitz.h"
#include "multi_thread_allocator.h"
namespace hg {

void buildDepartures(
    mt::std::vector<ObjectAndTime<Box, Frame *> >::type const &nextBox,
    mt::std::vector<ObjectAndTime<Guy, Frame *> >::type const &nextGuy,
    PhysicsEngine::FrameDepartureT &newDepartures,
    Frame *frame)
{
    (void)frame;
    buildDeparturesForComplexEntities(nextBox, newDepartures);
    buildDeparturesForComplexEntities(nextGuy, newDepartures);
}

void makeBoxGlitzListForNormalDepartures(
    mt::std::vector<ObjectAndTime<Box, Frame *> >::type const &nextBox,
	mt::std::vector<char>::type &nextBoxNormalDeparture,
	BoxGlitzAdder const &boxGlitzAdder)	
{
	for (std::size_t j(0), jsize(nextBox.size()); j < jsize; ++j)
	{
        if (nextBoxNormalDeparture[j]) {
            boxGlitzAdder.addGlitzForBox(
                vector2<int>(nextBox[j].object.getX(), nextBox[j].object.getY()),
                nextBox[j].object.getSize(), nextBox[j].object.getTimeDirection());
        }
	}
}

struct Collidables
{
    Collidables(
        Wall const *nwalls,
        mt::std::vector<Collision>::type const *nplatforms,
        mt::std::vector<ObjectAndTime<Box, Frame *> >::type const *nboxes) :
        walls(nwalls), platforms(nplatforms), boxes(nboxes) {}

    Wall const *walls;
    mt::std::vector<Collision>::type const *platforms;
    mt::std::vector<ObjectAndTime<Box, Frame *> >::type const *boxes;
};

bool explodeBoxes(
    mt::std::vector<int>::type &pos,
    mt::std::vector<int>::type const &size,
    mt::std::vector<mt::std::vector<std::size_t>::type >::type const &links,
    mt::std::vector<char>::type &toBeSquished,
    mt::std::vector<std::pair<bool, int> >::type const &bound,
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
    mt::std::vector<int>::type &x,
    mt::std::vector<int>::type const &xTemp,
    mt::std::vector<int>::type &y,
    mt::std::vector<int>::type const &size,
    mt::std::vector<mt::std::vector<std::size_t>::type >::type const &links,
    bool firstTime,
    mt::std::vector<char>::type &toBeSquished,
    mt::std::vector<std::pair<bool, int> >::type const &bound,
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
    mt::std::vector<int>::type &majorAxis,
    mt::std::vector<int>::type const &minorAxis,
    mt::std::vector<int>::type const &size,
    mt::std::vector<char>::type const &squished,
    mt::std::vector<std::size_t>::type &boxesSoFar,
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
bool wallAtInclusive(Environment const &env, int x, int y, int w, int h)
{
    return env.wall.at(x, y) || env.wall.at(x+w, y) || env.wall.at(x, y+h) || env.wall.at(x+w, y+h);
}
#endif
bool wallAtExclusive(Wall const &wall, int x, int y, int w, int h)
{
    return wall.at(x+1, y+1) || wall.at(x+w-1, y+1) || wall.at(x+1, y+h-1) || wall.at(x+w-1, y+h-1);
}
bool wallAtExclusive(Environment const &env, int x, int y, int w, int h)
{
    return wallAtExclusive(env.wall, x, y, w, h);
}

bool getRaytraceRectangleCollision(
	int sx, int sy, 
	int &px, int &py,
	int left, int top,
	int width, int height,
	int dx, int dy,
	bool mostlySideways)
{
	int right = left + width;
	int bottom = top + height;

	if (PointInRectangleExclusive(sx, sy, left, top, width, height))
	{
		px = sx;
		py = sy;
	}

	if (px != sx || py != sy)
	{
		if (mostlySideways)
		{
			if (dx > 0)
			{
				if (left < px && right > sx)
				{
					if (dy > 0) // even Diagonal, OCTANT 0
					{
						if (bottom > sy)
						{
							if (sy + (left - sx)*dy/dx < bottom && sy + (right - sx)*dy/dx > top) 
							{
								// Test which face was hit
								if (sy + (left - sx)*dy/dx < top)
								{
									int offFaceValue = sx + (top - sy)*dx/dy;
									if (offFaceValue < px)
									{
										py = top;
										px = offFaceValue;
										return true;
									}
								}
								else
								{
									px = left;
									py = sy + (px - sx)*dy/dx;
									return true;
								}
							}
						}
					}
					else // odd Diagonal, OCTANT 7
					{
						if (top < sy)
						{
							if (sy + (right - sx)*dy/dx < bottom && sy + (left - sx)*dy/dx > top) 
							{
								// Test which face was hit
								if (sy + (left - sx)*dy/dx > bottom)
								{
									int offFaceValue = sx + (bottom - sy)*dx/dy;
									if (offFaceValue < px)
									{
										py = bottom;
										px = offFaceValue;
										return true;
									}
								}
								else
								{
									px = left;
									py = sy + (px - sx)*dy/dx;
									return true;
								}
							}
						}
					}
				}
			}
			else // dx < 0
			{
				if (left < sx && right > px)
				{
					if (dy < 0) // even Diagonal, OCTANT 4
					{
						if (top < sy)
						{
							if (sy + (left - sx)*dy/dx < bottom && sy + (right - sx)*dy/dx > top) 
							{
								// Test which face was hit
								if (sy + (right - sx)*dy/dx > bottom)
								{
									int offFaceValue = sx + (bottom - sy)*dx/dy;
									if (offFaceValue > px)
									{
										py = bottom;
										px = offFaceValue;
										return true;
									}
								}
								else
								{
									px = right;
									py = sy + (px - sx)*dy/dx;
									return true;
								}
							}
						}
					}
					else // odd Diagonal, OCTANT 3
					{
						if (bottom > sy)
						{
							if (sy + (right - sx)*dy/dx < bottom && sy + (left - sx)*dy/dx > top) 
							{
								// Test which face was hit
								if (sy + (right - sx)*dy/dx < top)
								{
									int offFaceValue = sx + (top - sy)*dx/dy;
									if (offFaceValue > px)
									{
										py = top;
										px = offFaceValue;
										return true;
									}
								}
								else
								{
									px = right;
									py = sy + (px - sx)*dy/dx;
									return true;
								}
							}
						}
					}
				}
			}
		}
		else // Mostly uppy-downy
		{
			if (dy > 0)
			{
				if (top < py && bottom > sy)
				{
					if (dx > 0) // even Diagonal, OCTANT 1
					{
						if (right > sx)
						{
							if (sx + (top - sy)*dx/dy < right && sx + (bottom - sy)*dx/dy > left) 
							{
								// Test which face was hit
								if (sx + (top - sy)*dx/dy < left)
								{
									int offFaceValue = sy + (left - sx)*dy/dx;
									if (offFaceValue < py)
									{
										px = left;
										py = offFaceValue;
										return true;
									}
								}
								else
								{
									py = top;
									px = sx + (py - sy)*dx/dy;
									return true;
								}
							}
						}
					}
					else // odd Diagonal, OCTANT 2
					{
						if (left < sx)
						{
							if (sx + (bottom - sy)*dx/dy < right && sx + (top - sy)*dx/dy > left) 
							{
								// Test which face was hit
								if (sx + (top - sy)*dx/dy > right)
								{
									int offFaceValue = sy + (right - sx)*dy/dx;
									if (offFaceValue < py)
									{
										px = right;
										py = offFaceValue;
										return true;
									}
								}
								else
								{
									py = top;
									px = sx + (py - sy)*dx/dy;
									return true;
								}
							}
						}
					}
				}
			}
			else // dy < 0
			{
				if (top < sy && bottom > py)
				{
					if (dx < 0) // even Diagonal, OCTANT 5
					{
						if (left < sx)
						{
							if (sx + (top - sy)*dx/dy < right && sx + (bottom - sy)*dx/dy > left) 
							{
								// Test which face was hit
								if (sx + (bottom - sy)*dx/dy > right)
								{
									int offFaceValue = sy + (right - sx)*dy/dx;
									if (offFaceValue > py)
									{
										px = right;
										py = offFaceValue;
										return true;
									}
								}
								else
								{
									py = bottom;
									px = sx + (py - sy)*dx/dy;
									return true;
								}
							}
						}
					}
					else // odd Diagonal, OCTANT 6
					{
						if (right > sx)
						{
							if (sx + (bottom - sy)*dx/dy < right && sx + (top - sy)*dx/dy > left) 
							{
								// Test which face was hit
								if (sx + (bottom - sy)*dx/dy < left)
								{
									int offFaceValue = sy + (left - sx)*dy/dx;
									if (offFaceValue > py)
									{
										px = left;
										py = offFaceValue;
										return true;
									}
								}
								else
								{
									py = bottom;
									px = sx + (py - sy)*dx/dy;
									return true;
								}
							}
						}
					}
				}
			}
		}
	}
	return false;
}

void doGunRaytrace(
		PhysicsObjectType &targetType,
		int &targetId,
		Environment const &env,
		int &sx, int &sy, int &px, int &py,
		mt::std::vector<Collision>::type const &nextPlatform,
		mt::std::vector<ObjectAndTime<Box, Frame *> >::type nextBox,
		mt::std::vector<char>::type &nextBoxNormalDeparture,
		mt::std::vector<int>::type gx, // other guy things
		mt::std::vector<int>::type gy,
		mt::std::vector<int>::type gw,
		mt::std::vector<int>::type gh,
		mt::std::vector<char>::type shootable)
{
	// source x,y
	// point  x,y

	// starts in a wall all it can hit is a wall
	if (env.wall.at(sx,sy))
	{
		px = sx;
		py = sy;
		targetType = NONE;
		targetId = -1;
		return;
	}

	// prevent zero length vectors
	if (sx == px && sy == py)
	{
		px = sx + 1;
	}

	int size = env.wall.segmentSize();
	
	// ***  Find Wall ***
	int dx = px - sx;
	int dy = py - sy;

	int absDx = std::abs(dx);
	int absDy = std::abs(dy);
	
	bool mostlySideways = (absDx > absDy);
	
	if (mostlySideways)
	{
		if (dx > 0) // Right
		{
			if (dy > 0) // Down Right
			{
				int dist = (sx/size + 1)*size - sx;
				int nx, ny;
				
				while (true)
				{
					nx = sx + dist;
					ny = sy + dy*dist/dx;
					
					if (env.wall.at(nx-1,ny))
					{
						py = (ny/size)*size;
						px = sx + (py - sy)*dx/dy;
						break;
					}
					if (env.wall.at(nx,ny))
					{
						py = ny;
						px = nx;
						break;
					}
					dist = dist + size;
				}
			}
			else // Up Right
			{
				int dist = (sx/size + 1)*size - sx;
				int nx, ny;
				
				while (true)
				{
					nx = sx + dist;
					ny = sy + dy*dist/dx;
					
					if (env.wall.at(nx-1,ny))
					{
						py = (ny/size + 1)*size;
						px = sx + (py - sy)*dx/dy;
						break;
					}
					if (env.wall.at(nx,ny))
					{
						py = ny;
						px = nx;
						break;
					}
					dist = dist + size;
				}
			}
		}
		else // Left
		{
			if (dy > 0) // Down Left
			{
				int dist = (sx/size)*size - sx;
				int nx, ny;
				
				while (true)
				{
					nx = sx + dist;
					ny = sy + dy*dist/dx;
					
					if (env.wall.at(nx,ny))
					{
						py = (ny/size)*size;
						px = sx + (py - sy)*dx/dy;
						break;
					}
					if (env.wall.at(nx-1,ny))
					{
						py = ny;
						px = nx;
						break;
					}
					dist = dist - size;
				}
			}
			else // Up Left
			{
				int dist = (sx/size)*size - sx;
				int nx, ny;
				
				while (true)
				{
					nx = sx + dist;
					ny = sy + dy*dist/dx;
					
					if (env.wall.at(nx,ny))
					{
						py = (ny/size + 1)*size;
						px = sx + (py - sy)*dx/dy;
						break;
					}
					if (env.wall.at(nx-1,ny))
					{
						py = ny;
						px = nx;
						break;
					}
					dist = dist - size;
				}
			}
		}
	}
	else // Mostly Up or Down
	{
		if (dy > 0) // Down
		{
			if (dx > 0) // Down Right
			{
				int dist = (sy/size + 1)*size - sy;
				int nx, ny;
				
				while (true)
				{
					ny = sy + dist;
					nx = sx + dx*dist/dy;
					
					if (env.wall.at(nx,ny-1))
					{
						px = (nx/size)*size;
						py = sy + (px - sx)*dy/dx;
						break;
					}
					if (env.wall.at(nx,ny))
					{
						py = ny;
						px = nx;
						break;
					}
					dist = dist + size;
				}
			}
			else // Down Left
			{
				int dist = (sy/size + 1)*size - sy;
				int nx, ny;
				
				while (true)
				{
					ny = sy + dist;
					nx = sx + dx*dist/dy;
					
					if (env.wall.at(nx,ny-1))
					{
						px = (nx/size + 1)*size;
						py = sy + (px - sx)*dy/dx;
						break;
					}
					if (env.wall.at(nx,ny))
					{
						py = ny;
						px = nx;
						break;
					}
					dist = dist + size;
				}
			}
		}
		else // Up
		{
			if (dy > 0) // Up Right
			{
				int dist = (sy/size)*size - sy;
				int nx, ny;
				
				while (true)
				{
					ny = sy + dist;
					nx = sx + dx*dist/dy;
					
					if (env.wall.at(nx,ny))
					{
						px = (nx/size)*size;
						py = sy + (px - sx)*dy/dx;
						break;
					}
					if (env.wall.at(nx,ny-1))
					{
						py = ny;
						px = nx;
						break;
					}
					dist = dist - size;
				}
			}
			else // Up Left
			{
				int dist = (sy/size)*size - sy;
				int nx, ny;
				
				while (true)
				{
					ny = sy + dist;
					nx = sx + dx*dist/dy;
					
					if (env.wall.at(nx,ny))
					{
						px = (nx/size + 1)*size;
						py = sy + (px - sx)*dy/dx;
						break;
					}
					if (env.wall.at(nx,ny-1))
					{
						py = ny;
						px = nx;
						break;
					}
					dist = dist - size;
				}
			}
		}
	}
	
	// *** Find Object Collisions ***
	
	// Platforms
	for (std::size_t i(0), isize(nextPlatform.size()); i < isize; ++i)
	{
		getRaytraceRectangleCollision(
			sx, sy, px, py,
			nextPlatform[i].getX(), nextPlatform[i].getY(), 
			nextPlatform[i].getWidth(), nextPlatform[i].getHeight(),
			dx, dy, mostlySideways);
	}

	// Box
	for (std::size_t i(0), isize(nextBox.size()); i < isize; ++i)
	{
		if (nextBoxNormalDeparture[i])
		{
			if (getRaytraceRectangleCollision(
				sx, sy, px, py,
				nextBox[i].object.getX(), nextBox[i].object.getY(), 
				nextBox[i].object.getSize(), nextBox[i].object.getSize(),
				dx, dy, mostlySideways))
			{
				targetType = BOX;
				targetId = i;
			}
		}
	}
	
	// Guy
	for (std::size_t i(0), isize(boost::distance(gx)); i < isize; ++i)
	{
		if (shootable[i])
		{
			if (getRaytraceRectangleCollision(
				sx, sy, px, py,
				gx[i], gy[i], 
				gw[i], gh[i],
				dx, dy, mostlySideways))
			{
				targetType = GUY;
				targetId = i;
			}
		}
	}
	
}

int RectangleIntersectionDirection(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
	// Returns the deepest edge of rectangle 2 that rectangle 1 is in.
	// Does not assume intersection.
	if (x1 + w1/2 < x2 + w2/2) // Left
	{
		if (y1 + h1/2 < y2 + h2/2) // Top Left
		{
			// y = m(x - x') + y'
			// m = 1
			if ((y1 + h1) < ((x1 + w1) - (x2)) + (y2))
			{
				return 1; // TOP
			}
			else
			{
				return 2; // LEFT
			}
		}
		else // Bottom Left
		{
			if ((y1) < -((x1 + w1) - (x2)) + (y2 + h2))
			{
				return 2; // LEFT
			}
			else
			{
				return 3; // BOTTOM
			}
		}
	}
	else // Right
	{
		if (y1 + h1/2 < y2 + h2/2) // Top Right
		{
			if ((y1 + h1) < -((x1) - (x2 + w2)) + (y2))
			{
				return 1; // TOP
			}
			else
			{
				return 0; // RIGHT
			}
		}
		else // Bottom Right
		{
			if ((y1) < ((x1) - (x2 + w2)) + (y2 + h2))
			{
				return 0; // RIGHT
			}
			else
			{
				return 3; // BOTTOM
			}
		}
	}
}

bool PointInRectangleInclusive(int px, int py, int x, int y, int w, int h)
{
    return
        (px <= x + w && px >= x)
        &&
        (py <= y + h && py >= y)
        ;
}

bool PointInRectangleExclusive(int px, int py, int x, int y, int w, int h)
{
    return
        (px < x + w && px > x)
        &&
        (py < y + h && py > y)
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
		else if  (y < y1 + h - w/2) // middle
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
