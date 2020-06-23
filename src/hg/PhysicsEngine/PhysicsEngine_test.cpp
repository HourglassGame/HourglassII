#include "PhysicsEngineUtilities.h"
#include "hg/Util/TestDriver.h"
#include "hg/PhysicsEngine/TriggerSystem/PortalArea.h"
#include "hg/PhysicsEngine/TriggerSystem/MutatorArea.h"
#include "hg/TimeEngine/FrameID.h"
#include "hg/Util/multi_thread_allocator.h"
#include "hg/Util/memory_pool.h"
#include "hg/mp/std/vector"
namespace hg {
namespace physics_engine_test {
namespace {

class MockTriggerFrameStateImplementation final : public TriggerFrameStateImplementation
{
	public:
	virtual PhysicsAffectingStuff
		calculatePhysicsAffectingStuff(
			Frame const * /*currentFrame*/,
			boost::transformed_range<
				GetBase<TriggerDataConstPtr>,
				mt::boost::container::vector<TriggerDataConstPtr> const> const &/*triggerArrivals*/) override
	{
		assert(false && "Not Implemented!");
		throw std::runtime_error("Not Implemented Exception");
	}

	virtual bool shouldArrive(Guy const &/*potentialArriver*/) override { return true; }
	virtual bool shouldArrive(Box const &/*potentialArriver*/) override { return true; }

	virtual bool shouldPort(
		int /*responsiblePortalIndex*/,
		Guy const &/*potentialPorter*/,
		bool /*porterActionedPortal*/) override { return true; }
	virtual bool shouldPort(
		int /*responsiblePortalIndex*/,
		Box const &/*potentialPorter*/,
		bool /*porterActionedPortal*/) override { return true; }

	virtual boost::optional<Guy> mutateObject(
		mp::std::vector<int> const &/*responsibleMutatorIndices*/,
		Guy const &objectToManipulate) override { return objectToManipulate; }
	virtual boost::optional<Box> mutateObject(
		mp::std::vector<int> const &/*responsibleMutatorIndices*/,
		Box const &objectToManipulate) override { return objectToManipulate; }

	virtual DepartureInformation
	getDepartureInformation(
		mt::std::map<Frame *, ObjectList<Normal> > const &/*departures*/,
		Frame * /*currentFrame*/) override
	{
		assert(false && "Not implemented");
		throw std::runtime_error("Not Implemented Exception");
	}

	virtual ~MockTriggerFrameStateImplementation() noexcept override {}
};


