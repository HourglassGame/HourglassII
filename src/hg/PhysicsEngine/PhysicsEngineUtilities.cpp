#include "PhysicsEngineUtilities.h"
#include <boost/range/algorithm_ext/push_back.hpp>
#include "hg/TimeEngine/Glitz/RectangleGlitz.h"
#include "hg/TimeEngine/Glitz/CircleGlitz.h"
#include "hg/Util/multi_thread_allocator.h"
#include "hg/mp/std/vector"
#include <tuple>
namespace hg {

void buildDepartures(
	mp::std::vector<ObjectAndTime<Box, Frame *>> const &nextBox,
	mp::std::vector<ObjectAndTime<Explosion, Frame*> > const &nextExplosion,
	mp::std::vector<ObjectAndTime<Guy, Frame *>> const &nextGuy,
	mt::std::vector<std::tuple<std::size_t, Frame *>> &guyDepartureFrames,
	PhysicsEngine::FrameDepartureT &newDepartures,
	Frame *frame)
{
	(void)frame;
	buildDeparturesForComplexEntities(nextBox, newDepartures);
	buildDeparturesForComplexEntities(nextExplosion, newDepartures);
	buildDeparturesForComplexEntitiesWithIndexCaching(nextGuy, guyDepartureFrames, newDepartures);
}

void makeBoxGlitzListForNormalDepartures(
	mp::std::vector<ObjectAndTime<Box, Frame *>> const &nextBox,
	mp::std::vector<char> &nextBoxNormalDeparture,
	BoxGlitzAdder const &boxGlitzAdder)
{
	for (std::size_t j(0), jsize(nextBox.size()); j < jsize; ++j)
	{
		if (nextBoxNormalDeparture[j]) {
			boxGlitzAdder.addGlitzForBox(
				vec2<int>{
					nextBox[j].object.getX() - nextBox[j].object.getXspeed(), 
					nextBox[j].object.getY() - nextBox[j].object.getYspeed()},
				nextBox[j].object.getWidth(), nextBox[j].object.getHeight(),
				nextBox[j].object.getBoxType(), nextBox[j].object.getState(),
				nextBox[j].object.getTimeDirection());
		}
	}
}

struct Collidables final
{
	Collidables(
		Wall const *nwalls,
		mt::std::vector<Collision> const *nplatforms,
		mt::std::vector<ObjectAndTime<Box, Frame *>> const *nboxes) :
		walls(nwalls), platforms(nplatforms), boxes(nboxes) {}

