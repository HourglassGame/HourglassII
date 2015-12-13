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
        mt::std::vector<int> const &/*responsibleMutatorIndices*/,
        Guy const &objectToManipulate) override { return objectToManipulate; }
    virtual boost::optional<Box> mutateObject(
        mt::std::vector<int> const &/*responsibleMutatorIndices*/,
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
        std::vector<Box> boxArrivalList;
        boxArrivalList +=
            Box(19200, 34800, 1380, 3800, 3200, -1, -1, TimeDirection::FORWARDS),
            Box(22400, 40600, 0, 0, 3200, -1, -1, TimeDirection::FORWARDS),
            Box(25600, 40600, 0, 0, 3200, -1, -1, TimeDirection::FORWARDS),
            Box(28800, 38400, 0, 0, 3200, -1, -1, TimeDirection::FORWARDS),
            Box(32000, 38400, 0, 0, 3200, -1, -1, TimeDirection::FORWARDS),
            Box(35200, 38400, 0, 0, 3200, -1, -1, TimeDirection::FORWARDS),
            Box(38400, 38400, 0, 0, 3200, -1, -1, TimeDirection::FORWARDS),
            Box(41600, 38400, 0, 0, 3200, -1, -1, TimeDirection::FORWARDS);
        
        mt::std::vector<Box> additionalBoxes;
        mt::std::vector<ObjectAndTime<Box, FrameID>> nextBox;
        mt::std::vector<char> nextBoxNormalDeparture;
        mt::std::vector<Glitz> forwardsGlitz;
        mt::std::vector<Glitz> reverseGlitz;
        mt::std::vector<GlitzPersister> persistentGlitz;
        std::vector<Collision> nextPlatform;
        nextPlatform += Collision(22400, 43800, 0, 0, 0, 0, 6400, 1600, TimeDirection::FORWARDS);
        std::vector<PortalArea> nextPortal;
        std::vector<ArrivalLocation> arrivalLocations;
        arrivalLocations += ArrivalLocation(18200, 4060, 0, 0, TimeDirection::FORWARDS);
        std::vector<MutatorArea> mutators;
        TriggerFrameState triggerFrameState(new (multi_thread_tag{}) MockTriggerFrameStateImplementation());
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
            BoxGlitzAdder(forwardsGlitz, reverseGlitz, persistentGlitz),
            frame);

        return true;
    }
    struct tester {
        tester() {
            ::hg::getTestDriver().registerUnitTest("PhysicsEngine_testTester", testTester);
        }
    } tester;
}
}
}
