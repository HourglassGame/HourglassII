#include "PhysicsEngineUtilities.h"
#include "TestDriver.h"
#include "PortalArea.h"
#include "MutatorArea.h"
#include "FrameID.h"
#include "multi_thread_allocator.h"
#include <boost/assign.hpp>
namespace hg {
namespace physics_engine_test {
namespace {

class MockTriggerFrameStateImplementation : public TriggerFrameStateImplementation
{
    public:
    virtual PhysicsAffectingStuff
        calculatePhysicsAffectingStuff(
            Frame const* /*currentFrame*/,
            boost::transformed_range<
                GetBase<TriggerDataConstPtr>,
                mt::boost::container::vector<TriggerDataConstPtr>::type const> const& /*triggerArrivals*/)
    {
        assert(false && "Not Implemented!");
    }
    
    virtual bool shouldArrive(Guy const& /*potentialArriver*/) { return true; }
    virtual bool shouldArrive(Box const& /*potentialArriver*/) { return true; }
    
    virtual bool shouldPort(
        int /*responsiblePortalIndex*/,
        Guy const& /*potentialPorter*/,
        bool /*porterActionedPortal*/) { return true; }
    virtual bool shouldPort(
        int /*responsiblePortalIndex*/,
        Box const& /*potentialPorter*/,
        bool /*porterActionedPortal*/) { return true; }
    
    virtual boost::optional<Guy> mutateObject(
        mt::std::vector<int>::type const& /*responsibleMutatorIndices*/,
        Guy const& objectToManipulate) { return objectToManipulate; }
    virtual boost::optional<Box> mutateObject(
        mt::std::vector<int>::type const& /*responsibleMutatorIndices*/,
        Box const& objectToManipulate) { return objectToManipulate; }

    virtual boost::tuple<
		mt::std::map<Frame*, mt::std::vector<TriggerData>::type >::type,
		mt::std::vector<RectangleGlitz>::type,
		mt::std::vector<ObjectAndTime<Box, Frame*> >::type
	>
    getDepartureInformation(
        mt::boost::container::map<Frame*, ObjectList<Normal> >::type const& /*departures*/,
        Frame* /*currentFrame*/)
    {
        assert(false && "Not implemented");
    }

    virtual ~MockTriggerFrameStateImplementation(){}
};