	bool testTester() {
		std::vector<std::vector<bool> > wall;
		std::vector<bool> row;
		#define E row = std::vector<bool>{
		#define D }; wall.push_back(row); row.clear();
		E 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 D
		E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
		E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
		E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
		E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
		E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
		E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
		E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
		E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
		E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
		E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1 D
		E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1 D
		E 1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1 D
		E 1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1 D
		E 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 D
		#undef E
		#undef D
		std::array<hg::multi_vector<bool, 2>::index, 2> shape = {{ 20, 15 }};
		hg::multi_vector<bool, 2> wallmap;
		wallmap.resize(shape);

		for (std::size_t i(0), iend(shape[0]); i != iend; ++i) {
			for (std::size_t j(0), jend(shape[1]); j != jend; ++j) {
				wallmap[i][j] = wall[j][i];
			}
		}

		Wall actualWall(3200, wallmap, "HourglassI");
		Environment env{actualWall, 30};
		std::vector<Box> boxArrivalList{
			Box(19200, 34800, 1380, 3800, 3200, 3200, BoxType::CRATE, 0, -1, -1, TimeDirection::FORWARDS),
			Box(22400, 40600, 0, 0, 3200, 3200, BoxType::CRATE, 0, -1, -1, TimeDirection::FORWARDS),
			Box(25600, 40600, 0, 0, 3200, 3200, BoxType::CRATE, 0, -1, -1, TimeDirection::FORWARDS),
			Box(28800, 38400, 0, 0, 3200, 3200, BoxType::CRATE, 0, -1, -1, TimeDirection::FORWARDS),
			Box(32000, 38400, 0, 0, 3200, 3200, BoxType::CRATE, 0, -1, -1, TimeDirection::FORWARDS),
			Box(35200, 38400, 0, 0, 3200, 3200, BoxType::CRATE, 0, -1, -1, TimeDirection::FORWARDS),
			Box(38400, 38400, 0, 0, 3200, 3200, BoxType::CRATE, 0, -1, -1, TimeDirection::FORWARDS),
			Box(41600, 38400, 0, 0, 3200, 3200, BoxType::CRATE, 0, -1, -1, TimeDirection::FORWARDS)
		};

		const std::size_t initialPoolSize{ 2 << 5 };
		memory_pool<user_allocator_tbb_alloc> pool{ initialPoolSize };

		mp::std::vector<Box> additionalBoxes(pool);
		mp::std::vector<ObjectAndTime<Box, FrameID>> nextBox(pool);
		mp::std::vector<char> nextBoxNormalDeparture(pool);
		mt::std::vector<Glitz> forwardsGlitz;
		mt::std::vector<Glitz> reverseGlitz;
		mp::std::vector<GlitzPersister> persistentGlitz(pool);
		std::vector<Collision> nextPlatform{
			Collision(22400, 43800, 0, 0, 0, 0, 6400, 1600, CollisionType::PLATFORM, TimeDirection::FORWARDS)
		};
		std::vector<PortalArea> nextPortal;
		std::vector<ArrivalLocation> arrivalLocations{
			ArrivalLocation(18200, 4060, 0, 0, TimeDirection::FORWARDS)
		};
		std::vector<MutatorArea> mutators;
		TriggerFrameState triggerFrameState(new (pool) MockTriggerFrameStateImplementation());
		FrameID frame(9654, UniverseID(10800));
		
		//This is testing a case which caused a crash. If this line does not cause a crash,
		//then the test has succeeded.
		boxCollisionAlgorithm(
			env,
			boxArrivalList,
			additionalBoxes,
			nextBox,
			nextBoxNormalDeparture,
			nextPlatform,
			nextPortal,
			arrivalLocations,
			mutators,
			triggerFrameState,
			BoxGlitzAdder(forwardsGlitz, reverseGlitz, persistentGlitz),
			frame,
			pool);

		return true;
	}
	bool testGetRayToLineCollision() {
		//

#if 0
		auto const sa = 500;
		auto const sb = 500;
		auto const aima = 1200;
		auto const aimb = 
		auto const collisionResult = getNormalisedRayToLineCollision(
			int const sa, int const sb, int const aima, int const aimb,
			int const lineStarta, int const lineEnda, int const linePosb,
			bool const axesSwapped, bool const lineStartaInclusive, bool const lineEndaInclusive, bool const lineBinclusive,
			bool const lineBisSmallB);
#endif
		//TODO


		//Start on Line:
		// Start at start
		// Start in middle
		// Start at end

		//Start directly above line
		
		//Start diagonally above line

		//

		return true;
	}