	Wall const *walls;
	mt::std::vector<Collision> const *platforms;
	mt::std::vector<ObjectAndTime<Box, Frame *>> const *boxes;
};

bool explodeBoxes(
	mp::std::vector<int> &pos,
	mp::std::vector<int> const &size,
	mp::std::vector<mp::std::vector<std::size_t>> const &links,
	mp::std::vector<char> &toBeSquished,
	mp::std::vector<std::pair<bool, int>> const &bound,
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

	if (subSquished || (bound[index].first && bound[index].second * sign < boundSoFar * sign))
	{
		toBeSquished[index] = true;
		return true;
	}
	return false;
}

bool explodeBoxesUpwards(
	mp::std::vector<int> &x,
	mp::std::vector<int> const &xTemp,
	mp::std::vector<int> &y,
	mp::std::vector<int> const &size,
	mp::std::vector<mp::std::vector<std::size_t>> const &links,
	bool firstTime,
	mp::std::vector<char> &toBeSquished,
	mp::std::vector<std::pair<bool, int>> const &bound,
	std::size_t index,
	int boundSoFar)
{
	//int sign = -1;
	y[index] = boundSoFar;
	//std::cerr << "Exploding Up " << index << ": " << boundSoFar << ", " << size[index] << "\n";

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

	if (subSquished || (bound[index].first && bound[index].second > boundSoFar))
	{
		toBeSquished[index] = true;
		return true;
	}
	return false;
}
void recursiveBoxCollision(
	mp::std::vector<int> &majorAxis,
	mp::std::vector<int> const &minorAxis,
	mp::std::vector<int> const &majorSize,
	mp::std::vector<int> const &minorSize,
	mp::std::vector<char> const &squished,
	mp::std::vector<BoxType> const& boxType,
	mp::std::vector<std::size_t> &boxesSoFar,
	std::size_t index,
	bool winTies, // horizontal wins a tie
	TimeDirection const boxDirection,
	mp::std::vector<Box> const &oldBoxList)
{
	boxesSoFar.push_back(index);

	for (std::size_t i(0), isize(majorAxis.size()); i < isize; ++i)
	{
		if (i != index && !squished[i] && boxCollidable(boxType[i])
			&& oldBoxList[i].getTimeDirection() == boxDirection
			&& IntersectingRectanglesExclusive(
				majorAxis[index], minorAxis[index], majorSize[index], minorSize[index],
				majorAxis[i], minorAxis[i], majorSize[i], minorSize[i])
			// Find vertical collision with the major axis as the vertical component. winTies is used to make one of the axies win ties.
			&& IsRectangleRelationVertical(
				minorAxis[index], majorAxis[index], minorSize[index], majorSize[index],
				minorAxis[i], majorAxis[i], minorSize[i], majorSize[i], winTies))
		{
			//std::cerr << "Collide link " << majorAxis[index] << ", " << size[index] << ", " << majorAxis[i] << ", " << size[i] << "\n";
			int overlap = -(majorAxis[index] + majorSize[index] - majorAxis[i]); // index must move UP
			if (majorAxis[i] < majorAxis[index])
			{
				overlap = majorAxis[i] + majorSize[i] - majorAxis[index];  // index must move DOWN
			}

			int indexMovement = overlap/(static_cast<int>(boxesSoFar.size()) + 1);
			int iMovement = -overlap + indexMovement;
			for (std::size_t j(0), jsize(boxesSoFar.size()); j < jsize; ++j)
			{
				majorAxis[boxesSoFar[j]] = majorAxis[boxesSoFar[j]] + indexMovement;
			}
			majorAxis[i] = majorAxis[i] + iMovement;
			recursiveBoxCollision(majorAxis, minorAxis, majorSize, minorSize, squished, boxType, boxesSoFar, i, winTies, boxDirection, oldBoxList);
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
	return wall.at(x, y) || wall.at(x+w-1, y) || wall.at(x, y+h-1) || wall.at(x+w-1, y+h-1);
}
bool wallAtExclusive(Environment const &env, int x, int y, int w, int h)
{
	return wallAtExclusive(env.wall, x, y, w, h);
}

struct RayToLineCollisionResult{
	bool hit;
	int aPos;
};

RayToLineCollisionResult getNormalisedRayToLineCollision(
	int const sa, int const sb, int const aima, int const aimb,
	int const lineStarta, int const lineEnda, int const linePosb,
	//If true, the rectangle that the line is an edge of extends in the positive b direction;
	//If false, the rectangle extends in the negative b direction.
	//(I'm pretty sure that due to normalisation/preprocessing, lineBisSmallB is only relevant in cases where sb == linePosb)
	//(Perhaps the logic for the case where sb == linePosb should be moved from getNormalisedRayToLineCollision to 
	// getRayToLineCollision (or even further up the call tree), so that this parameter is no longer needed?)
	bool const lineBisSmallB,
	//-1 or 1; decides if the aPos of the return value should be computed rounding up (-1) or rounding down (1).
	int const aRound)
{
	//Should be normalised and preprocessed such that
	assert(sa <= aima);
	assert(lineStarta <= lineEnda);
	assert(sb <= aimb);
	assert(aRound == 1 || aRound == -1);
	assert(sa != aima || sb != aimb);
	//i.e. increasing a and b for ray, increasing a for line.

	//TODO: Take advantage of this construction of rayAatLine (the sb == aimb special case); or eliminate it?
	int const rayAatLine =
		sb == aimb ?
		 std::max(sa, lineStarta) :
		 sa + (aRound*flooredDivision(aRound*(linePosb - sb)*(aima - sa), (aimb - sb)));
	
	bool const shouldHit =
		//sb <= aimb, so ray can't hit line segment unless sb <= linePosb
		(sb <= linePosb) &&
	  (
		//For perfectly vertical rays, only hit if the b position matches the b position of the line.
		//Also check the vertical location; since rays that start below the line segment can't hit the line segment
		//(since sa < aima in this case; so the ray must be aiming downwards)
		(sb == aimb) ?
			sb == linePosb && sa < lineEnda
		//For rays that start on the b position of the line but which are not perfectly vertical; only hit if the
		//ray is aiming into the object whose edge the line corresponds to.
	  : (sb == linePosb) ?
			lineBisSmallB && (lineStarta <= sa && sa < lineEnda)
		//For all other rays, the ray must hit the line
	  : lineStarta <= rayAatLine && rayAatLine <= lineEnda);

	return {
		shouldHit,
		rayAatLine
	};
}

RayToLineCollisionResult getRayToLineCollision(
	int const sa, int const sb, int const aima, int const aimb,
	int const lineStarta, int const lineEnda, int const linePosb,
	bool const axesSwapped,// bool const lineStartaInclusive, bool const lineEndaInclusive, bool const lineBinclusive,
	bool const lineBisSmallB
	)
{
	assert(lineStarta <= lineEnda);

	//The following assert could easily be relaxed (and replaced with 'return {false};'); it is just a check that the overall rectangle algorithm
	//is working as expected.
	assert(lineBisSmallB ? sb <= linePosb : linePosb <= sb);
	/*
	+x +y -> ceil x, floor y
	+x -y -> floor x, floor y
	-x +y -> ceil x, ceil y
	-x -y -> floor x, ceil y
	*/

	//axesSwapped variable is for performing rounding correctly
	//  false => a == x, b == y; this is a top/bottom edge.
	//           When computing hit location in a axis, ceil if (aimb - sb) > 0 else floor
	//  true  => a == y, b == x; this is a left/right edge.
	//           When computing hit location in a axis, floor if (aimb - sb) > 0 else ceil

	int const aFactor = sa <= aima ? 1 : -1;
	int const bFactor = sb <= aimb ? 1 : -1;

	int const aRound = (axesSwapped ? 1 : -1)*bFactor;

	auto const normResult = getNormalisedRayToLineCollision(
		sa*aFactor,
		sb*bFactor,
		aima*aFactor,
		aimb*bFactor,
		aFactor == 1 ? lineStarta : lineEnda*aFactor,
		aFactor == 1 ? lineEnda : lineStarta*aFactor,
		linePosb*bFactor,
		//aFactor == 1 ? lineStartaInclusive : lineEndaInclusive,
		//aFactor == 1 ? lineEndaInclusive : lineStartaInclusive,
		//lineBinclusive,
		(lineBisSmallB ? 1 : -1)*bFactor == 1,
		aRound*aFactor
		);

	return {
		normResult.hit,
		normResult.aPos * aFactor
	};
}

RaytraceRectangleCollisionResult getRaytraceRectangleCollision(
	int const sx, int const sy,
	int const aimx, int const aimy,
	int const left, int const top,
	int const width, int const height
)
{
	//TODO: Update to correctly handle zero-size rectangles?
	//TODO: Update to correctly handle zero-length lines?
	assert(sx != aimx || sy != aimy);


	//TODO: Handle s == aim, so that the 'move the aimpoint to the right' hack is consistent over the different object types...?
	auto const raytraceRectangleResult = [=]() -> RaytraceRectangleCollisionResult {
		int const right = left + width;
		int const bottom = top + height;


		if (PointInRectangleExclusive(sx, sy, left, top, width, height)) {
			return {
				true,
				{sx, sy}
			};
		}
		auto const checkSides = [=] (bool const swapAxes) -> RaytraceRectangleCollisionResult {
			auto maybeSwap = [swapAxes](int& a, int& b) { if (swapAxes) {std::swap(a, b);} };

			int sa = sx; int sb = sy; maybeSwap(sa, sb);
			int aima = aimx; int aimb = aimy; maybeSwap(aima, aimb);
			int smallA = left; int smallB = top; maybeSwap(smallA, smallB);
			int largeA = right; int largeB = bottom; maybeSwap(largeA, largeB);

			//Half-open range for rectangle
			if (sb <= smallB || largeB <= sb) {
				bool const lineBisSmallb = sb <= smallB;
				int const lineB = lineBisSmallb ? smallB : largeB;
				//bool const lineStartaInclusive = lineBisSmallb;
				//bool const lineEndaInclusive = false;
				auto const lineCollision = getRayToLineCollision(sa, sb, aima, aimb, smallA, largeA, lineB, swapAxes, /*lineStartaInclusive, lineEndaInclusive, lineBisSmallb,*/ lineBisSmallb);

				if (lineCollision.hit) {
					return {
						lineCollision.hit,
						{ swapAxes ? lineB : lineCollision.aPos, swapAxes ? lineCollision.aPos : lineB }
					};
				}
			}
			return { false, { 0, 0 } };
		};

		auto const topBotRes = checkSides(false);
		auto const leftRightRes = checkSides(true);

		//TODO: Double check this logic for sanity.
		//      Is it consistent with the rounding logic (round angles down)?
		if (iabs(aimx - sx) > iabs(aimy - sy)) {
			return topBotRes.hit ? topBotRes : leftRightRes;
		}
		else {
			return leftRightRes.hit ? leftRightRes : topBotRes;
		}
	}();
	assert(
		   !raytraceRectangleResult.hit
		|| (raytraceRectangleResult.point == vec2<int>{sx, sy} && sx > left && sx < left+width && sy > top && sy < top + height)
		|| (raytraceRectangleResult.point.a == left || raytraceRectangleResult.point.a == left + width) && top  <= raytraceRectangleResult.point.b && raytraceRectangleResult.point.b <= top + height
		|| (raytraceRectangleResult.point.b == top  || raytraceRectangleResult.point.b == top + height) && left <= raytraceRectangleResult.point.a && raytraceRectangleResult.point.a <= left + width
	);
	return raytraceRectangleResult;
}

class NormalisedWall final {
public:
	NormalisedWall(Wall const &wall, bool const swapAxes, int const xFactor, int const yFactor) noexcept :
		wall(&wall), swapAxes(swapAxes), xFactor(xFactor), yFactor(yFactor)
	{
	}
	int segmentSize() const {
		return wall->segmentSize();
	}
	bool atIndex(int const a, int const b) const {
		auto const x = xFactor * (swapAxes ? b : a) + (xFactor == -1 ? -1 : 0);
		auto const y = yFactor * (swapAxes ? a : b) + (yFactor == -1 ? -1 : 0);
		return wall->atIndex(x, y);
	}

private:
	Wall const * const wall;
	bool const swapAxes;
	int const xFactor;
	int const yFactor;
};

vec2<int> doNormalisedGunWallRaytrace(
	NormalisedWall const &wall,
	int const sa, int const sb,
	int const aima, int const aimb,
	//-1 or 1; decides if the aPos of the return value should be computed rounding up (-1) or rounding down (1).
	int const aRound,
	//-1 or 1; decides if the bPos of the return value should be computed rounding up (-1) or rounding down (1).
	int const bRound
){
	assert(sa < aima);
	assert(sb <= aimb);
	assert(aimb - sb <= aima - sa);
	
	//Should be increasing in the a and b axes;
	//Should be mostly moving in the a axis.
	//(If a==x and b==y, this corresponds to a shallow down-right line;
	// so this will be the naming scheme used for directions later in this function.
	// i.e. Left == small a, Right == large a, Top == small b, bottom == large b, etc.
	//)

	auto const bForA = [=](int const a) {
		assert(aima - sa != 0);
		return sb + (bRound*flooredDivision(bRound*(a - sa)*(aimb - sb),(aima - sa)));
	};
	auto const aForB = [=](int const b) {
		assert(aimb - sb != 0);
		return sa + (aRound*flooredDivision(aRound*(b - sb)*(aima - sa),(aimb - sb)));
	};
	auto const indexOf = [&wall](int const pos) {
		//TODO: Handle negative pos correctly/ensure that normalisation doesn't make negative positions
		return flooredDivision(pos,wall.segmentSize());
	};
	auto const posFromIndex = [&wall](int const pos) {
		//TODO: Handle negative pos correctly/ensure that normalisation doesn't make negative positions
		return pos * wall.segmentSize();
	};
	//General algorithm:
	// Move starting point to top-left of block containing the starting point.
	// Loop;
	//  checking visited points for wall;
	//  moving one point to the right unless the next point to the right is in the next row below,
	//   in which case first move downwards and then right.

	//(It is safe to always move right, since the line must have a shallow gradient by (aimb - sb <= aima - sa), so da/db >= 1)
	//TODO: Need to check that it is still safe for perfectly diagonal angles (it seems kinda unsafe, depending on what we want to do in the edge cases.)

	int currentAIndex = indexOf(sa);
	int currentBIndex = indexOf(sb);
	//TODO: If laser starts right on edge of block and is aiming away from block, don't hit the block...
	//TODO: If laser starts in concave corner of 3 blocks, aiming away from all 3, don't hit any of them...
	//TODO: If laser starts on edge of block and is aimed directly along that edge, hit that edge...
	if (wall.atIndex(currentAIndex, currentBIndex) || (sb == aimb && sb % wall.segmentSize() == 0 && sa % wall.segmentSize() != 0 && wall.atIndex(currentAIndex, currentBIndex-1))) {
		//Source was inside wall (should only happen for laser-portals, never for guy lasers)
		return {sa, sb};
	}
	int topB = posFromIndex(currentBIndex + 1);
	while (true) {
		int const nextA = posFromIndex(currentAIndex+1);
		int const leftBforNextA = bForA(nextA);
		{
			//int const bAbove = posFromIndex(currentBIndex);
			if (posFromIndex(currentBIndex) == leftBforNextA) {
				if (wall.atIndex(currentAIndex, currentBIndex - 1)) {
					return { posFromIndex(currentAIndex), leftBforNextA };
				}
			}
			//Using a while loop here would allow non-shallow gradients to work (I think?)
			if (topB < leftBforNextA) {
				currentBIndex += 1;
				if (wall.atIndex(currentAIndex, currentBIndex)) {
					//Hit top of wall below
					return {aForB(topB), topB};
				}
				topB = posFromIndex(currentBIndex + 1);
			}
			//Perfect diagonal.
			//Check both A+1, B+0 and A+0, B+1, then go to A+1, B+1
			if (topB == leftBforNextA) {
				if (wall.atIndex(currentAIndex+1, currentBIndex)
				 || wall.atIndex(currentAIndex,   currentBIndex+1))
				{
					//Hit exactly on diagonal corner
					return {nextA, topB};
				}

				//Special case; hit the acute edge rather than the right angle edge; to match behaviour of rectangle collision algo.
				//TODO: What about extremely shallow lines
				if (wall.atIndex(currentAIndex+1, currentBIndex+1)) {
					//Hit top of wall below and to right
					return { aForB(topB), topB };
				}

				currentBIndex += 1;
				topB = posFromIndex(currentBIndex + 1);
			}
			
		}
		currentAIndex += 1;
		if (wall.atIndex(currentAIndex, currentBIndex)) {
			//Hit left of wall to the right
			return {nextA, leftBforNextA};
		}
	}
}

vec2<int> doGunWallRaytraceUsingRectangles(
	Wall const &wall,
	int const sx, int const sy,
	int const aimx, int const aimy,
	memory_pool<user_allocator_tbb_alloc> &pool
)
{
	if (sx < 0 || sy < 0 || sx > wall.roomWidth() || sy > wall.roomHeight()) {
		return { sx, sy };
	}

	mp::std::vector<Rect<int>> wallRects(pool);

	wallRects.reserve(((wall.roomWidth() / wall.segmentSize())+2)*((wall.roomHeight() / wall.segmentSize())+2));

	auto const makeRectForIndex = [&wall](int const xIndex, int const yIndex) -> Rect<int> {
		return { xIndex * wall.segmentSize(), yIndex * wall.segmentSize(), wall.segmentSize(), wall.segmentSize() };
	};

	for (int xIndex = -1; xIndex < wall.roomWidth() / wall.segmentSize() + 1; ++xIndex) {
		for (int yIndex = -1; yIndex < wall.roomHeight() / wall.segmentSize() + 1; ++yIndex) {
			if (wall.atIndex(xIndex, yIndex)) {
				wallRects.push_back(makeRectForIndex(xIndex, yIndex));
			}
		}
	}
	bool hasHit{ false };
	vec2<int> hitPoint{ 0,0 };

	for (auto const &rect : wallRects) {
		auto const collision = getRaytraceRectangleCollision(
			sx, sy, aimx, aimy,
			rect.x, rect.y,
			rect.w, rect.h);
		//TODO: Add vector2 functions instead of doing it manually?
		//TODO: Merge this logic with the logic in doGunRaytrace
		//TODO: Properly round this value, rather than just always taking the smaller line.
		if (collision.hit && (!hasHit || (iabs(collision.point.a - sx) < iabs(hitPoint.a - sx) || iabs(collision.point.b - sy) < iabs(hitPoint.b - sy)))) {
			hasHit = true;
			hitPoint = collision.point;
		}
	}
	assert(hasHit);
	return hitPoint;
}

vec2<int> doGunWallRaytrace(
	Wall const &wall,
	int const sx, int const sy,
	int const aimx, int const aimy,
	memory_pool<user_allocator_tbb_alloc> &pool)
{
	/*
	line quadrant->floor / ceil when rounding
		+ x + y->ceil x, floor y
		+ x - y->floor x, floor y
		- x + y->ceil x, ceil y
		- x - y->floor x, ceil y
	*/
	//TODO: Reduce number of operations here?
	//If compiler doesn't already do so?
	bool const swapAxes = iabs(aimx - sx) < iabs(aimy - sy);
	int const xFactor = sx <= aimx ? 1 : -1;
	int const yFactor = sy <= aimy ? 1 : -1;

	//+ x + y -> xFactor =  1, yFactor =  1 -> xRound = -1, yRound =  1
	//+ x - y -> xFactor =  1, yFactor = -1 -> xRound =  1, yRound =  1
	//- x + y -> xFactor = -1, yFactor =  1 -> xRound = -1, yRound = -1
	//- x - y -> xFactor = -1, yFactor = -1 -> xRound =  1, yRound = -1

	int const xRound = -yFactor;
	int const yRound =  xFactor;

	int const aRound = swapAxes ? yRound * yFactor : xRound * xFactor;
	int const bRound = swapAxes ? xRound * xFactor : yRound * yFactor;

	auto const normResult = doNormalisedGunWallRaytrace(
		NormalisedWall(wall, swapAxes, xFactor, yFactor),
		swapAxes ? yFactor * sy : xFactor * sx, swapAxes ? xFactor * sx : yFactor * sy,
		swapAxes ? yFactor * aimy : xFactor * aimx, swapAxes ? xFactor * aimx : yFactor * aimy,
		aRound,
		bRound
	);
	auto const rVal = vec2<int>{
		(swapAxes ? normResult.b : normResult.a)*xFactor,
		(swapAxes ? normResult.a : normResult.b)*yFactor
	};
	//Assert that the x/y pos is on the edge of a wall block.
	assert(
		(rVal == vec2<int>{sx, sy} && wall.at(sx, sy))
	 || flooredModulo(rVal.a, wall.segmentSize()) == 0 || flooredModulo(rVal.b, wall.segmentSize()) == 0
	);
	auto const xIntersect = flooredDivision(rVal.a, wall.segmentSize());
	auto const yIntersect = flooredDivision(rVal.b, wall.segmentSize());
	assert(
		wall.atIndex(xIntersect, yIntersect)
	 || wall.atIndex(xIntersect-1, yIntersect)
	 || wall.atIndex(xIntersect, yIntersect-1)
	 || wall.atIndex(xIntersect-1, yIntersect-1)
	);

	assert(doGunWallRaytraceUsingRectangles(wall, sx, sy, aimx, aimy, pool) == rVal);

	//   Dy_in/Dx_in  == Dy_out/Dx_out
	//=> Dy_in*Dx_out == Dy_out*Dx_in //Compare these values to avoid division by zero
	//TODO: Assert that the x/y pos is (approximately) on the incoming line
	//auto const a = (aimy - sy) * (rVal.x - sx);
	//auto const b = (rVal.y - sy) * (aimx - sx);
	//TODO: This assert is not quite right? Perhaps it is overly loose?
	//assert(b - iabs(b)/2 <= a + iabs(a)/2 && a - iabs(a)/2 <= b + iabs(b)/2);
	return rVal;
}

GunRaytraceResult doGunRaytrace(
	int const sx, int const sy,
	int const aimx_raw, int const aimy,

	//Env data
	Wall const &wall,

	//Platform data
	mp::std::vector<Collision> const &nextPlatform,

	//Box Data
	mp::std::vector<ObjectAndTime<Box, Frame *>> const &nextBox,
	mp::std::vector<char> const &nextBoxNormalDeparture,

	//Guy Data
	mp::std::vector<int> const &gx,
	mp::std::vector<int> const &gy,
	mp::std::vector<int> const &gw,
	mp::std::vector<int> const &gh,
	mp::std::vector<char> const &shootable,

	memory_pool<user_allocator_tbb_alloc> &pool
)
{
	
/*
	if (wall.at(sx,sy))
	{
		return {
			NONE,
			std::numeric_limits<std::size_t>::max(),
			sx,
			sy
		};
	}
	*/
	// prevent zero length vectors
	auto const aimx = sx == aimx_raw && sy == aimy ? aimx_raw + 1 : aimx_raw;

	auto hitPoint = doGunWallRaytrace(wall, sx, sy, aimx, aimy, pool);

	//TODO: Don't write the same code 3 times?
	// Platforms
	for (std::size_t i(0), isize(nextPlatform.size()); i < isize; ++i)
	{
		//TODO
		if (nextPlatform[i].getFunctional()) {
			auto const collision = getRaytraceRectangleCollision(
				sx, sy, aimx, aimy,
				nextPlatform[i].getX(), nextPlatform[i].getY(), 
				nextPlatform[i].getWidth(), nextPlatform[i].getHeight());
			//TODO: Add vector2 functions instead of doing it manually?
			if (collision.hit && (iabs(collision.point.a - sx) < iabs(hitPoint.a - sx) || iabs(collision.point.b - sy) < iabs(hitPoint.b - sy))) {
				hitPoint = collision.point;
			}
		}
	}
	auto targetType = PhysicsObjectType::NONE;
	std::size_t targetId = std::numeric_limits<std::size_t>::max();
	// Box
	for (std::size_t i(0), isize(nextBox.size()); i < isize; ++i)
	{
		if (nextBoxNormalDeparture[i])
		{
			auto const collision = getRaytraceRectangleCollision(
				sx, sy, aimx, aimy,
				nextBox[i].object.getX(), nextBox[i].object.getY(), 
				nextBox[i].object.getWidth(), nextBox[i].object.getHeight());
			if (collision.hit && (iabs(collision.point.a - sx) < iabs(hitPoint.a - sx) || iabs(collision.point.b - sy) < iabs(hitPoint.b - sy))) {
				hitPoint = collision.point;
				targetType = PhysicsObjectType::BOX;
				targetId = i;
			}
		}
	}
	
	// Guy
	for (std::size_t i(0), isize(boost::size(gx)); i < isize; ++i)
	{
		if (shootable[i])
		{
			auto const collision = getRaytraceRectangleCollision(
				sx, sy, aimx, aimy,
				gx[i], gy[i], 
				gw[i], gh[i]);
			if (collision.hit && (iabs(collision.point.a - sx) < iabs(hitPoint.a - sx) || iabs(collision.point.b - sy) < iabs(hitPoint.b - sy))) {
				hitPoint = collision.point;
				targetType = PhysicsObjectType::GUY;
				targetId = i;
			}
		}
	}
	return {targetType, targetId, hitPoint.a, hitPoint.b};
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

bool PointInRectangleSemiInclusive(int px, int py, int x, int y, int w, int h)
{
	return
		(px < x + w && px >= x)
		&&
		(py < y + h && py >= y)
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

int ManhattanDistance(int x1, int y1, int x2, int y2)
{
	return std::abs(x1 - x2) + std::abs(y1 - y2);
}

int ManhattanDistanceToRectangle(int px, int py, int x, int y, int w, int h)
{
	if (px < x) {
		if (py < y) {
			return ManhattanDistance(px, py, x, y);
		}
		else if (py <= y + h) {
			return std::abs(px - x);
		}
		else {
			return ManhattanDistance(px, py, x, y + h);
		}
	}
	else if (px <= x + w) {
		if (py < y) {
			return std::abs(py - y);
		}
		else if (py <= y + h) {
			return -1*std::min(std::min(std::abs(px - x), std::abs(px - (x + w))), std::min(std::abs(py - y), std::abs(py - (y + h))));
		}
		else {
			return std::abs(py - (y + h));
		}
	}
	else {
		if (py < y) {
			return ManhattanDistance(px, py, x + w, y);
		}
		else if (py <= y + h) {
			return std::abs(px - (x + w));
		}
		else {
			return ManhattanDistance(px, py, x + w, y + h);
		}
	}
}

int Distance(int x1, int y1, int x2, int y2)
{
	long long xDiff = static_cast<long long>(x1 - x2);
	long long yDiff = static_cast<long long>(y1 - y2);
	return static_cast<int>(std::sqrt(xDiff*xDiff + yDiff*yDiff));
}

int DistanceToRectangle(int px, int py, int x, int y, int w, int h)
{
	if (px < x) {
		if (py < y) {
			return Distance(px, py, x, y);
		}
		else if (py <= y + h) {
			return std::abs(px - x);
		}
		else {
			return Distance(px, py, x, y + h);
		}
	}
	else if (px <= x + w) {
		if (py < y) {
			return std::abs(py - y);
		}
		else if (py <= y + h) {
			return -1*std::min(std::min(std::abs(px - x), std::abs(px - (x + w))), std::min(std::abs(py - y), std::abs(py - (y + h))));
		}
		else {
			return std::abs(py - (y + h));
		}
	}
	else {
		if (py < y) {
			return Distance(px, py, x + w, y);
		}
		else if (py <= y + h) {
			return std::abs(px - (x + w));
		}
		else {
			return Distance(px, py, x + w, y + h);
		}
	}
}

int DistanceToRectangleAddSize(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
	return DistanceToRectangle(x1 + w1/2, y1 + h1/2, x2, y2, w2, h2) - ((w1 + h1) / 4);
}

bool IsRectangleRelationVertical(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, bool vertWinTies)
{
	// Extend non-square rectangles to be square by extending away from the other rectangle.
	if (x1 + w1 / 2 > x2 + w2 / 2) {
		if (y1 + h1 / 2 > y2 + h2 / 2) {
			// rec_1 down and right of rec_2
			if (w1 > h1) {
				h1 = w1;
			}
			else {
				w1 = h1;
			}
			if (w2 > h2) {
				y2 = y2 - (w2 - h2);
				h2 = w2;
			}
			else {
				x2 = x2 - (h2 - w2);
				w2 = h2;
			}
		}
		else {
			// rec_1 up and right of rec_2
			if (w1 > h1) {
				y1 = y1 - (w1 - h1);
				h1 = w1;
			}
			else {
				w1 = h1;
			}
			if (w2 > h2) {
				h2 = w2;
			}
			else {
				x2 = x2 - (h2 - w2);
				w2 = h2;
			}
		}
	}
	else {
		if (y1 + h1 / 2 > y2 + h2 / 2) {
			// rec_1 down and left of rec_2
			if (w1 > h1) {
				h1 = w1;
			}
			else {
				x1 = x1 - (h1 - w1);
				w1 = h1;
			}
			if (w2 > h2) {
				y2 = y2 - (w2 - h2);
				h2 = w2;
			}
			else {
				w2 = h2;
			}
		}
		else {
			// rec_1 up and left of rec_2
			if (w1 > h1) {
				y1 = y1 - (w1 - h1);
				h1 = w1;
			}
			else {
				x1 = x1 - (h1 - w1);
				w1 = h1;
			}
			if (w2 > h2) {
				h2 = w2;
			}
			else {
				w2 = h2;
			}
		}
	}
	
	// The relation of two squares is vertical if the vertical difference of their midpoints
	// is greater than the horiziontal difference.
	if (vertWinTies) {
		return (std::abs(y1 + h1 / 2 - (y2 + h2 / 2)) >= std::abs(x1 + w1 / 2 - (x2 + w2 / 2)));
	}
	return (std::abs(y1 + h1 / 2 - (y2 + h2 / 2)) > std::abs(x1 + w1 / 2 - (x2 + w2 / 2)));
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