    bool testTester() {
        using namespace boost::assign;
        
        std::vector<std::vector<bool> > wall;
        std::vector<bool> row;
        #define E row +=
        #define D wall.push_back(row); row.clear();
        E 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1; D
        E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1; D
        E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1; D
        E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1; D
        E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1; D
        E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1; D
        E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1; D
        E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1; D
        E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1; D
        E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1; D
        E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1; D
        E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1; D
        E 1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1; D
        E 1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1; D
        E 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1; D
        #undef E
        #undef D
        boost::array<boost::multi_array_types::index, 2> shape = {{ 20, 15 }};
        boost::multi_array<bool, 2> wallmap(shape);
        
        for (std::size_t i(0), iend(shape[0]); i != iend; ++i) {
            for (std::size_t j(0), jend(shape[1]); j != jend; ++j) {
                wallmap[i][j] = wall[j][i];
            }
        }
        
        Wall actualWall(3200, wallmap);
        Environment env(actualWall, 30);
        std::vector<Box> boxArrivalList;
        boxArrivalList +=
            //Box(3200, 28800, 0, 0, 3200, -1, -1, FORWARDS),
            /*
            Box(3200, 32000, 0, 0, 3200, -1, -1, FORWARDS),
            Box(3200, 35200, 0, 0, 3200, -1, -1, FORWARDS),
            Box(6400, 35200, 0, 0, 3200, -1, -1, FORWARDS),
            Box(8220, 32000, 0, 0, 3200, -1, -1, FORWARDS),
            Box(9600, 35200, 0, 0, 3200, -1, -1, FORWARDS),
            *//*
            Box(10701, 28800, 0, 0, 3200, -1, -1, FORWARDS),
            Box(11420, 32000, 0, 0, 3200, -1, -1, FORWARDS),
            Box(12800, 35200, 0, 0, 3200, -1, -1, FORWARDS),
            Box(13901, 28800, 0, 0, 3200, -1, -1, FORWARDS),
            Box(14620, 32000, 0, 0, 3200, -1, -1, FORWARDS),
            Box(16000, 35200, 0, 0, 3200, -1, -1, FORWARDS),*/
            Box(19200, 34800, 1380, 3800, 3200, -1, -1, FORWARDS),//--
            //Box(22400, 37400, 0, 0, 3200, -1, -1, FORWARDS),
            Box(22400, 40600, 0, 0, 3200, -1, -1, FORWARDS),//--
            Box(25600, 40600, 0, 0, 3200, -1, -1, FORWARDS),//--
            Box(28800, 38400, 0, 0, 3200, -1, -1, FORWARDS),//--
            //Box(28850, 41600, 0, 0, 3200, -1, -1, FORWARDS),
            Box(32000, 38400, 0, 0, 3200, -1, -1, FORWARDS),//--
            //Box(32552, 41600, 0, 0, 3200, -1, -1, FORWARDS),
            //Box(33106, 32000, 0, 0, 3200, -1, -1, FORWARDS),
            //Box(33711, 35200, 0, 0, 3200, -1, -1, FORWARDS),
            Box(35200, 38400, 0, 0, 3200, -1, -1, FORWARDS),//--
            //Box(36711, 32000, 0, 0, 3200, -1, -1, FORWARDS),
            //Box(36911, 35200, 0, 0, 3200, -1, -1, FORWARDS),
            Box(38400, 38400, 0, 0, 3200, -1, -1, FORWARDS),//--
            //Box(38400, 41600, 0, 0, 3200, -1, -1, FORWARDS),
            //Box(39911, 32000, 0, 0, 3200, -1, -1, FORWARDS),
            //Box(40111, 35200, 0, 0, 3200, -1, -1, FORWARDS),
            Box(41600, 38400, 0, 0, 3200, -1, -1, FORWARDS)//--
            /*,
            Box(41600, 41600, 0, 0, 3200, -1, -1, FORWARDS),
            Box(56400, 28800, 0, 0, 3200, -1, -1, FORWARDS),
            Box(57600, 22400, 0, 0, 3200, -1, -1, FORWARDS)//,
            Box(57600, 25600, 0, 0, 3200, -1, -1, FORWARDS)*/
            ;
        mt::std::vector<Box>::type additionalBoxes;
        mt::std::vector<ObjectAndTime<Box, FrameID> >::type nextBox;
        mt::std::vector<char>::type nextBoxNormalDeparture;
        std::vector<Collision> nextPlatform;
        nextPlatform += Collision(22400, 43800, 0, 0, 6400, 1600, FORWARDS);
        std::vector<PortalArea> nextPortal;
        //nextPortal +=
          //  PortalArea(0, 18200, 40600, 4200, 4200, 0, 0, 50, FORWARDS, 0, 0, -16000, 1, 120, 0, 1, 1);
        std::vector<ArrivalLocation> arrivalLocations;
        arrivalLocations += ArrivalLocation(18200, 4060, 0, 0, FORWARDS);
        std::vector<MutatorArea> mutators;
        //mutators += MutatorArea(50000, 25600, 800, 6400, 0, 0, 0, FORWARDS);
        TriggerFrameState triggerFrameState(multi_thread_new<MockTriggerFrameStateImplementation>());
        FrameID frame(9654, UniverseID(10800));
        //This is testing a case which caused a crash. If this line does not cause a crash,
        //then the test has succeeded.
        
        boxCollisionAlogorithm(
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
            frame);
        
        return true;
    }
    struct tester {
        tester() {
            ::hg::getTestDriver().registerUnitTest(testTester);
        }
    } tester;
}
}
}