	bool testGetNormalisedRayToLineCollision() {

		//TODO
		return true;
	}
/*
Rectangle Collision
(Wall/Box/Guy/Platform/etc)
Spec:
	at least two corners(both on the same edge) must be inclusive,
		otherwise perfect diagonal lines can phase between diagonally
		adjacent blocks
		(this would be a stupid game mechanic, and would force all levels with lasers to not use such diagonal blocks)


		'Inclusive' can mean either
		* 'lines that start on this edge/corner hit the block'
		* 'lines that pass through coordinates equal to this corner/block hit the block, rather than just missing the block'
		Or both of these

		The first kind of inclusive does not seem to be useful in any circumstance,
		other than for consistency. (it is equivalent to saying that the start of the gun rays are inclusive; they should probably be exclusive, so that they can start right up against a wall / other object, and still be able to fire)



		Options:
	make 2 corners inclusive,
		make 3 corners inclusive
		make 4 corners inclusive

		(in the 2 / 3 corners inclusive options, there are also choices about whether or not the associated edges are inclusive(that is, if shots starting on the edge and aiming directly at the non - inclusive corner should hit the edge))


		orthogonally, should the start of the ray can be inclusive or not
		either choice is valid here, but I would suggest making it non - inclusive
		(so that shots can start right on the edge of an object without any problems)

		Whatever number of corners > 1 we pick, there will need to be a tie - break to choose which specific block we hit when we hit right on the corner shared by two adjacent blocks
		(even with just 1 corner there would be a tie - break, but it would be simplified in some cases)

		2 corners is not directionally symmetrical, seems ugly

		3 corners is ok, but it makes a lot of complex 'aiming directly along the line' logic necessary, even though that logic would only be used for a single direction along 2 edges; similarly the 'glancing-the-corner' logic would be for just a single corner

		4 corners seems simplest(though it still maybe needs a bit of hackery to handle the fact that there can be multiple collidable lines in a single place; and the most natural version of the code would sometimes only check one of the two)



		so: 4 corners, but with ray - start non - inclusive
			(in this case,
				I can't see any good reason to make the edges non-inclusive
				and making the edges also inclusive seem the most natural, to me;

		Rounding:
		Should always be exactly on line in one dimension (no rounding here)
		For the other dimension, the choice is essentially arbitrary.

		Need to respect
		(py-sy)*dx ~= (px-sx)*dy

		Let's say (arbitrarily chosen):
		Should select aimpoint that would make the angle be smaller.

		Because +y is downwards in Hourglass, angles increase in the clockwise direction;
		So we want the floored clockwise angle.
		(The rounding should move the angle in the anti-clockwise direction, if at all):

		So:
		line quadrant -> floor/ceil when rounding
		+x +y -> ceil x, floor y
		+x -y -> floor x, floor y
		-x +y -> ceil x, ceil y
		-x -y -> floor x, ceil y
	))
*/



	bool testGetRaytraceRectangleCollision() {
		//TODO
		//Foreach 8 starting points (midpoints of edges/corners)
		// Foreach 16 shooting directions
		//  Foreach (inside, outside, far-outside) block at starting point
		//   check result correct.

		//TODO: also test rectangles at negative positions
		
		auto const testRectangleCollision = [](
			int const rectangleleft,
			int const rectangletop,
			int const rectanglewidth,
			int const rectangleheight)
		{
			{
				std::map<int, std::map<int, std::map<int, RaytraceRectangleCollisionResult>>> hardcodedExpectedCollisionResults{
					{-1,//xpos
						std::map<int, std::map<int, RaytraceRectangleCollisionResult>>{
							{-1,//ypos
								std::map<int, RaytraceRectangleCollisionResult>{
									{ 0,//verticalLine (horizontal)
									  RaytraceRectangleCollisionResult{true, {rectangleleft, rectangletop }}
									},
									{ 1,//verticalLine (vertical)
									  RaytraceRectangleCollisionResult{true, {rectangleleft, rectangletop }}
									}
								}
							},
							{ 1,//ypos
								std::map<int, RaytraceRectangleCollisionResult>{
									{ 0,//verticalLine (horizontal)
									  RaytraceRectangleCollisionResult{true, {rectangleleft, rectangletop + rectangleheight }}
									},
									{ 1,//verticalLine (vertical)
									  RaytraceRectangleCollisionResult{true, {rectangleleft, rectangletop + rectangleheight}}
									}
								}
							}
						}
					},
					{ 1,//xpos
						std::map<int, std::map<int, RaytraceRectangleCollisionResult>>{
							{-1,//ypos
								std::map<int, RaytraceRectangleCollisionResult>{
									{ 0,//verticalLine (horizontal)
									  RaytraceRectangleCollisionResult{true, {rectangleleft + rectanglewidth, rectangletop}}
									},
									{ 1,//verticalLine (vertical)
									  RaytraceRectangleCollisionResult{ true,{rectangleleft + rectanglewidth, rectangletop } }
									}
								}
							},
							{ 1,//ypos
								std::map<int, RaytraceRectangleCollisionResult>{
									{ 0,//verticalLine (horizontal)
									  RaytraceRectangleCollisionResult{true, {rectangleleft + rectanglewidth, rectangletop + rectangleheight}}
									},
									{ 1,//verticalLine (vertical)
									  RaytraceRectangleCollisionResult{true, {rectangleleft + rectanglewidth, rectangletop + rectangleheight}}
									}
								}
							}
						}
					},
				};

				//For each corner, test horizontal/vertical shots starting from outside the corner
				//and aimed beyond the other side of the rectangle (inline with the horizontal/vertical edge starting from the given corner)
				//
				//This test is checking the handling of the edge condition for exactly horizontal/vertical lines that go exactly along the edge of the rectangle.
				//The all edges should be hit

				for (int xpos = -1; xpos <= 1; xpos += 2) {
					for (int ypos = -1; ypos <= 1; ypos += 2) {
						for (int verticalLine = 0; verticalLine <= 1; ++verticalLine) {
							int const sx = rectangleleft
								+ (xpos + 1)*rectanglewidth / 2
								+ (verticalLine ? 0 : xpos * (rectanglewidth / 2));
							int const sy = rectangletop
								+ (ypos + 1)*rectangleheight / 2
								+ (verticalLine ? ypos * (rectangleheight / 2) : 0);

							int const aimx = sx - (verticalLine ? 0 : xpos * (rectanglewidth + rectanglewidth));
							int const aimy = sy - (!verticalLine ? 0 : ypos * (rectangleheight + rectangleheight));

							//TODO: test a whole bunch of different aim lengths

							auto const collisionResult = getRaytraceRectangleCollision(
								sx, sy,
								aimx, aimy,
								rectangleleft, rectangletop,
								rectanglewidth, rectangleheight);

							auto const expectedCollisionResult = RaytraceRectangleCollisionResult{
								true,
								{
									rectangleleft + (xpos + 1)*rectanglewidth / 2,
									rectangletop + (ypos + 1)*rectangleheight / 2
								}
							};

							auto const hardcodedExpectedCollisionResult = hardcodedExpectedCollisionResults.at(xpos).at(ypos).at(verticalLine);
							if (!(hardcodedExpectedCollisionResult == expectedCollisionResult)) {
								return false;
							}

							if (!(collisionResult == expectedCollisionResult))
							{
								return false;
							}
						}
					}
				}
			}

			{
				for (int xpos = -1; xpos <= 1; xpos += 2) {
					for (int ypos = -1; ypos <= 1; ypos += 2) {
						int const aimx = rectangleleft + (xpos + 1)*rectanglewidth / 2;
						int const aimy = rectangletop + (ypos + 1)*rectangleheight / 2;

						for (int xstartdir = -1; xstartdir <= 1; ++xstartdir) {
							for (int ystartdir = -1; ystartdir <= 1; ++ystartdir) {
								if (xstartdir == 0 && ystartdir == 0) continue;

								auto const sx = aimx + xstartdir * 500;
								auto const sy = aimy + ystartdir * 500;

								//TODO: test a whole bunch of different aim lengths
								auto const collisionResult = getRaytraceRectangleCollision(
									sx, sy,
									aimx, aimy,
									rectangleleft, rectangletop,
									rectanglewidth, rectangleheight);

								bool startsInside = PointInRectangleInclusive(sx, sy, rectangleleft, rectangletop, rectanglewidth, rectangleheight);

								auto const expectedCollisionResult = RaytraceRectangleCollisionResult{
									true,
									{
										startsInside ? sx : aimx,
										startsInside ? sy : aimy
									}
								};

								if (!(collisionResult == expectedCollisionResult))
								{
									return false;
								}
							}
						}
					}
				}
			}
			
			for (int xpos = -1; xpos <= 1; ++xpos) {
				for (int ypos = -1; ypos <= 1; ++ypos) {
					//if (xpos == 0 && ypos == 0) continue;
					//startInside: Start exactly on the edge of the rectangle, rather than one pixel outside the rectangle.
					for (int startInside = (xpos == 0 && ypos == 0) ? 1 : 0; startInside <= 1; ++startInside) {
						int const sx = rectangleleft
							+ (xpos + 1)*rectanglewidth / 2
							+ (startInside ? 0 : xpos);
						int const sy = rectangletop
							+ (ypos + 1)*rectangleheight / 2
							+ (startInside ? 0 : ypos);

						for (int xdir = -2; xdir <= 2; ++xdir) {
							for (int ydir = -2; ydir <= 2; ++ydir) {
								//TODO: Do diagonals better?
								if (xdir == 0 && ydir == 0) continue;//TODO: actually check result for zero-length input lines?
								int const aimx = sx + xdir * 5000;
								int const aimy = sy + ydir * 5000;

								//TODO: test a whole bunch of different aim lengths
								auto const collisionResult = getRaytraceRectangleCollision(
									sx, sy,
									aimx, aimy,
									rectangleleft, rectangletop,
									rectanglewidth, rectangleheight);

								bool const expectedHit =
									(startInside &&
									((xpos != 0 && ypos != 0 && xpos * xdir <= 0 && ypos * ydir <= 0) //Corners...
										|| (xpos == 0 && ypos * ydir <= 0) //Sides/centre...
										|| (ypos == 0 && xpos * xdir <= 0)
										))
									|| ((xpos != 0 && ypos != 0 && xpos * xdir < 0 && ypos * ydir < 0)//Corners...
										|| (xpos == 0 && ypos * ydir < 0) //Sides..
										|| (ypos == 0 && xpos * xdir < 0)
										);

								bool const expectToHitLeftOrRight = expectedHit && (ypos == 0 || (xpos != 0 && iabs(ydir) > iabs(xdir)));
								//+ve => floor
								//-ve => ceil
								int const xRound = ydir > 0 ? -1 : 1;
								int const yRound = xdir < 0 ? -1 : 1;
								auto const expectedCollisionResult = RaytraceRectangleCollisionResult{
									expectedHit,
									{
										/*
										!expectedHit ? 0 : startInside ? sx :  expectToHitLeftOrRight ? sx - xpos : sx - ypos*xRound*flooredDivision(xRound*xdir,ydir), //ceil(x) == -(floor(-x))
										!expectedHit ? 0 : startInside ? sy : !expectToHitLeftOrRight ? sy - ypos : sy - xpos*yRound*flooredDivision(yRound*ydir,xdir)
										*/

										!expectedHit ? 0 : startInside ? sx : expectToHitLeftOrRight ? sx - xpos : sx - -1 * xRound*flooredDivision(-1 * ypos*xRound*xdir,ydir), //ceil(x) == -(floor(-x))
										!expectedHit ? 0 : startInside ? sy : !expectToHitLeftOrRight ? sy - ypos : sy - -1 * yRound*flooredDivision(-1 * xpos*yRound*ydir,xdir)
									}
								};

								if (!(collisionResult == expectedCollisionResult)) {
									return false;
								}
							}
						}
					}
				}
			}
			return true;
		};
		return
			testRectangleCollision( 26000,  14000, 3200, 1200)
		 && testRectangleCollision( 26000, -14000, 3200, 1200)
		 && testRectangleCollision(-26000,  14000, 3200, 1200)
		 && testRectangleCollision(-26000, -14000, 3200, 1200)
			;
	}
	bool testDoGunWallRaytrace(){
		memory_pool<user_allocator_tbb_alloc> pool;
		//TODO
		//Test perfectly diagonal line
		//Test rounding is consistent between directions
		//Test line extension
		//Test line contraction
		//Test start point inside wall
		//Test start point exactly at edge of wall

		std::vector<std::vector<bool>> wall;
		std::vector<bool> row;
#define E row = std::vector<bool>{
#define D }; wall.push_back(row); row.clear();
		E 1, 1, 1, 1, 1 D
		E 1, 0, 0, 0, 1 D
		E 1, 0, 1, 0, 1 D
		E 1, 0, 0, 0, 1 D
		E 1, 1, 1, 1, 1 D
#undef E
#undef D
		std::array<hg::multi_vector<bool, 2>::index, 2> shape = { { 5, 5 } };
		hg::multi_vector<bool, 2> wallmap;
		wallmap.resize(shape);

		for (std::size_t i(0), iend(shape[0]); i != iend; ++i) {
			for (std::size_t j(0), jend(shape[1]); j != jend; ++j) {
				wallmap[i][j] = wall[j][i];
			}
		}
		int const segmentSize = 3200;
		Wall actualWall(segmentSize, wallmap, "HourglassI");

		int const rectanglewidth = segmentSize;
		int const rectangleheight = segmentSize;
		int const rectangleleft = 2 * segmentSize;
		int const rectangletop = 2 * segmentSize;

		//Horizontal/vertical lines that glance edges hit corners
		for (int xpos = -1; xpos <= 1; xpos += 2) {
			for (int ypos = -1; ypos <= 1; ypos += 2) {
				for (int verticalLine = 0; verticalLine <= 1; ++verticalLine) {
					int const sx = rectangleleft
						+ (xpos + 1)*rectanglewidth / 2
						+ (verticalLine ? 0 : xpos * (segmentSize/2));
					int const sy = rectangletop
						+ (ypos + 1)*rectangleheight / 2
						+ (verticalLine ? ypos * (segmentSize / 2) : 0);

					int const aimx = sx - (verticalLine ? 0 : xpos * (segmentSize + rectanglewidth));
					int const aimy = sy - (!verticalLine ? 0 : ypos * (segmentSize + rectangleheight));

					auto const collisionResult = doGunWallRaytrace(
						actualWall,
						sx, sy,
						aimx, aimy,
						pool);

					auto const expectedCollisionResult =
						vec2<int>{
							rectangleleft + (xpos + 1)*rectanglewidth / 2,
							rectangletop + (ypos + 1)*rectangleheight / 2
						};

					if (collisionResult != expectedCollisionResult)
					{
						return false;
					}
				}
			}
		}


		{
			for (int xpos = -1; xpos <= 1; xpos += 2) {
				for (int ypos = -1; ypos <= 1; ypos += 2) {
					int const aimx = rectangleleft + (xpos + 1)*rectanglewidth / 2;
					int const aimy = rectangletop + (ypos + 1)*rectangleheight / 2;

					for (int xstartdir = -1; xstartdir <= 1; ++xstartdir) {
						for (int ystartdir = -1; ystartdir <= 1; ++ystartdir) {
							if (xstartdir == 0 && ystartdir == 0) continue;

							auto const sx = aimx + xstartdir * segmentSize/2;
							auto const sy = aimy + ystartdir * segmentSize/2;

							//TODO: test a whole bunch of different aim lengths
							auto const collisionResult = doGunWallRaytrace(
								actualWall,
								sx, sy,
								aimx, aimy,
								pool);

							bool startsInside = PointInRectangleInclusive(sx, sy, rectangleleft, rectangletop, rectanglewidth, rectangleheight);
							

							auto const expectedCollisionResult = vec2<int>{
								//expectHit,
								//{
									(startsInside ? sx : aimx),
									(startsInside ? sy : aimy)
								//}
							};

							if (collisionResult != expectedCollisionResult)
							{
								return false;
							}
						}
					}
				}
			}
		}


		for (int xpos = -1; xpos <= 1; ++xpos) {
			for (int ypos = -1; ypos <= 1; ++ypos) {
				//if (xpos == 0 && ypos == 0) continue;
				//startInside: Start exactly on the edge of the rectangle, rather than one pixel outside the rectangle.
				for (int startInside = (xpos == 0 && ypos == 0) ? 1 : 0; startInside <= 1; ++startInside) {
					int const sx = rectangleleft
						+ (xpos + 1)*rectanglewidth / 2
						+ (startInside ? 0 : xpos);
					int const sy = rectangletop
						+ (ypos + 1)*rectangleheight / 2
						+ (startInside ? 0 : ypos);

					for (int xdir = -2; xdir <= 2; ++xdir) {
						for (int ydir = -2; ydir <= 2; ++ydir) {
							//TODO: Do diagonals better?
							if (xdir == 0 && ydir == 0) continue;//TODO: actually check result for zero-length input lines?
							int const aimx = sx + xdir * 5000;
							int const aimy = sy + ydir * 5000;

							//TODO: test a whole bunch of different aim lengths
							auto const collisionResult = doGunWallRaytrace(
								actualWall,
								sx, sy,
								aimx, aimy,
								pool);
							
							bool const expectedHit =
								(startInside &&
								((xpos != 0 && ypos != 0 && xpos * xdir <= 0 && ypos * ydir <= 0) //Corners...
									|| (xpos == 0 && ypos * ydir <= 0) //Sides/centre...
									|| (ypos == 0 && xpos * xdir <= 0)
									))
								|| ((xpos != 0 && ypos != 0 && xpos * xdir < 0 && ypos * ydir < 0)//Corners...
									|| (xpos == 0 && ypos * ydir < 0) //Sides..
									|| (ypos == 0 && xpos * xdir < 0)
									);

							bool const expectToHitLeftOrRight = expectedHit && (ypos == 0 || (xpos != 0 && iabs(ydir) > iabs(xdir)));
							//+ve => floor
							//-ve => ceil
							int const xRound = ydir > 0 ? -1 : 1;
							int const yRound = xdir < 0 ? -1 : 1;
							auto const expectedCollisionResult = vec2<int>{
								/*
								!expectedHit ? 0 : startInside ? sx : expectToHitLeftOrRight ? sx - xpos : sx - ypos*xRound*flooredDivision(xRound*xdir,ydir), //ceil(x) == -(floor(-x))
								!expectedHit ? 0 : startInside ? sy : !expectToHitLeftOrRight ? sy - ypos : sy - xpos*yRound*flooredDivision(yRound*ydir,xdir)
								*/
								!expectedHit ? 0 : startInside ? sx : expectToHitLeftOrRight ? sx - xpos : sx - -1*xRound*flooredDivision(-1*ypos*xRound*xdir,ydir), //ceil(x) == -(floor(-x))
								!expectedHit ? 0 : startInside ? sy : !expectToHitLeftOrRight ? sy - ypos : sy - -1*yRound*flooredDivision(-1*xpos*yRound*ydir,xdir)
							};

							//TODO: check result when !expectedHit too!
							if (expectedHit && collisionResult != expectedCollisionResult) {
								return false;
							}
						}
					}
				}
			}
		}


		return true;
	}
	bool testDoGunRaytrace() {
		//TODO
		// Correct ordering of wall -> platform -> box -> guy precedence for equal location objects.
		return true;
	}

	struct tester final {
		tester() {
			::hg::getTestDriver().registerUnitTest("PhysicsEngine_testTester", testTester);
			::hg::getTestDriver().registerUnitTest("PhysicsEngine_testGetNormalisedRayToLineCollision", testGetNormalisedRayToLineCollision);
			::hg::getTestDriver().registerUnitTest("PhysicsEngine_testGetRayToLineCollision", testGetRayToLineCollision);
			::hg::getTestDriver().registerUnitTest("PhysicsEngine_testGetRaytraceRectangleCollision", testGetRaytraceRectangleCollision);
			::hg::getTestDriver().registerUnitTest("PhysicsEngine_testDoGunWallRaytrace", testDoGunWallRaytrace);
			::hg::getTestDriver().registerUnitTest("PhysicsEngine_testDoGunRaytrace", testDoGunRaytrace);
		}
	} tester;
}
}
}
