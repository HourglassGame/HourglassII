#include "SimpleConfiguredTriggerSystem.h"
#include "CommonTriggerCode.h"
#include "hg/TimeEngine/Glitz/RectangleGlitz.h"
#include "hg/TimeEngine/Glitz/ImageGlitz.h"
#include "hg/TimeEngine/Glitz/AudioGlitz.h"
#include "hg/TimeEngine/Glitz/TextGlitz.h"
#include "hg/LuaUtil/LuaUtilities.h"
#include "hg/LuaUtil/LuaError.h"
#include "hg/LuaUtil/LuaStackManager.h"
#include "hg/LuaUtil/LuaSandbox.h"
#include "TriggerClause.h"
#include "hg/mt/std/memory"

#include "hg/Util/Maths.h"
#include "hg/GlobalConst.h"

#include "lua/lauxlib.h"
#include <cmath>

namespace hg {
	//TODO: Move this to a central location
	unsigned asPackedColour(int const r, int const g, int const b) { 
		return r << 24 | g << 16 | b << 8;
	}

	//TODO: Remove this overload!
	std::tuple<Glitz, Glitz> calculateBidirectionalGlitz(
		int const layer,
		int const x, int const y, int const xspeed, int const yspeed, int const width, int const height, TimeDirection const timeDirection,
		unsigned const forwardsColour,
		unsigned const reverseColour)
	{
		Glitz sameDirectionGlitz(mt::std::make_unique<RectangleGlitz>(
			layer,
			x - xspeed, y - yspeed,
			width, height,
			forwardsColour
		));
		Glitz oppositeDirectionGlitz(mt::std::make_unique<RectangleGlitz>(
			layer,
			x - xspeed, y - yspeed,
			width, height,
			reverseColour
		));
		if (timeDirection == TimeDirection::FORWARDS) {
			return {std::move(sameDirectionGlitz), std::move(oppositeDirectionGlitz)};
		}
		else {
			return {std::move(oppositeDirectionGlitz), std::move(sameDirectionGlitz)};
		}
	}

	std::tuple<Glitz, Glitz> calculateBidirectionalGlitz(
		int const layer,
		DynamicArea const &dynamicArea,
		unsigned const forwardsColour,
		unsigned const reverseColour)
	{
		return calculateBidirectionalGlitz(
			layer,
			dynamicArea.x, dynamicArea.y, dynamicArea.xspeed, dynamicArea.yspeed, dynamicArea.width, dynamicArea.height, dynamicArea.timeDirection,
			forwardsColour,
			reverseColour);
	}

	std::tuple<Glitz, Glitz> calculateButtonGlitz(DynamicArea const &buttonArea, bool const buttonState) {
		auto const colour = buttonState ? asPackedColour(150, 255, 150) : asPackedColour(255, 150, 150);

		return calculateBidirectionalGlitz(400, buttonArea, colour, colour);
	}
	/*
	local function calculateBeamGlitz(proto, beamPositionAndVelocity, buttonState)
		if buttonState then
			return false
		end
		local colour = {r = 255, g = 0, b = 0}

		return calculateBidirectionalGlitz(400, constructDynamicArea(proto, beamPositionAndVelocity), colour, colour)
	end
	*/
	std::optional<std::tuple<Glitz, Glitz>> calculateBeamGlitz(DynamicArea const &beamArea, bool const buttonState) {
		if (buttonState) {
			return {};
		}

		auto const colour = asPackedColour(255, 0, 0);
		return {calculateBidirectionalGlitz(400, beamArea, colour, colour)};
	}
	struct PositionAndVelocity {
		int position;
		int velocity;
	};
	PositionAndVelocity solvePDEquation(
		AxisCollisionDestination const &dest,
		int const position,
		double velocity,
		mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
		Frame const *const currentFrame
		)
	{
		auto const desiredPosition = (dest.hasClauseDesiredPosition ? 
			dest.clauseDesiredPosition.execute(triggerArrivals, getFrameNumber(currentFrame)) : dest.desiredPosition);
		auto const acceleration = (dest.hasClauseAcceleration ? 
			dest.clauseAcceleration.execute(triggerArrivals, getFrameNumber(currentFrame)) : dest.acceleration);
		auto const deceleration = (dest.hasClauseDeceleration ? 
			dest.clauseDeceleration.execute(triggerArrivals, getFrameNumber(currentFrame)) : dest.deceleration);
		auto const maxSpeed = (dest.hasClauseMaxSpeed ? 
			dest.clauseMaxSpeed.execute(triggerArrivals, getFrameNumber(currentFrame)) : dest.maxSpeed);

		if (desiredPosition != position) {
			if (abs(desiredPosition - position) <= abs(velocity) && abs(velocity) <= deceleration)
			{
				velocity = desiredPosition - position;
			}
			else
			{
				auto const direction = sign(position-desiredPosition);
				if (velocity * direction > 0) {
					velocity -= direction*deceleration;
					if (velocity * direction < 0) {
						velocity = 0;
					}
				}
				else if (
					abs(position - desiredPosition)
					>
					(pow(velocity - direction * acceleration, 2) * 3. / (2. * deceleration)))
				{
					velocity -= direction * acceleration;
				}
				else if (abs(position - desiredPosition) > pow(velocity, 2)*3./(2.*deceleration))
				{
					velocity = -direction * sqrt(abs(position-desiredPosition)*deceleration*2./3.);
				}
				else {
					velocity += direction*deceleration;
				}
			}
		}
		else {
			if (abs(velocity) <= deceleration) {
				velocity = 0;
			}
			else {
				velocity += sign(velocity)*deceleration;
			}
		}

		if (abs(velocity) > maxSpeed) {
			velocity = sign(velocity) * maxSpeed;
		}

		//TODO: float/int maths consistency between C++ and Lua
		velocity = velocity >= 0 ? floor(velocity) : ceil(velocity);

		return {position + static_cast<int>(velocity), static_cast<int>(velocity)};
	}

	void calculateCollisions(
		mp::std::vector<Collision> &collisions,
		std::vector<ProtoCollision> const &protoCollisions,
		mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
		Frame const *const currentFrame
	)
	{
		auto const calculateCollision = [&triggerArrivals, &currentFrame](ProtoCollision const &protoCollision){
			/*
			//TODO
			if (protoCollision.rawCollisionFunction) {
				return protoCollision.rawCollisionFunction(triggerArrivals, outputTriggers, frameNumber);
			}
			*/
			

			bool const active =
				(protoCollision.hasButtonTriggerID && triggerArrivals[protoCollision.buttonTriggerID][0] != 0)
				|| (protoCollision.hasTriggerClause && (protoCollision.triggerClause.execute(triggerArrivals, getFrameNumber(currentFrame)) != 0))
				;

			CollisionDestination const &destination =
				active ? protoCollision.onDestination : protoCollision.offDestination;

			mp::std::vector<int> const &lastStateTrigger = triggerArrivals[protoCollision.lastStateTriggerID];
			assert(3 < lastStateTrigger.size());
			auto const horizontalPosAndVel = solvePDEquation(
				destination.xDestination, lastStateTrigger[0], lastStateTrigger[2], triggerArrivals, currentFrame);

			auto const verticalPosAndVel = solvePDEquation(
				destination.yDestination, lastStateTrigger[1], lastStateTrigger[3], triggerArrivals, currentFrame);

			return Collision(
				/*int x*/horizontalPosAndVel.position,/*int y*/verticalPosAndVel.position,
				/*int xspeed*/horizontalPosAndVel.velocity, /*int yspeed*/verticalPosAndVel.velocity,
				/*int prevXspeed*/lastStateTrigger[2], /*int prevYspeed*/lastStateTrigger[3],
				/*int width*/protoCollision.width, /*int height*/protoCollision.height,
				protoCollision.collisionType,
				/*TimeDirection timeDirection*/protoCollision.timeDirection
			);
		};

		boost::push_back(collisions, protoCollisions | boost::adaptors::transformed(calculateCollision));
	}

	ArrivalLocation calculateArrivalLocation(PortalArea const &portal)
	{
		return
		{
			portal.getX(),
			portal.getY(),
			portal.getXspeed(),
			portal.getYspeed(),
			portal.getTimeDirection()
		};
	}

	std::tuple<int, int, int, int> /*PositionAndVelocity2D*/ snapAttachment(TimeDirection objectTimeDirection, Attachment const &attachment, mp::std::vector<Collision> const &collisions)
	{
		if (attachment.hasPlatform) {
			//TODO: Properly check/report this assert
			assert(attachment.platform >= 0 && attachment.platform < collisions.size());
			auto const &collision = collisions[attachment.platform];
			if (collision.getTimeDirection() == objectTimeDirection) {
				return {
					collision.getX() + attachment.xOffset,
					collision.getY() + attachment.yOffset,
					collision.getXspeed(),
					collision.getYspeed(),
				};
			}
			else {
				return {
					collision.getX()-collision.getXspeed() + attachment.xOffset,
					collision.getY()-collision.getYspeed() + attachment.yOffset,
					-collision.getXspeed(),
					-collision.getYspeed(),
				};
			}
		}
		else {
			return {
				attachment.xOffset,
				attachment.yOffset,
				0,
				0,
			};
		}
	}

	std::tuple<PortalArea, bool, int> calculatePortal(
		ProtoPortal const &protoPortal,
		mp::std::vector<Collision> const &collisions,
		mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
		Frame const *const currentFrame)
	{
		auto [x, y, xspeed, yspeed] = snapAttachment(protoPortal.timeDirection, protoPortal.attachment, collisions);
		PortalArea retPortal(
			protoPortal.index,//int index,
			x,// int x,
			y,// int y,
			protoPortal.xaim,// int xaim,
			protoPortal.yaim,// int yaim,
			protoPortal.width, //int width,
			protoPortal.height, //int height,
			xspeed,// int xspeed,
			yspeed,// int yspeed,
			protoPortal.collisionOverlap,// int collisionOverlap,
			protoPortal.timeDirection,// TimeDirection timeDirection,
			protoPortal.destinationIndex,// int destinationIndex,
			protoPortal.xDestination,// int xDestination,
			protoPortal.yDestination,// int yDestination,
			protoPortal.relativeTime,// bool relativeTime,
			protoPortal.timeDestination,// int timeDestination,
			protoPortal.relativeDirection,// bool relativeDirection,
			protoPortal.destinationDirection,// TimeDirection destinationDirection,
			protoPortal.illegalDestination,// int illegalDestination,
			protoPortal.fallable,// bool fallable,
			protoPortal.isLaser,// bool isLaser,
			protoPortal.winner// bool winner
		);
		
		int charges = -1;
		if (protoPortal.chargeTriggerID != -1) {
			charges = triggerArrivals[protoPortal.chargeTriggerID][0];
		}
		bool const active = (charges != 0) && (!protoPortal.hasTriggerClause || (protoPortal.triggerClause.execute(triggerArrivals, getFrameNumber(currentFrame)) != 0));

		return {
			retPortal, active, charges
		};
	}

	mt::std::string formatTime(int const frames) {
		std::stringstream ss; //TODO: Use appropriate allocator here!
		
		if (frames%hg::FRAMERATE == 0) {
			ss << floor(frames/static_cast<double>(hg::FRAMERATE)) << "s";
		}
		else {
			double const seconds = frames/static_cast<double>(hg::FRAMERATE);
			ss << std::setprecision(gsl::narrow<std::streamsize>(std::ceil(std::log10(std::abs(seconds)))));
			ss << frames/static_cast<double>(hg::FRAMERATE) << "s";
		}
		//TODO: Use appropriate allocator here too!!
		auto s = ss.str();
		return {s.begin(), s.end()};
	}
	void calculatePortalGlitz(
		PortalArea const &portal,
		mt::std::vector<Glitz> &forwardsGlitz,
		mt::std::vector<Glitz> &reverseGlitz,
		bool const active,
		int const charges)
	{
		#if 0
		local colour
		if active then
			colour = {r = 120, g = 120, b = 120}, {r = 120, g = 120, b = 120}
		else
			colour = {r = 70, g = 70, b = 70}, {r = 70, g = 70, b = 70}
		end

		local forGlitz, revGlitz = calculateBidirectionalGlitz(350, portal, colour, colour)
		
		-- Text does not support changing time directions
		local text
		if portal.winner then
			text = active and "Win" or "Inactive"
		elseif portal.relativeTime and portal.timeDestination > 0 then
			text = "+" .. formatTime(portal.timeDestination)
		else
			text = formatTime(portal.timeDestination)
		end
		
		local textGlitz = {
			type = "text",
			x = portal.x+portal.width/2-1600,
			y = portal.y-2400,
			text = text,
			size = 2000,
			layer = 440,
			colour = {r = 0, g = 0, b = 0},
		}
		
		table.insert(forwardsGlitz, forGlitz)
		table.insert(reverseGlitz, revGlitz)
		table.insert(forwardsGlitz, textGlitz)
		table.insert(reverseGlitz, textGlitz)
		#endif

		auto const colour = active ? asPackedColour(120,120,120) : asPackedColour(70,70,70);
		//TODO: asDynamicArea(portal)?
		auto const [forGlitz, revGlitz] = calculateBidirectionalGlitz(350, portal.getX(), portal.getY(), portal.getXspeed(), portal.getYspeed(), portal.getWidth(), portal.getHeight(), portal.getTimeDirection(), colour, colour);
		
		mt::std::string text;
		if (portal.getWinner()) {
			text = active ? "Win" : "Inactive";
		}
		else if (portal.getRelativeTime() && portal.getTimeDestination() > 0) {
			text = "+" + formatTime(portal.getTimeDestination());
		}
		else {
			text = formatTime(portal.getTimeDestination());
		}

		/*
		int layer,
		mt::std::string text,
		int x, int y,
		int size,
		unsigned colour
		*/
		auto textGlitz = Glitz(mt::std::make_unique<TextGlitz>(
			440,
			text,
			portal.getX() + portal.getWidth()/2-1600,
			portal.getY() - 2400,
			2000,
			asPackedColour(0,0,0)
		));
		forwardsGlitz.push_back(forGlitz);
		reverseGlitz.push_back(revGlitz);

		forwardsGlitz.push_back(textGlitz);
		reverseGlitz.push_back(textGlitz);

		if (charges != -1) {
			std::stringstream ss;
			ss << "Uses " << charges;
			auto s = ss.str();
			mt::std::string text = { s.begin(), s.end() };
			auto chargeText = Glitz(mt::std::make_unique<TextGlitz>(
				440,
				text,
				portal.getX() + portal.getWidth() / 2 - 2700,
				portal.getY() + 200,
				1600,
				asPackedColour(0, 0, 0)
			));
			forwardsGlitz.push_back(chargeText);
			reverseGlitz.push_back(chargeText);
		}
	}

	void calculatePortals(
		mp::std::vector<PortalArea> &portals,
		mp::std::vector<ArrivalLocation> &arrivalLocations,
		mt::std::vector<Glitz> &forwardsGlitz,
		mt::std::vector<Glitz> &reverseGlitz,
		std::vector<ProtoPortal> const &protoPortals,
		mp::std::vector<Collision> const &collisions,
		mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
		mp::std::vector<int> &portalCharges,
		Frame const *const currentFrame
	)
	{
		portalCharges.resize(protoPortals.size());
		for (auto &&protoPortal : protoPortals) {
			auto [portal, active, charges] = calculatePortal(protoPortal, collisions, triggerArrivals, currentFrame);
			if (!protoPortal.isLaser) {
				calculatePortalGlitz(portal, forwardsGlitz, reverseGlitz, active, charges);//TODO
			}
			portalCharges[protoPortal.index] = charges;
			arrivalLocations.push_back(calculateArrivalLocation(portal));
			if (active) {
				portals.push_back(portal);
			}
		}
	}
	void calculateButtonPositionsAndVelocities(
		mp::std::vector<ButtonFrameState> &buttonFrameStates,
		mp::std::vector<Collision> const &collisions
		)
	{
		for (auto &&buttonFrameState : buttonFrameStates) {
			buttonFrameState.calcPnV(collisions);
		}
	}
	void fillCollisionTriggers(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers, std::vector<ProtoCollision> const &protoCollisions, mp::std::vector<Collision> const &collisions) {
		for (std::size_t i{0}, sz{protoCollisions.size()}; i != sz; ++i) {
			assert(collisions.size() == sz);
			auto const &coll = collisions[i];
			outputTriggers[protoCollisions[i].lastStateTriggerID] = mt::std::vector<int>{coll.getX(), coll.getY(), coll.getXspeed(), coll.getYspeed()};
		}
	}

	void calculateButtonStates(
		mp::std::vector<ButtonFrameState> &buttonFrameStates,
		mt::std::map<Frame *, ObjectList<Normal>> const &departures,
		mp::std::vector<mp::std::vector<int>> const &triggerArrivals)
	{
		for (auto &&buttonFrameState : buttonFrameStates) {
			buttonFrameState.updateState(departures, triggerArrivals);
		}
	}

	void calculateMutators(
		mp::std::vector<MutatorArea> &mutators,
		mp::std::vector<MutatorFrameStateImpl *> &activeMutators,
		mp::std::vector<MutatorFrameState> &mutatorFrameStates,
		mp::std::vector<Collision> const &collisions,
		mp::std::vector<mp::std::vector<int>> const &triggerArrivals)
	{
		assert(mutators.empty());
		assert(activeMutators.empty()); 
		for (auto &mutatorFrameState : mutatorFrameStates) {
			mutatorFrameState.addMutator(triggerArrivals, collisions, mutators, activeMutators);
		}
	}

	int calculateSpeedOfTime(
		mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers,
		bool hasSpeedOfTimeTrigger,
		int speedOfTimeTriggerID,
		std::vector<std::pair<int, std::vector<int>>> const &triggerOffsetsAndDefaults)
	{
		if (!hasSpeedOfTimeTrigger) {
			return -1;
		}
		auto const outputTriggerIt{ outputTriggers.find(speedOfTimeTriggerID) };
		if (outputTriggerIt == outputTriggers.end() || outputTriggerIt->second.size() < 1) {
			return triggerOffsetsAndDefaults[speedOfTimeTriggerID].second[0];
		}
		return outputTriggers[speedOfTimeTriggerID][0];
	}

	int calculateParadoxPressure(
		mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers,
		bool hasParadoxPressureTrigger,
		int paradoxPressureTriggerID,
		std::vector<std::pair<int, std::vector<int>>> const &triggerOffsetsAndDefaults)
	{
		if (!hasParadoxPressureTrigger) {
			return -1;
		}
		auto const outputTriggerIt{ outputTriggers.find(paradoxPressureTriggerID) };
		if (outputTriggerIt == outputTriggers.end() || outputTriggerIt->second.size() < 1) {
			return triggerOffsetsAndDefaults[paradoxPressureTriggerID].second[0];
		}
		return outputTriggers[paradoxPressureTriggerID][0];
	}

	SimpleConfiguredTriggerFrameState::SimpleConfiguredTriggerFrameState(
		std::vector<
			std::pair<
				int,
				std::vector<int>
			>
		> const &triggerOffsetsAndDefaults,
		bool hasSpeedOfTimeTrigger,
		int speedOfTimeTriggerID,
		bool hasParadoxPressureTrigger,
		int paradoxPressureTriggerID,
		std::size_t arrivalLocationsSize,
		std::vector<ProtoCollision> const &protoCollisions,
		std::vector<ProtoPortal> const &protoPortals,
		std::vector<ProtoButton> const &protoButtons,
		std::vector<ProtoMutator> const &protoMutators,
		std::vector<ProtoTriggerMod> const &protoTriggerMods,
		std::vector<ProtoGlitz> const &protoGlitzs,
		memory_pool<user_allocator_tbb_alloc> &pool,
		OperationInterrupter &interrupter)
	  :
		pool_(pool),
		interrupter_(interrupter),
		triggerOffsetsAndDefaults_(triggerOffsetsAndDefaults),
		hasSpeedOfTimeTrigger_(hasSpeedOfTimeTrigger),
		speedOfTimeTriggerID_(speedOfTimeTriggerID),
		hasParadoxPressureTrigger_(hasParadoxPressureTrigger),
		paradoxPressureTriggerID_(paradoxPressureTriggerID),
		arrivalLocationsSize_(arrivalLocationsSize),
		outputTriggers_(pool),
		forwardsGlitz_(),
		reverseGlitz_(),
		buttonFrameStates_(pool),
		mutatorFrameStates_(pool),
		activeMutators_(pool),
		triggerArrivals_(pool),
		portalCharges_(pool),
		physicsAffectingStuff_(pool),
		protoCollisions_(protoCollisions),
		protoPortals_(protoPortals),
		protoTriggerMods_(protoTriggerMods),
		protoGlitzs_(protoGlitzs)
	{
		buttonFrameStates_.reserve(protoButtons.size());
		for(auto &&a : protoButtons) buttonFrameStates_.push_back(a.getFrameState(pool_));
		
		mutatorFrameStates_.reserve(protoButtons.size());
		for(auto &&a : protoMutators) mutatorFrameStates_.push_back(a.getFrameState(pool_));
	}


	std::tuple<Glitz, Glitz> calculateCollisionGlitz(Collision const &collision) {
		return calculateBidirectionalGlitz(
			300,
			collision.getX(), collision.getY(), collision.getXspeed(), collision.getYspeed(), collision.getWidth(), collision.getHeight(), collision.getTimeDirection(),
			asPackedColour(50, 0, 0), asPackedColour(0, 0, 50));
	}

	PhysicsAffectingStuff
		SimpleConfiguredTriggerFrameState::calculatePhysicsAffectingStuff(
			Frame const *const currentFrame,
			boost::transformed_range<
				GetBase<TriggerDataConstPtr>,
				mt::boost::container::vector<TriggerDataConstPtr> const> const &triggerArrivals)
	{
#if 0
		local retv = {} //Done

		tempStore.outputTriggers = {}
		tempStore.forwardsGlitz = {}
		tempStore.reverseGlitz = {}
		tempStore.persistentGlitz = {}
		tempStore.frameNumber = frameNumber
		tempStore.triggerArrivals = triggerArrivals

		retv.additionalBoxes = {}
		tempStore.additionalEndBoxes = {}

		if tempStore.triggerManipulationFunction then
			tempStore.triggerManipulationFunction(triggerArrivals, tempStore.outputTriggers, tempStore.frameNumber)
		end

		retv.collisions = calculateCollisions(tempStore.protoCollisions, triggerArrivals, tempStore.outputTriggers, tempStore.frameNumber)//Mostly done

		retv.mutators, tempStore.activeMutators = calculateMutators(tempStore.protoMutators, retv.collisions, triggerArrivals)//mostly done

		local portals, arrivalLocations = calculatePortals(
			tempStore.forwardsGlitz,
			tempStore.reverseGlitz,
			tempStore.protoPortals,
			tempStore.protoLasers,
			retv.collisions,
			triggerArrivals,
			tempStore.frameNumber
		) //done?

		retv.portals = portals //done?
		retv.arrivalLocations = arrivalLocations//done?

		calculateButtonPositionsAndVelocities(tempStore.protoButtons, retv.collisions) //partial done

		fillCollisionTriggers(tempStore.outputTriggers, tempStore.protoCollisions, retv.collisions)//done?

		for collision in list_iter(retv.collisions) do
			local forwardsGlitz, reverseGlitz = calculateCollisionGlitz(collision)
			table.insert(tempStore.forwardsGlitz, forwardsGlitz)
			table.insert(tempStore.reverseGlitz, reverseGlitz)
		end

		if tempStore.protoBoxCreators then
			for protoBoxCreator in list_iter(tempStore.protoBoxCreators) do
				protoBoxCreator:calcPnV(retv.collisions)
				protoBoxCreator : calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz)
				protoBoxCreator : getAdditionalBoxes(tempStore.triggerArrivals, tempStore.frameNumber, retv.additionalBoxes)
			end
		end

		tempStore.physicsAffectingStuff = retv

		return retv
#endif
		triggerArrivals_ = calculateApparentTriggers(triggerOffsetsAndDefaults_, triggerArrivals, pool_);

		/*
		mp::std::vector<Box> additionalBoxes;
		mp::std::vector<PortalArea> portals;
		mp::std::vector<Collision> collisions;
		mp::std::vector<MutatorArea> mutators;
		//guaranteed to always contain elements at each index that could possibly be indexed
		//ie- arrivalLocations will always be the same length for a particular TriggerSystem
		mp::std::vector<ArrivalLocation> arrivalLocations;
		*/
		calculateCollisions(physicsAffectingStuff_.collisions, protoCollisions_, triggerArrivals_, /*outputTriggers,*/ currentFrame);

		calculateMutators(physicsAffectingStuff_.mutators, activeMutators_, mutatorFrameStates_, physicsAffectingStuff_.collisions, triggerArrivals_);

		calculatePortals(
			physicsAffectingStuff_.portals,
			physicsAffectingStuff_.arrivalLocations,
			forwardsGlitz_,
			reverseGlitz_,
			protoPortals_,
			physicsAffectingStuff_.collisions,
			triggerArrivals_,
			portalCharges_,
			currentFrame
		);

		calculateButtonPositionsAndVelocities(buttonFrameStates_, physicsAffectingStuff_.collisions);

		fillCollisionTriggers(outputTriggers_, protoCollisions_, physicsAffectingStuff_.collisions);

		for (auto &&collision : physicsAffectingStuff_.collisions) {
			auto [forwardsGlitz, reverseGlitz] = calculateCollisionGlitz(collision);
			forwardsGlitz_.push_back(std::move(forwardsGlitz));
			reverseGlitz_.push_back(std::move(reverseGlitz));
		}

		return physicsAffectingStuff_;
	}



	TriggerFrameStateImplementation::DepartureInformation SimpleConfiguredTriggerFrameState::getDepartureInformation(
		mt::std::map<Frame*, ObjectList<Normal>> const &departures,
		Frame *currentFrame)
	{
#if 0
		calculateButtonStates(tempStore.protoButtons, departures, tempStore.triggerArrivals)

		for protoButton in list_iter(tempStore.protoButtons) do
			protoButton:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.persistentGlitz)
			protoButton:fillTrigger(tempStore.outputTriggers)
		end

		for protoMutator in list_iter(tempStore.protoMutators) do
			protoMutator:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.persistentGlitz)
			protoMutator:fillTrigger(tempStore.outputTriggers)
		end

		if tempStore.protoGlitz then
			for protoGlitz in list_iter(tempStore.protoGlitz) do
				protoGlitz:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.physicsAffectingStuff, tempStore.triggerArrivals, tempStore.outputTriggers, currentFrame)
			end
		end

		return
			tempStore.outputTriggers,
			tempStore.forwardsGlitz,
			tempStore.reverseGlitz,
			tempStore.persistentGlitz,
			tempStore.additionalEndBoxes
#endif
		//mt::std::vector<Glitz> forwardsGlitz;
		//mt::std::vector<Glitz> reverseGlitz;
		mt::std::vector<GlitzPersister> additionalGlitzPersisters;
		mt::std::vector<ObjectAndTime<Box, Frame *>> additionalBoxDepartures;

		calculateButtonStates(buttonFrameStates_, departures, triggerArrivals_);
		for (auto const &buttonFrameState : buttonFrameStates_) {
			buttonFrameState.calculateGlitz(forwardsGlitz_, reverseGlitz_, additionalGlitzPersisters);
			buttonFrameState.fillTrigger(outputTriggers_);
		}
		for (auto const &mutatorFrameState : mutatorFrameStates_) {
			mutatorFrameState.calculateGlitz(forwardsGlitz_, reverseGlitz_, additionalGlitzPersisters);
			mutatorFrameState.fillTrigger(outputTriggers_);
		}
		for (auto const &protoGlitz : protoGlitzs_) {
			protoGlitz.calculateGlitz(forwardsGlitz_, reverseGlitz_, physicsAffectingStuff_, triggerArrivals_, outputTriggers_, currentFrame);
		}
		for (auto const  &protoTriggerMod : protoTriggerMods_) {
			protoTriggerMod.modifyTrigger(triggerArrivals_, outputTriggers_, triggerOffsetsAndDefaults_, currentFrame);
		}

		for (auto const &protoPortal : protoPortals_) {
			int triggerID = protoPortal.chargeTriggerID;
			if (triggerID != -1) {
				outputTriggers_[triggerID] = mt::std::vector<int>{ portalCharges_[protoPortal.index] };
			}
		}

		mp::std::vector<TriggerData> triggerVector(pool_);
		boost::push_back(triggerVector, outputTriggers_ | boost::adaptors::transformed([](auto &&v) {return TriggerData{v.first, std::move(v.second)};}));

		return {
			calculateActualTriggerDepartures(triggerVector, triggerOffsetsAndDefaults_, currentFrame),
			forwardsGlitz_,
			reverseGlitz_,
			additionalGlitzPersisters,
			additionalBoxDepartures, 
			calculateSpeedOfTime(outputTriggers_, hasSpeedOfTimeTrigger_, speedOfTimeTriggerID_, triggerOffsetsAndDefaults_),
			calculateParadoxPressure(outputTriggers_, hasParadoxPressureTrigger_, paradoxPressureTriggerID_, triggerOffsetsAndDefaults_) };
	}


	bool SimpleConfiguredTriggerFrameState::shouldArrive(Guy const &potentialArriver) { return true; }
	bool SimpleConfiguredTriggerFrameState::shouldArrive(Box const &potentialArriver) { return true; }

	bool SimpleConfiguredTriggerFrameState::shouldPort(
		int responsiblePortalIndex,
		Guy const &potentialPorter,
		bool porterActionedPortal) 
	{
		if (portalCharges_[responsiblePortalIndex] == 0) {
			return false;
		}
		else if (portalCharges_[responsiblePortalIndex] > 0) {
			portalCharges_[responsiblePortalIndex] -= 1;
		}
		return true; 
	}

	bool SimpleConfiguredTriggerFrameState::shouldPort(
		int responsiblePortalIndex,
		Box const &potentialPorter,
		bool porterActionedPortal) { return true; }

	template<typename T>
	boost::optional<T> mutateObjectGeneric(
		mp::std::vector<int> const &responsibleMutatorIndices,
		T const &objectToManipulate,
		mp::std::vector<MutatorFrameStateImpl *> const &activeMutators)
	{
		boost::optional<T> dynamicObject{objectToManipulate};
		for (auto i : responsibleMutatorIndices) {
			assert(i >= 0 && i < activeMutators.size());
			dynamicObject = activeMutators[i]->effect(*dynamicObject);

			if (!dynamicObject) {
				break;
			}
		}
		return dynamicObject;
	}

	boost::optional<Guy> SimpleConfiguredTriggerFrameState::mutateObject(
		mp::std::vector<int> const &responsibleMutatorIndices,
		Guy const &objectToManipulate)
	{
		return mutateObjectGeneric(responsibleMutatorIndices, objectToManipulate, activeMutators_);
	}
	boost::optional<Box> SimpleConfiguredTriggerFrameState::mutateObject(
		mp::std::vector<int> const &responsibleMutatorIndices,
		Box const &objectToManipulate)
	{
		return mutateObjectGeneric(responsibleMutatorIndices, objectToManipulate, activeMutators_);
	}

	SimpleConfiguredTriggerFrameState::~SimpleConfiguredTriggerFrameState() noexcept {}


	template<>
	Attachment to<Attachment>(lua_State *L, int index) {
		try {
			if (!lua_istable(L, index)) {
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Attachments must be tables"));
			}

			lua_getfield(L, index, "platform");
			bool hasPlatform(!lua_isnil(L, -1));
			int platform(hasPlatform ?lua_index_to_C_index(to<int>(L)):0);
			lua_pop(L, 1);

			int xOffset(readField<int>(L, "xOffset", index));
			int yOffset(readField<int>(L, "yOffset", index));
			return {
				hasPlatform,
				platform,
				xOffset,
				yOffset
			};
		}
		catch (LuaError &e) {
			add_semantic_callstack_info(e, "to<Attachment>");
			throw;
		}
	}

	ProtoPortal toProtoPortal(lua_State *L, std::size_t arrivalLocationsSize)
	{
		LuaStackManager stack_manager(L);
		if (!lua_istable(L, -1)) {
			BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("a protoPortal must be a table"));
		}
		#if 0
		struct ProtoPortal {
			Attachment attachment;
			int index;
			int xaim;
			int yaim;
			int width;
			int height;
			int collisionOverlap;
			TimeDirection timeDirection;
			int destinationIndex;
			int xDestination;
			int yDestination;
			bool relativeTime;
			int timeDestination;
			bool relativeDirection;
			TimeDirection destinationDirection;
			bool hasTriggerClause;
			TriggerClause triggerClause;
			int illegalDestination;
			bool fallable;
			bool isLaser;
			bool winner;
		};
		#endif
		Attachment const attachment(readField<Attachment>(L, "attachment"));
		int const index(lua_index_to_C_index(readField<int>(L, "index")));
		int const width(readField<int>(L, "width"));
		int const height(readField<int>(L, "height"));
		int const collisionOverlap(readField<int>(L, "collisionOverlap"));
		TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));
		int destinationIndex(-1);
		luaL_checkstack(L, 1, nullptr);
		lua_getfield(L, -1, "destinationIndex");
		if (!lua_isnil(L, -1)) {
			destinationIndex = lua_index_to_C_index(to<int>(L));
			if (!(destinationIndex >= 0 && static_cast<std::size_t>(destinationIndex) < arrivalLocationsSize))
			{
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("destinationIndex must be nil, or in the range [1, arrivalLocationsSize+1)"));
			}
		}
		lua_pop(L, 1);
		int const xDestination(readField<int>(L, "xDestination"));
		int const yDestination(readField<int>(L, "yDestination"));
		bool const relativeTime(readField<bool>(L, "relativeTime"));
		int const timeDestination(readField<int>(L, "timeDestination"));
		bool const relativeDirection(readFieldWithDefault<bool>(L, "relativeDirection", -1, true));
		TimeDirection const destinationDirection(readFieldWithDefault<TimeDirection>(L, "destinationDirection", -1, TimeDirection::FORWARDS));
		
		lua_getfield(L, -1, "triggerClause");
		bool const hasTriggerClause(!lua_isnil(L, -1));
		std::string triggerClauseString(hasTriggerClause ? to<std::string>(L) : "0");
		TriggerClause triggerClause(triggerClauseString);
		lua_pop(L, 1);

		//TODO: Centralise this logic?
		lua_getfield(L, -1, "illegalDestination");
		int illegalDestination;
		if (lua_isnumber(L, -1)) {
			illegalDestination = lua_index_to_C_index(static_cast<int>(std::round(lua_tonumber(L, -1))));
			if (illegalDestination < 0)
			{
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Proto Portal's illegalDestination must be >= 1"));
			}
		}
		else {
			if (!lua_isnil(L, -1))
			{
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Proto Portal's illegalDestination must be an integer, or nil."));
			}
			illegalDestination = -1;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "chargeTriggerID");
		int const chargeTriggerID = (!lua_isnil(L, -1) ? lua_index_to_C_index(to<int>(L)) : -1);
		lua_pop(L, 1);

		bool const fallable(readField<bool>(L, "fallable"));
		bool const isLaser(readFieldWithDefault<bool>(L, "isLaser", -1, false));
		int const xaim(isLaser?readField<int>(L, "xaim"):0);
		int const yaim(isLaser?readField<int>(L, "yaim"):0);
		bool const winner(readField<bool>(L, "winner"));
		return {
			attachment,
			index,
			xaim,
			yaim,
			width,
			height,
			collisionOverlap,
			timeDirection,
			destinationIndex,
			xDestination,
			yDestination,
			relativeTime,
			timeDestination,
			relativeDirection,
			destinationDirection,
			illegalDestination,
			chargeTriggerID,
			hasTriggerClause,
			triggerClause,
			fallable,
			isLaser,
			winner
		};
	}

	ProtoMutator toProtoPickup(
		lua_State * const L,
		std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
	{
		TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));
		Attachment const attachment(readField<Attachment>(L, "attachment"));
		int const width(readField<int>(L, "width"));
		int const height(readField<int>(L, "height"));
		Ability const pickupType(readField<Ability>(L, "pickupType"));
		int const pickupNumber(readFieldWithDefault<int>(L, "pickupNumber", -1, 1));
		int const triggerID(lua_index_to_C_index(readField<int>(L, "triggerID")));
		assert(triggerID < triggerOffsetsAndDefaults.size());
		assert(0 < triggerOffsetsAndDefaults[triggerID].second.size());
		return ProtoMutator(mt::std::make_unique<ProtoPickupImpl>(
			timeDirection,
			attachment,
			width,
			height,
			pickupType,
			pickupNumber,
			triggerID
		));
	}

	ProtoMutator toProtoSpikes(
		lua_State * const L)
	{
		TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));
		Attachment const attachment(readField<Attachment>(L, "attachment"));
		int const width(readField<int>(L, "width"));
		int const height(readField<int>(L, "height"));
		return ProtoMutator(mt::std::make_unique<ProtoSpikesImpl>(
			timeDirection,
			attachment,
			width,
			height
		));
	}
	template<>
	Powerup to<Powerup>(lua_State *L, int index) {
		try {
			//TODO abstract and combine this with to<TimeDirection>, to<FacingDirection> and to<Ability>
			if (!lua_isstring(L, index)) {
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Powerup values must be strings"));
			}
			char const * const powerupString(lua_tostring(L, index));
			if (strcmp(powerupString, "jumpBoots") == 0) {
				return Powerup::JUMP_BOOTS;
			}
			else {
				std::stringstream ss;
				ss << "invalid powerup string: " << powerupString;
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info(ss.str()));
			}
		}
		catch (LuaError &e) {
			add_semantic_callstack_info(e, "to<Powerup>");
			throw;
		}
	}
	ProtoMutator toProtoPowerup(
		lua_State *const L,
		std::vector<std::pair<int, std::vector<int>>> const &triggerOffsetsAndDefaults)
	{
		int const triggerID(lua_index_to_C_index(readField<int>(L, "triggerID")));
		assert(triggerID < triggerOffsetsAndDefaults.size());
		return ProtoMutator(
			mt::std::make_unique<ProtoPowerupImpl>(
				readField<Powerup>(L, "powerupType"),
				readField<TimeDirection>(L, "timeDirection"),
				readField<Attachment>(L, "attachment"),
				readField<int>(L, "width"),
				readField<int>(L, "height"),
				triggerID
			)
		);
	}

	ProtoMutator toProtoMutator(
		lua_State * const L,
		std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults) {
		std::string const type(readField<std::string>(L, "type"));
		if (type == "pickup") {
			return toProtoPickup(L, triggerOffsetsAndDefaults);
		}
		else if (type == "spikes") {
			return toProtoSpikes(L);
		}
		else if (type == "powerup") {
			return toProtoPowerup(L, triggerOffsetsAndDefaults);
		}
		else {
			BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info(R"(ProtoMutators must have a type in {"pickup", "spikes", "powerup"})"));
		}
	}

	
	template<>
	AxisCollisionDestination to<AxisCollisionDestination>(lua_State *L, int index) {
		try {
			if (!lua_istable(L, index)) {
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("AxisCollisionDestinations must be tables"));
			}

			lua_getfield(L, -1, "desiredPosition");
			bool const hasClauseDesiredPosition(!lua_isnumber(L, -1));
			int const desiredPosition(hasClauseDesiredPosition ? 0 : to<int>(L));
			std::string clauseDesiredPositionString(hasClauseDesiredPosition ? to<std::string>(L) : "0");
			TriggerClause clauseDesiredPosition(clauseDesiredPositionString);
			lua_pop(L, 1);

			lua_getfield(L, -1, "acceleration");
			bool const hasClauseAcceleration(!lua_isnumber(L, -1));
			int const acceleration(hasClauseAcceleration ? 0 : to<int>(L));
			std::string clauseAccelerationString(hasClauseAcceleration ? to<std::string>(L) : "0");
			TriggerClause clauseAcceleration(clauseAccelerationString);
			lua_pop(L, 1);

			lua_getfield(L, -1, "deceleration");
			bool const hasClauseDeceleration(!lua_isnumber(L, -1));
			int const deceleration(hasClauseDeceleration ? 0 : to<int>(L));
			std::string clauseDecelerationString(hasClauseDeceleration ? to<std::string>(L) : "0");
			TriggerClause clauseDeceleration(clauseDecelerationString);
			lua_pop(L, 1);

			lua_getfield(L, -1, "maxSpeed");
			bool const hasClauseMaxSpeed(!lua_isnumber(L, -1));
			int const maxSpeed(hasClauseMaxSpeed ? 0 : to<int>(L));
			std::string clauseMaxSpeedString(hasClauseMaxSpeed ? to<std::string>(L) : "0");
			TriggerClause clauseMaxSpeed(clauseMaxSpeedString);
			lua_pop(L, 1);

			return {
				desiredPosition,
				acceleration,
				deceleration,
				maxSpeed,
				hasClauseDesiredPosition,
				hasClauseAcceleration,
				hasClauseDeceleration,
				hasClauseMaxSpeed,
				clauseDesiredPosition,
				clauseAcceleration,
				clauseDeceleration,
				clauseMaxSpeed
			};
		}
		catch (LuaError &e) {
			add_semantic_callstack_info(e, "to<AxisCollisionDestination>");
			throw;
		}
	}

	template<>
	CollisionDestination to<CollisionDestination>(lua_State *L, int index) {
		try {
			if (!lua_istable(L, index)) {
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("CollisionDestinations must be tables"));
			}

			return {
				readField<AxisCollisionDestination>(L, "xDestination", index),
				readField<AxisCollisionDestination>(L, "yDestination", index)
			};
		}
		catch (LuaError &e) {
			add_semantic_callstack_info(e, "to<CollisionDestination>");
			throw;
		}
	}

	ProtoCollision toProtoCollision(
		lua_State * const L,
		std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
	{

		TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));
		int const width(readField<int>(L, "width"));
		int const height(readField<int>(L, "height"));
		CollisionType const collisionType(readField<CollisionType>(L, "collisionType"));

		lua_getfield(L, -1, "destinations");
		assert(lua_istable(L, -1));
		CollisionDestination const onDestination(readField<CollisionDestination>(L, "onDestination"));
		CollisionDestination const offDestination(readField<CollisionDestination>(L, "offDestination"));
		lua_pop(L, 1);

		lua_getfield(L, -1, "buttonTriggerID");
		bool const hasButtonTriggerID(!lua_isnil(L, -1));
		int buttonTriggerID(hasButtonTriggerID ?lua_index_to_C_index(to<int>(L)):0);
		lua_pop(L, 1);

		lua_getfield(L, -1, "triggerClause");
		bool const hasTriggerClause(!lua_isnil(L, -1));
		std::string triggerClauseString(hasTriggerClause ? to<std::string>(L) : "0");
		TriggerClause triggerClause(triggerClauseString);
		lua_pop(L, 1);

		int const lastStateTriggerID(lua_index_to_C_index(readField<int>(L, "lastStateTriggerID")));
		assert(lastStateTriggerID < triggerOffsetsAndDefaults.size());
		assert(3 < triggerOffsetsAndDefaults[lastStateTriggerID].second.size());//Must have room for [x, y, xspeed, yspeed]

		return {
			timeDirection,
			width,
			height,
			collisionType,
			onDestination,
			offDestination,
			hasButtonTriggerID,
			buttonTriggerID,
			hasTriggerClause,
			triggerClause,
			lastStateTriggerID
		};
	}

	ProtoTriggerMod toProtoTriggerMod(
		lua_State * const L,
		std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
	{
		lua_getfield(L, -1, "triggerID");
		assert(!lua_isnil(L, -1) && "TriggerMod missing triggerID.");
		int triggerID(lua_index_to_C_index(to<int>(L)));
		lua_pop(L, 1);

		assert(triggerID < triggerOffsetsAndDefaults.size());
		assert(0 < triggerOffsetsAndDefaults[triggerID].second.size());

		lua_getfield(L, -1, "triggerClause");
		assert(!lua_isnil(L, -1) && "TriggerMod missing triggerClause.");
		std::string triggerClauseString(to<std::string>(L));
		TriggerClause triggerClause(triggerClauseString);
		lua_pop(L, 1);

		int const triggerSubindex(lua_index_to_C_index(readFieldWithDefault<int>(L, "triggerSubindex", -1, 1)));

		return {
			triggerID,
			triggerSubindex,
			triggerClause
		};
	}

	ProtoButton toProtoMomentarySwitch(
		lua_State * const L,
		std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
	{
		TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));
		Attachment const attachment(readField<Attachment>(L, "attachment"));
		int const width(readField<int>(L, "width"));
		int const height(readField<int>(L, "height"));
		int const triggerID(lua_index_to_C_index(readField<int>(L, "triggerID")));
		int const stateTriggerID(lua_index_to_C_index(readFieldWithDefault<int>(L, "stateTriggerID", -1, gsl::narrow<int>(C_index_to_lua_index(triggerID)))));

		assert(triggerID < triggerOffsetsAndDefaults.size());
		assert(0 < triggerOffsetsAndDefaults[triggerID].second.size());

		assert(stateTriggerID < triggerOffsetsAndDefaults.size());
		assert(0 < triggerOffsetsAndDefaults[stateTriggerID].second.size());

		std::vector<int> extraTriggerIDs;
		lua_getfield(L, -1, "extraTriggerIDs");
		if (!lua_isnil(L, -1)) {
			assert(lua_istable(L, -1));

			lua_len(L, -1);
			lua_Integer const tablelen(lua_tointeger(L, -1));
			lua_pop(L, 1);
			extraTriggerIDs.reserve(tablelen);

			for (lua_Integer i(0), end(tablelen); i != end; ++i) {
				lua_rawgeti(L, -1, i + 1);
				extraTriggerIDs.push_back(lua_index_to_C_index(to<int>(L, -1)));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		return ProtoButton(mt::std::make_unique<ProtoMomentarySwitchImpl>(
			timeDirection,
			attachment,
			width,
			height,
			triggerID,
			stateTriggerID,
			std::move(extraTriggerIDs)
		));
	}

	ProtoButton toProtoStickySwitch(
		lua_State * const L,
		std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
	{

		//TODO: extract this into shared function with toProtoMomentarySwitch

		TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));
		Attachment const attachment(readField<Attachment>(L, "attachment"));
		int const width(readField<int>(L, "width"));
		int const height(readField<int>(L, "height"));
		int const triggerID(lua_index_to_C_index(readField<int>(L, "triggerID")));
		int const stateTriggerID(lua_index_to_C_index(readFieldWithDefault<int>(L, "stateTriggerID", -1, gsl::narrow<int>(C_index_to_lua_index(triggerID)))));

		assert(triggerID < triggerOffsetsAndDefaults.size());
		assert(0 < triggerOffsetsAndDefaults[triggerID].second.size());

		assert(stateTriggerID < triggerOffsetsAndDefaults.size());
		assert(0 < triggerOffsetsAndDefaults[stateTriggerID].second.size());

		std::vector<int> extraTriggerIDs;
		lua_getfield(L, -1, "extraTriggerIDs");
		if (!lua_isnil(L, -1)) {
			assert(lua_istable(L, -1));

			lua_len(L, -1);
			lua_Integer const tablelen(lua_tointeger(L, -1));
			lua_pop(L, 1);
			extraTriggerIDs.reserve(tablelen);

			for (lua_Integer i(0), end(tablelen); i != end; ++i) {
				lua_rawgeti(L, -1, i + 1);
				extraTriggerIDs.push_back(lua_index_to_C_index(to<int>(L, -1)));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		return ProtoButton(mt::std::make_unique<ProtoStickySwitchImpl>(
			timeDirection,
			attachment,
			width,
			height,
			triggerID,
			stateTriggerID,
			std::move(extraTriggerIDs)
		));
	}

	template<>
	ButtonSegment to<ButtonSegment>(lua_State *L, int index) {
		try {
			if (!lua_istable(L, index)) {
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("ButtonSegments must be tables"));
			}

			return {
				readField<Attachment>(L, "attachment", index),
				readField<int>(L, "width", index),
				readField<int>(L, "height", index)
			};
		}
		catch (LuaError &e) {
			add_semantic_callstack_info(e, "to<ButtonSegment>");
			throw;
		}
	}


	ProtoButton toProtoToggleSwitch(
		lua_State * const L,
		std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
	{
		TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));

		int const triggerID(lua_index_to_C_index(readField<int>(L, "triggerID")));
		int const stateTriggerID(lua_index_to_C_index(readFieldWithDefault<int>(L, "stateTriggerID", -1, gsl::narrow<int>(C_index_to_lua_index(triggerID)))));

		assert(triggerID < triggerOffsetsAndDefaults.size());
		assert(0 < triggerOffsetsAndDefaults[triggerID].second.size());

		assert(stateTriggerID < triggerOffsetsAndDefaults.size());
		assert(0 < triggerOffsetsAndDefaults[stateTriggerID].second.size());

		std::vector<int> extraTriggerIDs;
		lua_getfield(L, -1, "extraTriggerIDs");
		if (!lua_isnil(L, -1)) {
			assert(lua_istable(L, -1));

			lua_len(L, -1);
			lua_Integer const tablelen(lua_tointeger(L, -1));
			lua_pop(L, 1);
			extraTriggerIDs.reserve(tablelen);

			for (lua_Integer i(0), end(tablelen); i != end; ++i) {
				lua_rawgeti(L, -1, i + 1);
				extraTriggerIDs.push_back(lua_index_to_C_index(to<int>(L, -1)));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		auto const first{readField<ButtonSegment>(L, "first")};
		auto const second{readField<ButtonSegment>(L, "second")};

		return ProtoButton(mt::std::make_unique<ProtoToggleSwitchImpl>(
			timeDirection,
			first,
			second,
			triggerID,
			stateTriggerID,
			std::move(extraTriggerIDs)
		));
	}

	ProtoButton toProtoMultiStickySwitch(
		lua_State * const L,
		std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
	{
		TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));

		int const triggerID(lua_index_to_C_index(readField<int>(L, "triggerID")));
		int const stateTriggerID(lua_index_to_C_index(readField<int>(L, "stateTriggerID")));

		assert(triggerID < triggerOffsetsAndDefaults.size());
		assert(0 < triggerOffsetsAndDefaults[triggerID].second.size());

		assert(stateTriggerID < triggerOffsetsAndDefaults.size());

		std::vector<int> extraTriggerIDs;
		lua_getfield(L, -1, "extraTriggerIDs");
		if (!lua_isnil(L, -1)) {
			assert(lua_istable(L, -1));

			lua_len(L, -1);
			lua_Integer const tablelen(lua_tointeger(L, -1));
			lua_pop(L, 1);
			extraTriggerIDs.reserve(tablelen);

			for (lua_Integer i(0), end(tablelen); i != end; ++i) {
				lua_rawgeti(L, -1, i + 1);
				extraTriggerIDs.push_back(lua_index_to_C_index(to<int>(L, -1)));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);


		std::vector<ButtonSegment> buttons;
		lua_getfield(L, -1, "buttons");
		if (!lua_isnil(L, -1)) {
			assert(lua_istable(L, -1));

			lua_len(L, -1);
			lua_Integer const tablelen(lua_tointeger(L, -1));
			lua_pop(L, 1);
			buttons.reserve(tablelen);

			for (lua_Integer i(0), end(tablelen); i != end; ++i) {
				lua_rawgeti(L, -1, i + 1);
				buttons.push_back(to<ButtonSegment>(L, -1));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		assert(buttons.size() <= triggerOffsetsAndDefaults[stateTriggerID].second.size());

		return ProtoButton(mt::std::make_unique<ProtoMultiStickySwitchImpl>(
			timeDirection,
			std::move(buttons),
			triggerID,
			stateTriggerID,
			std::move(extraTriggerIDs)
		));
	}


	ProtoButton toProtoStickyLaserSwitch(
		lua_State * const L,
		std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
	{
		/*
		type = 'stickyLaserSwitch',
		timeDirection = p.timeDirection
		attachment = cloneAttachment(p.attachment)
		beamLength = p.beamLength
		beamDirection = p.beamDirection
		triggerID = p.triggerID,
		stateTriggerID = p.stateTriggerID,
		extraTriggerIDs = p.extraTriggerIDs,
		*/

		TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));

		Attachment const attachment(readField<Attachment>(L, "attachment"));
		int const beamLength(readField<int>(L, "beamLength"));
		int const beamDirection(readField<int>(L, "beamDirection"));

		int const triggerID(lua_index_to_C_index(readField<int>(L, "triggerID")));
		int const stateTriggerID(lua_index_to_C_index(readFieldWithDefault<int>(L, "stateTriggerID", -1, gsl::narrow<int>(C_index_to_lua_index(triggerID)))));

		assert(triggerID < triggerOffsetsAndDefaults.size());
		assert(stateTriggerID < triggerOffsetsAndDefaults.size());

		std::vector<int> extraTriggerIDs;
		lua_getfield(L, -1, "extraTriggerIDs");
		if (!lua_isnil(L, -1)) {
			assert(lua_istable(L, -1));

			lua_len(L, -1);
			lua_Integer const tablelen(lua_tointeger(L, -1));
			lua_pop(L, 1);
			extraTriggerIDs.reserve(tablelen);

			for (lua_Integer i(0), end(tablelen); i != end; ++i) {
				lua_rawgeti(L, -1, i + 1);
				extraTriggerIDs.push_back(lua_index_to_C_index(to<int>(L, -1)));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		return ProtoButton(mt::std::make_unique<ProtoStickyLaserSwitchImpl>(
			timeDirection,
			attachment,
			beamLength,
			beamDirection,
			triggerID,
			stateTriggerID,
			std::move(extraTriggerIDs)
		));
	}


	ProtoButton toProtoButton(
		lua_State * const L,
		std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
	{
		std::string const type(readField<std::string>(L, "type"));
		if (type == "momentarySwitch") {
			return toProtoMomentarySwitch(L, triggerOffsetsAndDefaults);
		}
		else if (type == "stickySwitch") {
			return toProtoStickySwitch(L, triggerOffsetsAndDefaults);
		}
		else if (type == "toggleSwitch") {
			return toProtoToggleSwitch(L, triggerOffsetsAndDefaults);
		}
		else if (type == "multiStickySwitch") {
			return toProtoMultiStickySwitch(L, triggerOffsetsAndDefaults);
		}
		else if (type == "stickyLaserSwitch") {
			return toProtoStickyLaserSwitch(L, triggerOffsetsAndDefaults);
		}
		else {
			BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info(R"(ProtoButton must have a type in {"momentarySwitch", "stickySwitch", "toggleSwitch", "multiStickySwitch", "stickyLaserSwitch"})"));
		}
	}


	template<typename T, typename F>
	auto optionalMap(std::optional<T> const &o, F &&f) -> std::optional<decltype(f(*o))>{
		if (o) {
			return std::optional<decltype(f(*o))>{f(*o)};
		}
		else {
			return std::optional<decltype(f(*o))>{};
		}
	}

	template<>
	PlatformAndPos to<PlatformAndPos>(lua_State *L, int index) {
		try
		{
			return { optionalMap(readFieldOptional<int>(L, "platform", index), &lua_index_to_C_index<int>), readField<int>(L, "pos", index) };
		}
		catch (LuaError &e) {
			add_semantic_callstack_info(e, "to<PlatformAndPos>");
			throw;
		}
	}

	ProtoGlitz toProtoWireGlitz(
		lua_State * const L)
	{
		std::optional<int> const triggerID{ optionalMap(readFieldOptional<int>(L, "triggerID"), &lua_index_to_C_index<int>) }; 

		lua_getfield(L, -1, "triggerClause");
		bool const hasTriggerClause(!lua_isnil(L, -1));
		std::string triggerClauseString(hasTriggerClause ? to<std::string>(L) : "0");
		TriggerClause triggerClause(triggerClauseString);
		lua_pop(L, 1);

		bool const useTriggerArrival{readField<bool>(L, "useTriggerArrival")};
		PlatformAndPos const x1{readField<PlatformAndPos>(L, "x1")};
		PlatformAndPos const y1{readField<PlatformAndPos>(L, "y1")};
		PlatformAndPos const x2{readField<PlatformAndPos>(L, "x2")};
		PlatformAndPos const y2{readField<PlatformAndPos>(L, "y2")};

		return ProtoGlitz(mt::std::make_unique<ProtoWireGlitzImpl>(
			triggerID,
			hasTriggerClause,
			triggerClause,
			useTriggerArrival,
			x1,
			y1,
			x2,
			y2
			));
	}

	ProtoGlitz toProtoBasicRectangleGlitz(
		lua_State * const L)
	{
		unsigned const colour{ readColourField(L, "colour") };
		int const layer{ readField<int>(L, "layer") };
		int const x{ readField<int>(L, "x") };
		int const y{ readField<int>(L, "y") };
		int const width{ readField<int>(L, "width") };
		int const height{ readField<int>(L, "height") };

		return ProtoGlitz(mt::std::make_unique<ProtoBasicRectangleGlitzImpl>(
			colour,
			layer,
			x,
			y,
			width,
			height
			));
	}
	ProtoGlitz toProtoBasicTextGlitz(
		lua_State * const L)
	{
		int const layer(readField<int>(L, "layer"));
		auto const text(readField<mt::std::string>(L, "text"));
		int const x(readField<int>(L, "x"));
		int const y(readField<int>(L, "y"));
		int const size(readField<int>(L, "size"));
		unsigned const colour(readColourField(L, "colour"));

		return ProtoGlitz(mt::std::make_unique<ProtoBasicTextGlitzImpl>(
			TextGlitz(layer, std::move(text), x, y, size, colour)));
	}
	ProtoGlitz toProtoGlitz(
		lua_State * const L)
	{
		std::string const type(readField<std::string>(L, "type"));
		if (type == "wireGlitz") {
			return toProtoWireGlitz(L);
		}
		else if (type == "basicRectangleGlitz"){
			return toProtoBasicRectangleGlitz(L);
		}
		else if (type == "basicTextGlitz") {
			return toProtoBasicTextGlitz(L);
		}
		else {
			BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info(R"(ProtoGlitz must have a type in {"wireGlitz", "basicRectangleGlitz", "basicTextGlitz"})"));
		}
	}


	SimpleConfiguredTriggerSystem::SimpleConfiguredTriggerSystem(
			std::vector<char> const &mainChunk,
			std::vector<LuaModule> const &extraChunks,
			std::vector<
				std::pair<
					int,
					std::vector<int>
				>
			> triggerOffsetsAndDefaults,
			bool hasSpeedOfTimeTrigger,
			int speedOfTimeTriggerID,
			bool hasParadoxPressureTrigger,
			int paradoxPressureTriggerID,
			std::size_t arrivalLocationsSize)
		:
		protoPortals_(),
		protoCollisions_(),
		protoMutators_(),
		protoButtons_(),
		protoTriggerMods_(),
		triggerOffsetsAndDefaults_(std::move(triggerOffsetsAndDefaults)),
		hasSpeedOfTimeTrigger_(hasSpeedOfTimeTrigger),
		speedOfTimeTriggerID_(speedOfTimeTriggerID),
		hasParadoxPressureTrigger_(hasParadoxPressureTrigger),
		paradoxPressureTriggerID_(paradoxPressureTriggerID),
		arrivalLocationsSize_(arrivalLocationsSize)
	{
		//#if 0
		LuaState triggerSystemGenerator((LuaState::new_state_t()));

		lua_State *const L(triggerSystemGenerator.ptr);
		//The use of the split sandboxing system isn't stricly necessary here,
		//since this is only ever called once; but it's easier than writing a whole new non-split
		//sandboxing system.
		loadSandboxedLibraries(L);
		pushFunctionFromVector(L, mainChunk, "triggerSystem");
		setUpPreloadResetFunction(L, extraChunks);

		//LuaInterruptionHandle h(makeInterruptable(L, interrupter));//TODO

		//LuaStackManager stackSaver(L); ?
		//checkstack(L, 1);
		restoreGlobals(L);

		lua_call(L, 0, 0);

		LuaStackManager stackSaver(L);
		//push function to call
		luaL_checkstack(L, 1, nullptr);
		lua_getglobal(L, "calculatePhysicsAffectingStuff");

		//TODO: Extract this 'Read table' code into a function
		//read 'protoPortals' table
		luaL_checkstack(L, 1, nullptr);
		lua_getfield(L, -1, "protoPortals");
		if (!lua_isnil(L, -1)) {
			if (!lua_istable(L, -1)) {
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("protoPortals must be a table"));
			}
			for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
				luaL_checkstack(L, 1, nullptr);
				lua_pushinteger(L, i);
				lua_gettable(L, -2);
				protoPortals_.push_back(toProtoPortal(L, arrivalLocationsSize_));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);


		lua_getfield(L, -1, "protoMutators");
		if (!lua_isnil(L, -1)) {
			if (!lua_istable(L, -1)) {
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("protoMutators must be a table"));
			}
			for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
				luaL_checkstack(L, 1, nullptr);
				lua_pushinteger(L, i);
				lua_gettable(L, -2);
				protoMutators_.push_back(toProtoMutator(L, triggerOffsetsAndDefaults_));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "protoCollisions");
		if (!lua_isnil(L, -1)) {
			if (!lua_istable(L, -1)) {
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("protoCollisions must be a table"));
			}
			for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
				luaL_checkstack(L, 1, nullptr);
				lua_pushinteger(L, i);
				lua_gettable(L, -2);
				protoCollisions_.push_back(toProtoCollision(L, triggerOffsetsAndDefaults_));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "protoButtons");
		if (!lua_isnil(L, -1)) {
			if (!lua_istable(L, -1)) {
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("protoButtons must be a table"));
			}
			for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
				luaL_checkstack(L, 1, nullptr);
				lua_pushinteger(L, i);
				lua_gettable(L, -2);
				protoButtons_.push_back(toProtoButton(L, triggerOffsetsAndDefaults_));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "protoTriggerMods");
		if (!lua_isnil(L, -1)) {
			if (!lua_istable(L, -1)) {
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("protoTriggerMods must be a table"));
			}
			for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
				luaL_checkstack(L, 1, nullptr);
				lua_pushinteger(L, i);
				lua_gettable(L, -2);
				protoTriggerMods_.push_back(toProtoTriggerMod(L, triggerOffsetsAndDefaults_));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "protoGlitz");
		if (!lua_isnil(L, -1)) {
			if (!lua_istable(L, -1)) {
				BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("protoGlitz must be a table"));
			}
			for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
				luaL_checkstack(L, 1, nullptr);
				lua_pushinteger(L, i);
				lua_gettable(L, -2);
				protoGlitzs_.push_back(toProtoGlitz(L));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);
		//TODO...
		;

		//Also
		//lua_getglobal mutators, shouldport, shouldarrive, etc

		//#endif
	}
	void PickupFrameStateImpl::calculateGlitz(
			mt::std::vector<Glitz> &forwardsGlitz,
			mt::std::vector<Glitz> &reverseGlitz,
			mt::std::vector<GlitzPersister> &persistentGlitz) const
	{
		assert(proto);

		static std::map<Ability, mt::std::string> const pickupGlitzNameMap{
			{Ability::TIME_JUMP, "global.time_jump"},
			{Ability::TIME_REVERSE, "global.time_reverse"},
			{Ability::TIME_GUN, "global.time_gun"},
			{Ability::TIME_PAUSE, "global.time_pause"}
		};

		assert(proto->pickupType != Ability::NO_ABILITY);
		auto pickupGlitzMapIt = pickupGlitzNameMap.find(proto->pickupType);
		if (active && pickupGlitzMapIt != pickupGlitzNameMap.end()) {
			forwardsGlitz.push_back(
				Glitz(mt::std::make_unique<ImageGlitz>(
					430,
					pickupGlitzMapIt->second,
					x_, y_,
					proto->width, proto->height
				))
			);
			reverseGlitz.push_back(
				Glitz(mt::std::make_unique<ImageGlitz>(
					430,
					pickupGlitzMapIt->second,
					x_, y_,
					proto->width, proto->height
				))
			);
		}
		if (justTaken) {
			persistentGlitz.push_back(
				GlitzPersister(mt::std::make_unique<AudioGlitzPersister>(
					"global.pickup_pickup",
					9,
					proto->timeDirection
				)));
		}
		#if 0
		if active then
			local forwardsImageGlitz = {
				type = "image",
				layer = 430,
				key = pickupGlitzNameMap[proto.pickupType],
				x = PnV.x,
				y = PnV.y,
				width = proto.height,
				height = proto.width
			}

			local reverseImageGlitz = {
				type = "image",
				layer = 430,
				key = pickupGlitzNameMap[proto.pickupType],
				x = PnV.x,
				y = PnV.y,
				width = proto.height,
				height = proto.width
			}
			table.insert(forwardsGlitz, forwardsImageGlitz)
			table.insert(reverseGlitz, reverseImageGlitz)
		end
		if justTaken then
			table.insert(
				persistentGlitz,
				{type = "audio",
					key = "global.pickup_pickup",
					duration = 9,
					timeDirection = proto.timeDirection})
		end
		#endif

	}
	void PickupFrameStateImpl::fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const {
		assert(proto);
		outputTriggers[proto->triggerID] = mt::std::vector<int>{active};
	}

	boost::optional<Guy> PickupFrameStateImpl::effect(Guy const &guy) {
		//assert(false);
		assert(proto);
		#if 0
		if not active then 
			return dynamicObject 
		end
		if dynamicObject.type ~= 'guy' then
			return dynamicObject 
		end
		dynamicObject.pickups[proto.pickupType] =
			dynamicObject.pickups[proto.pickupType] + (proto.pickupNumber or 1)
		active = false
		justTaken = true
		return dynamicObject
		#endif
		if (!active) return guy;
		active = false;
		justTaken = true;
		auto pickups(guy.getPickups());
		pickups[proto->pickupType] += proto->pickupNumber;

		return Guy{
			guy.getIndex(),
			guy.getX(), guy.getY(),
			guy.getXspeed(), guy.getYspeed(),
			guy.getWalkSpeed(),
			guy.getJumpHold(),
			guy.getAction(),
			guy.getWidth(), guy.getHeight(),
			guy.getJumpSpeed(),

			guy.getIllegalPortal(),
			guy.getArrivalBasis(),
			guy.getSupported(),
			guy.getSupportedSpeed(),

			std::move(pickups),
			guy.getFacing(),

			guy.getBoxCarrying(),
			guy.getBoxCarryWidth(),
			guy.getBoxCarryHeight(),
			guy.getBoxCarryDirection(),

			guy.getTimeDirection(),
			guy.getTimePaused()};
	}
	void PickupFrameStateImpl::addMutator(
			mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
			mp::std::vector<Collision> const &collisions,
			mp::std::vector<MutatorArea> &mutators,
			mp::std::vector<MutatorFrameStateImpl *> &activeMutators
		)
	{
		#if 0
		if triggerArrivals[triggerID][1] == 1 then
			PnV = calculateButtonPositionAndVelocity(proto, collisions)
			mutators[#mutators+1] = {
				x = PnV.x, y = PnV.y,
				width = proto.width, height = proto.height,
				xspeed = PnV.xspeed, yspeed = PnV.yspeed,
				collisionOverlap = 0,
				timeDirection = proto.timeDirection
			}
			activeMutators[#activeMutators+1] = self
		else
			active = false
		end
		#endif
		assert(proto);
		assert(triggerArrivals.size() > proto->triggerID);
		assert(triggerArrivals[proto->triggerID].size() > 0);
		if (triggerArrivals[proto->triggerID][0]) {
			auto const [x, y, xspeed, yspeed] = snapAttachment(proto->timeDirection, proto->attachment, collisions);
			x_ = x;
			y_ = y;
			mutators.emplace_back(
				x, y,
				proto->width, proto->height,
				xspeed, yspeed,
				0,//collisionOverlap
				proto->timeDirection);
			activeMutators.emplace_back(this);
		}
		else {
			active = false;
		}

	}

	void SpikesFrameStateImpl::addMutator(
			mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
			mp::std::vector<Collision> const &collisions,
			mp::std::vector<MutatorArea> &mutators,
			mp::std::vector<MutatorFrameStateImpl *> &activeMutators
		)
	{
		auto [x, y, xspeed, yspeed] = snapAttachment(proto->timeDirection, proto->attachment, collisions);
		x_ = x;
		y_ = y;
		xspeed_ = xspeed;
		yspeed_ = yspeed;
		mutators.emplace_back(
			x, y,
			proto->width, proto->height,
			xspeed, yspeed,
			1,//collisionOverlap; So that spikes just below the surface are not deadly
			proto->timeDirection);
		activeMutators.emplace_back(this);
	}

	void SpikesFrameStateImpl::calculateGlitz(
			mt::std::vector<Glitz> &forwardsGlitz,
			mt::std::vector<Glitz> &reverseGlitz,
			mt::std::vector<GlitzPersister> &persistentGlitz) const
	{
		assert(proto);
		{
			auto const colour = asPackedColour(255, 0, 0);
			auto const [forGlitz, revGlitz] = calculateBidirectionalGlitz(430, x_, y_, xspeed_, yspeed_, proto->width, proto->height, proto->timeDirection, colour, colour);
			forwardsGlitz.push_back(forGlitz);
			reverseGlitz.push_back(revGlitz);
		}
		for (auto &&death : deathGlitz) {
			auto const [forGlitz, revGlitz] = calculateBidirectionalGlitz(430, death.x, death.y, death.xspeed, death.yspeed, death.width, death.height, death.timeDirection, asPackedColour(255,0,0), asPackedColour(0, 255, 255));
			forwardsGlitz.push_back(forGlitz);
			reverseGlitz.push_back(revGlitz);
		}
	}

	void SpikesFrameStateImpl::fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const {
		//Do Nothing
	}
	boost::optional<Guy> SpikesFrameStateImpl::effect(Guy const &guy) {
		deathGlitz.push_back(
			//TODO: getDynamicArea(guy)?
			{
				guy.getX(),
				guy.getY(),
				guy.getXspeed(),
				guy.getYspeed(),
				guy.getWidth(),
				guy.getHeight(),
				guy.getTimeDirection()
			}
			
		);
		#if 0
		deathGlitz[#deathGlitz+1] = {
			x = dynamicObject.x,
			y = dynamicObject.y,
			width = dynamicObject.width,
			height = dynamicObject.height,
			timeDirection = dynamicObject.timeDirection
		}
		return nil
		#endif
		return {};
	}




	void PowerupFrameStateImpl::addMutator(
		mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
		mp::std::vector<Collision> const &collisions,
		mp::std::vector<MutatorArea> &mutators,
		mp::std::vector<MutatorFrameStateImpl *> &activeMutators
	)
	{
		/*
		if triggerArrivals[triggerID][1] == 1 then
			PnV = calculateButtonPositionAndVelocity(proto, collisions)
			mutators[#mutators+1] = {
				x = PnV.x, y = PnV.y,
				width = proto.width, height = proto.height,
				xspeed = PnV.xspeed, yspeed = PnV.yspeed,
				collisionOverlap = 0,
				timeDirection = proto.timeDirection
			}
			activeMutators[#activeMutators+1] = self
		else
			active = false
		end
		*/
		assert(proto->triggerID < std::size(triggerArrivals));
		auto const &triggerArrival{triggerArrivals[proto->triggerID]};
		if (0 < std::size(triggerArrival) && triggerArrival[0] == 1) {
			auto[x, y, xspeed, yspeed] = snapAttachment(proto->timeDirection, proto->attachment, collisions);
			PnV = {x,y,xspeed,yspeed};
			mutators.emplace_back(
				x, y,
				proto->width, proto->height,
				xspeed, yspeed,
				0,
				proto->timeDirection);
			activeMutators.emplace_back(this);
		}
		else {
			active = false;
		}
	}

	void PowerupFrameStateImpl::calculateGlitz(
		mt::std::vector<Glitz> &forwardsGlitz,
		mt::std::vector<Glitz> &reverseGlitz,
		mt::std::vector<GlitzPersister> &persistentGlitz) const
	{
		/*
		if active then
			local forwardsImageGlitz = {
				type = "image",
				layer = 430,
				key = powerupGlitzNameMap[proto.powerupType],
				x = PnV.x,
				y = PnV.y,
				width = proto.width,
				height = proto.height
			}

			local reverseImageGlitz = {
				type = "image",
				layer = 430,
				key = powerupGlitzNameMap[proto.powerupType],
				x = PnV.x,
				y = PnV.y,
				width = proto.width,
				height = proto.height
			}
			table.insert(forwardsGlitz, forwardsImageGlitz)
			table.insert(reverseGlitz, reverseImageGlitz)
		end
		if justTaken then
			table.insert(
				persistentGlitz,
				{type = "audio",
					key = "global.pickup_pickup",
					duration = 9,
					timeDirection = proto.timeDirection})
		end
		*/
		static std::map<Powerup, mt::std::string> const powerupGlitzNameMap{
			{Powerup::JUMP_BOOTS, "global.powerup_jump"}
		};

		auto const powerupGlitzMapIt = powerupGlitzNameMap.find(proto->powerupType);

		if (active && powerupGlitzMapIt != powerupGlitzNameMap.end()) {
			forwardsGlitz.emplace_back(
				mt::std::make_unique<ImageGlitz>(
					430,
					powerupGlitzMapIt->second,
					PnV.x, PnV.y,
					proto->width, proto->height
					));
			reverseGlitz.emplace_back(
				mt::std::make_unique<ImageGlitz>(
					430,
					powerupGlitzMapIt->second,
					PnV.x, PnV.y,
					proto->width, proto->height
					));
		}
		if (justTaken) {
			persistentGlitz.push_back(
				GlitzPersister(mt::std::make_unique<AudioGlitzPersister>(
					"global.pickup_pickup",
					9,
					proto->timeDirection
					)));
		}
	}

	void PowerupFrameStateImpl::fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const {
		/*outputTriggers[triggerID] = {active and 1 or 0}*/
		outputTriggers[proto->triggerID] = mt::std::vector<int>{ active ? 1 : 0 };
	}
	boost::optional<Guy> PowerupFrameStateImpl::effect(Guy const &guy) {
		/*
		if not active then
			return dynamicObject
		end
		if dynamicObject.type ~= 'guy' then
			return dynamicObject
		end
		powerupEffectNameMap[proto.powerupType](dynamicObject)
		active = false
		justTaken = true
		return dynamicObject
		*/
		if (!active) {
			return {guy};
		}

		active = false;
		justTaken = true;
		switch (proto->powerupType) {
		case Powerup::JUMP_BOOTS:{
			return {Guy(
				guy.getIndex(),
				guy.getX(), guy.getY(),
				guy.getXspeed(), guy.getYspeed(),
				guy.getWalkSpeed(),
				guy.getJumpHold(),
				guy.getAction(),
				guy.getWidth(), guy.getHeight(),
				-500,

				guy.getIllegalPortal(),
				guy.getArrivalBasis(),
				guy.getSupported(),
				guy.getSupportedSpeed(),

				guy.getPickups(),
				guy.getFacing(),

				guy.getBoxCarrying(),
				guy.getBoxCarryWidth(),
				guy.getBoxCarryHeight(),
				guy.getBoxCarryDirection(),

				guy.getTimeDirection(),
				guy.getTimePaused()
			)};
		}break;
		}
		assert(false);
		return {guy};
	}



	//TODO: Does this logic already exist somewhere in C++??
	bool temporalIntersectingExclusive(
		int const xa_raw, int const ya_raw, int const xspeeda, int const yspeeda, int const wa, int const ha, TimeDirection const timeDirectiona,
		int const xb_raw, int const yb_raw, int const xspeedb, int const yspeedb, int const wb, int const hb, TimeDirection const timeDirectionb 
	)
	{
		int const xa = timeDirectiona == TimeDirection::FORWARDS ? xa_raw : xa_raw - xspeeda;
		int const ya = timeDirectiona == TimeDirection::FORWARDS ? ya_raw : ya_raw - yspeeda;

		int const xb = timeDirectiona == TimeDirection::FORWARDS ? xb_raw : xb_raw - xspeedb;
		int const yb = timeDirectiona == TimeDirection::FORWARDS ? yb_raw : yb_raw - yspeedb;

		return
			(
				(xa < xb && xa + wa > xb)
					||
				(xb < xa && xb + wb > xa)
					||
				(xa == xb)
			)
				&&
			(
				(ya < yb && ya + ha > yb)
					||
				(yb < ya && yb + hb > ya)
					||
				(ya == yb)
			);
	}
	bool checkPressed(
		int const x, int const y, int const xspeed, int const yspeed, int const width, int const height, TimeDirection const timeDirection,
		mt::std::map<Frame*, ObjectList<Normal>> const &departures)
	{

		#if 0
		local function checkPressed(buttonArea, departures)
			for frame, objectList in pairs(departures) do
				for box in list_iter(objectList.boxes) do
					if temporalIntersectingExclusive(buttonArea, box) then
						return true
					end
				end
				for guy in list_iter(objectList.guys) do
					if temporalIntersectingExclusive(buttonArea, guy) then
						return true
					end
				end
			end
			return false
		end
		#endif

		for(auto &&[_, objectList]:departures) {
			(void)_;
			for (auto &&box : objectList.getList<Box>()) {
				if (temporalIntersectingExclusive(
						x, y, xspeed, yspeed, width, height, timeDirection,
						box.getX(), box.getY(), box.getXspeed(), box.getYspeed(), box.getWidth(), box.getHeight(), box.getTimeDirection()
					))
				{
					return true;
				}
			}
			for (auto &&guy : objectList.getList<Guy>()) {
				if (temporalIntersectingExclusive(
						x, y, xspeed, yspeed, width, height, timeDirection,
						guy.getX(), guy.getY(), guy.getXspeed(), guy.getYspeed(), guy.getWidth(), guy.getHeight(), guy.getTimeDirection()
					))
				{
					return true;
				}
			}
		}
		return false;
	}


	void MomentarySwitchFrameStateImpl::calcPnV(mp::std::vector<Collision> const &collisions)
	{
		std::tie(x, y, xspeed, yspeed) = snapAttachment(proto->timeDirection, proto->attachment, collisions);
	}
	void MomentarySwitchFrameStateImpl::updateState(
		mt::std::map<Frame*, ObjectList<Normal>> const &departures,
		mp::std::vector<mp::std::vector<int>> const &triggerArrivals)
	{
		assert(proto->stateTriggerID < triggerArrivals.size());
		assert(0 < triggerArrivals[proto->stateTriggerID].size());
		int const oldState = triggerArrivals[proto->stateTriggerID][0];
		state = std::min(
			checkPressed(x, y, xspeed, yspeed, proto->width, proto->height, proto->timeDirection, departures) ? oldState+1 : 0,
			2);
		justPressed = oldState == 0 && state != 0;
		justReleased = oldState != 0 && state == 0;
	}
	void MomentarySwitchFrameStateImpl::calculateGlitz(
			mt::std::vector<Glitz> &forwardsGlitz,
			mt::std::vector<Glitz> &reverseGlitz,
			mt::std::vector<GlitzPersister> &persistentGlitz) const
	{

		#if 0
		local forGlitz, revGlitz = calculateButtonGlitz(proto, PnV, state > 0)
		table.insert(forwardsGlitz, forGlitz)
		table.insert(reverseGlitz, revGlitz)
		if justPressed then
			table.insert(
				persistentGlitz,
				{type = "audio", key = "global.switch_push_down", duration = 10, timeDirection = proto.timeDirection})
		end
		if justReleased then
			table.insert(
				persistentGlitz,
				{type = "audio", key = "global.switch_push_up", duration = 10, timeDirection = proto.timeDirection})
		end
		#endif

		auto [forGlitz, revGlitz] = calculateButtonGlitz(
			DynamicArea{
			   x,
			   y,
			   xspeed,
			   yspeed,
			   proto->width,
			   proto->height,
			   proto->timeDirection},
			state != 0);
		forwardsGlitz.push_back(std::move(forGlitz));
		reverseGlitz.push_back(std::move(revGlitz));

		if (justPressed) {
			persistentGlitz.push_back(
				GlitzPersister(mt::std::make_unique<AudioGlitzPersister>("global.switch_push_down", 10, proto->timeDirection))
			);
		}
		if (justReleased) {
			persistentGlitz.push_back(
				GlitzPersister(mt::std::make_unique<AudioGlitzPersister>("global.switch_push_up", 10, proto->timeDirection))
			);
		}
	}
	void MomentarySwitchFrameStateImpl::fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const {
		outputTriggers[proto->triggerID] = mt::std::vector<int>{state};
		outputTriggers[proto->stateTriggerID] = mt::std::vector<int>{state};
		for (auto extraTriggerID : proto->extraTriggerIDs) {
			outputTriggers[extraTriggerID] = mt::std::vector<int>{state};
		}
	}


	void StickySwitchFrameStateImpl::calcPnV(mp::std::vector<Collision> const &collisions)
	{
		std::tie(x, y, xspeed, yspeed) = snapAttachment(proto->timeDirection, proto->attachment, collisions);
	}
	void StickySwitchFrameStateImpl::updateState(
		mt::std::map<Frame*, ObjectList<Normal>> const &departures,
		mp::std::vector<mp::std::vector<int>> const &triggerArrivals)
	{
		assert(proto->stateTriggerID < triggerArrivals.size());
		assert(0 < triggerArrivals[proto->stateTriggerID].size());
		int const oldState = triggerArrivals[proto->stateTriggerID][0];
		state = std::min(
			oldState != 0 ?
			  oldState + 1
			: (checkPressed(x, y, xspeed, yspeed, proto->width, proto->height, proto->timeDirection, departures) ? 1 : 0),
			2);
		justPressed = oldState == 0 && state != 0;
	}
	void StickySwitchFrameStateImpl::calculateGlitz(
			mt::std::vector<Glitz> &forwardsGlitz,
			mt::std::vector<Glitz> &reverseGlitz,
			mt::std::vector<GlitzPersister> &persistentGlitz) const
	{
		auto [forGlitz, revGlitz] = calculateButtonGlitz(
			DynamicArea{
			   x,
			   y,
			   xspeed,
			   yspeed,
			   proto->width,
			   proto->height,
			   proto->timeDirection},
			state != 0);
		forwardsGlitz.push_back(std::move(forGlitz));
		reverseGlitz.push_back(std::move(revGlitz));

		if (justPressed) {
			persistentGlitz.push_back(
				GlitzPersister(mt::std::make_unique<AudioGlitzPersister>("global.switch_push_down", 10, proto->timeDirection))
			);
		}
	}
	void StickySwitchFrameStateImpl::fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const {
		outputTriggers[proto->triggerID] = mt::std::vector<int>{state};
		outputTriggers[proto->stateTriggerID] = mt::std::vector<int>{state};
		for (auto extraTriggerID : proto->extraTriggerIDs) {
			outputTriggers[extraTriggerID] = mt::std::vector<int>{state};
		}
	}


	void ToggleSwitchFrameStateImpl::calcPnV(mp::std::vector<Collision> const &collisions)
	{
		/*
		local function constructCompleteProto(timeDirection, segment)
		return {
			timeDirection = timeDirection,
			attachment = segment.attachment,
			width = segment.width, height = segment.height,
			x = segment.x, y = segment.y}
		end
		firstPnV = calculateButtonPositionAndVelocity(constructCompleteProto(timeDirection, self.first), collisions)
		secondPnV = calculateButtonPositionAndVelocity(constructCompleteProto(timeDirection, self.second), collisions)
		*/
		auto [firstx, firsty, firstxspeed, firstyspeed] = snapAttachment(proto->timeDirection, proto->first.attachment, collisions);
		firstPnV = {firstx, firsty, firstxspeed, firstyspeed};
		auto [secondx, secondy, secondxspeed, secondyspeed] = snapAttachment(proto->timeDirection, proto->second.attachment, collisions);
		secondPnV = {secondx, secondy, secondxspeed, secondyspeed};
	}
	void ToggleSwitchFrameStateImpl::updateState(
		mt::std::map<Frame*, ObjectList<Normal>> const &departures,
		mp::std::vector<mp::std::vector<int>> const &triggerArrivals)
	{
		/*
		local firstPressed = checkPressed(constructDynamicArea(constructCompleteProto(timeDirection, self.first), firstPnV), departures)
		local secondPressed = checkPressed(constructDynamicArea(constructCompleteProto(timeDirection, self.second), secondPnV), departures)
			
		if firstPressed and secondPressed then
			switchState = triggerArrivals[stateTriggerID][1]
		elseif triggerArrivals[stateTriggerID][1] == 0 then
			if firstPressed then switchState = 1 else switchState = 0 end
		else
			if secondPressed then switchState = 0 else switchState = 1 end
		end
		justPressed = switchState ~= triggerArrivals[stateTriggerID][1]
		*/
		assert(0 <= proto->stateTriggerID);
		assert(proto->stateTriggerID < triggerArrivals.size());
		assert(0 < triggerArrivals[proto->stateTriggerID].size());
		auto const stateTriggerValue{triggerArrivals[proto->stateTriggerID][0]};
		auto const firstPressed{checkPressed(
			firstPnV.x, firstPnV.y, firstPnV.xspeed, firstPnV.yspeed, proto->first.width, proto->first.height, proto->timeDirection,
			departures)};
		auto const secondPressed{checkPressed(
			secondPnV.x, secondPnV.y, secondPnV.xspeed, secondPnV.yspeed, proto->second.width, proto->second.height, proto->timeDirection,
			departures)};
		switchState = 
			(firstPressed && secondPressed) ?
				stateTriggerValue == 1
			: (stateTriggerValue == 0) ?
				firstPressed
			:   !secondPressed;

		justPressed = (switchState ? 1 : 0) != stateTriggerValue;
	}
	void ToggleSwitchFrameStateImpl::fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const
	{
		/*
		outputTriggers[triggerID] = {switchState}
		outputTriggers[stateTriggerID] = {switchState}
		if p.extraTriggerIDs then
			local extraTriggerIDs = p.extraTriggerIDs
			for i = 1, #extraTriggerIDs do
				outputTriggers[extraTriggerIDs[i]] = {switchState}
			end
		end
		*/
		outputTriggers[proto->triggerID] = mt::std::vector<int>{switchState ? 1 : 0};
		outputTriggers[proto->stateTriggerID] = mt::std::vector<int>{switchState ? 1 : 0 };
		for (auto const extraTriggerID : proto->extraTriggerIDs) {
			outputTriggers[extraTriggerID] = mt::std::vector<int>{switchState ? 1 : 0 };
		}
	}
	void ToggleSwitchFrameStateImpl::calculateGlitz(
		mt::std::vector<Glitz> &forwardsGlitz,
		mt::std::vector<Glitz> &reverseGlitz,
		mt::std::vector<GlitzPersister> &persistentGlitz) const
	{
		/*
		do
			local forGlitz, revGlitz =
				calculateButtonGlitz(constructCompleteProto(timeDirection, self.first), firstPnV, switchState > 0)
			table.insert(forwardsGlitz, forGlitz)
			table.insert(reverseGlitz, revGlitz)
		end
		do
			local forGlitz, revGlitz =
				calculateButtonGlitz(constructCompleteProto(timeDirection, self.second), secondPnV, switchState == 0)
			table.insert(forwardsGlitz, forGlitz)
			table.insert(reverseGlitz, revGlitz)
		end
		if justPressed then
			table.insert(
				persistentGlitz,
				{type = "audio", key = "global.switch_toggle", duration = 6, timeDirection = timeDirection})
		end
		*/
		{
			auto [forGlitz, revGlitz] = calculateButtonGlitz(
				DynamicArea{
				   firstPnV.x,
				   firstPnV.y,
				   firstPnV.xspeed,
				   firstPnV.yspeed,
				   proto->first.width,
				   proto->first.height,
				   proto->timeDirection},
				switchState/*switchState != 0*/);
			forwardsGlitz.push_back(std::move(forGlitz));
			reverseGlitz.push_back(std::move(revGlitz));
		}
		{
			auto [forGlitz, revGlitz] = calculateButtonGlitz(
				DynamicArea{
				   secondPnV.x,
				   secondPnV.y,
				   secondPnV.xspeed,
				   secondPnV.yspeed,
				   proto->second.width,
				   proto->second.height,
				   proto->timeDirection},
				!switchState/*switchState == 0*/);
			forwardsGlitz.push_back(std::move(forGlitz));
			reverseGlitz.push_back(std::move(revGlitz));
		}
		if (justPressed) {
			persistentGlitz.push_back(
				GlitzPersister(mt::std::make_unique<AudioGlitzPersister>("global.switch_toggle", 6, proto->timeDirection))
			);
		}
	}

	MultiStickySwitchFrameStateImpl::MultiStickySwitchFrameStateImpl(
			ProtoMultiStickySwitchImpl const &proto,
			hg::memory_pool<hg::user_allocator_tbb_alloc> &pool)
		: proto(&proto)
		, PnVs(pool)
		, individualState(pool)
		, justPressed(pool)
		, justReleased(pool)
	{
		PnVs.reserve(this->proto->buttons.size());
		individualState.reserve(this->proto->buttons.size());
		justPressed.reserve(this->proto->buttons.size());
		justReleased.reserve(this->proto->buttons.size());
	}

	void MultiStickySwitchFrameStateImpl::calcPnV(mp::std::vector<Collision> const &collisions)
	{
		/*
		for i = 1, bCount do
			PnV[i] = calculateButtonPositionAndVelocity(proto[i], collisions)
		end
		*/
		for (auto const &button : proto->buttons) {
			auto [x, y, xspeed, yspeed] = snapAttachment(proto->timeDirection, button.attachment, collisions);
			PnVs.push_back(PositionAndVelocity2D{x, y, xspeed, yspeed});
		}
		assert(PnVs.size() == proto->buttons.size());
	}
	void MultiStickySwitchFrameStateImpl::updateState(
		mt::std::map<Frame*, ObjectList<Normal>> const &departures,
		mp::std::vector<mp::std::vector<int>> const &triggerArrivals)
	{
		/*
		state = true
		for i = 1, bCount do
			if triggerArrivals[stateTriggerID][i] == 0 then
				state = false
				break
			end
		end

		if not state then
			state = true
			for i = 1, bCount do
				individualState[i] = checkPressed(constructDynamicArea(proto[i], PnV[i]), departures)
				if not individualState[i] then 
					state = false
				end
				justPressed[i] = triggerArrivals[stateTriggerID][i] == 0 and individualState[i]
				justReleased[i] = triggerArrivals[stateTriggerID][i] > 0 and not individualState[i]
			end
		else
			for i = 1, bCount do
				justPressed[i] = false
				justReleased[i] = false
			end
		end

		state = (state and triggerArrivals[triggerID][1] + 1) or 0
		if state > 2 then
			state = 2
		end
		*/
		assert(0 <= proto->stateTriggerID);
		assert(proto->stateTriggerID < triggerArrivals.size());
		auto const &stateTriggerValues = triggerArrivals[proto->stateTriggerID];

		bool state{true};
		for (std::size_t i{0}, end{proto->buttons.size()}, stateTriggerEnd{stateTriggerValues.size()}; i != end && i < stateTriggerEnd; ++i) {
			if (stateTriggerValues[i] == 0) {
				state = false;
				break;
			}
		}

		assert(PnVs.size() == proto->buttons.size());
		if (!state) {
			state = true;
			for (std::size_t i{0}, end{proto->buttons.size()}; i != end; ++i) {
				auto const &button{proto->buttons[i]};
				auto const &PnV{PnVs[i]};
				individualState.push_back(
					checkPressed(
						PnV.x, PnV.y, PnV.xspeed, PnV.yspeed, button.width, button.height, proto->timeDirection,
						departures));
				if (!individualState[i]) {
					state = false;
				}
				justPressed.push_back(stateTriggerValues[i] == 0 && individualState[i]);
				justReleased.push_back(stateTriggerValues[i] != 0 && !individualState[i]);
			}
		}
		else {
			for (std::size_t i{0}, end{proto->buttons.size()}; i != end; ++i) {
				justPressed.push_back(false);
				justReleased.push_back(false);
			}
		}

		assert(0 <= proto->triggerID);
		assert(proto->triggerID < triggerArrivals.size());
		assert(0 < triggerArrivals[proto->triggerID].size());
		switchState = std::min(2, state ? triggerArrivals[proto->triggerID][0]+1 : 0);
		assert(justPressed.size() == proto->buttons.size());
		assert(justReleased.size() == proto->buttons.size());

	}
	void MultiStickySwitchFrameStateImpl::fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const
	{
		/*
		outputTriggers[triggerID] = {state}
		outputTriggers[stateTriggerID] = map(function(val) return val and 1 or 0 end, individualState)
		if p.extraTriggerIDs then
			local extraTriggerIDs = p.extraTriggerIDs
			for i = 1, #extraTriggerIDs do
				outputTriggers[extraTriggerIDs[i]] = {state}
			end
		end
		*/
		outputTriggers[proto->triggerID] = mt::std::vector<int>{switchState};
		mt::std::vector<int> stateTriggerData;
		stateTriggerData.reserve(individualState.size());
		boost::push_back(stateTriggerData, individualState |  boost::adaptors::transformed([](char val) {return val ? 1 : 0;}));
		outputTriggers[proto->stateTriggerID] = std::move(stateTriggerData);
		for (auto const extraTriggerID : proto->extraTriggerIDs) {
			outputTriggers[extraTriggerID] = mt::std::vector<int>{switchState};
		}
	}
	void MultiStickySwitchFrameStateImpl::calculateGlitz(
		mt::std::vector<Glitz> &forwardsGlitz,
		mt::std::vector<Glitz> &reverseGlitz,
		mt::std::vector<GlitzPersister> &persistentGlitz) const
	{
		/*
		for i = 1, bCount do
			local forGlitz, revGlitz = calculateButtonGlitz(proto[i], PnV[i], (state > 0) or individualState[i])
			table.insert(forwardsGlitz, forGlitz)
			table.insert(reverseGlitz, revGlitz)
			if justPressed[i] then
				table.insert(
					persistentGlitz,
					{type = "audio", key = "global.switch_push_down", duration = 10, timeDirection = proto[i].timeDirection})
			end
			if justReleased[i] then
				table.insert(
					persistentGlitz,
					{type = "audio", key = "global.switch_push_up", duration = 10, timeDirection = proto[i].timeDirection})
			end
		end
		*/
		assert(switchState != 0 || individualState.size() == proto->buttons.size());
		assert(PnVs.size() == proto->buttons.size());
		for (std::size_t i{0}, end{proto->buttons.size()}; i != end; ++i) {
			auto [forGlitz, revGlitz] = calculateButtonGlitz(
				DynamicArea{
				   PnVs[i].x,
				   PnVs[i].y,
				   PnVs[i].xspeed,
				   PnVs[i].yspeed,
				   proto->buttons[i].width,
				   proto->buttons[i].height,
				   proto->timeDirection},
				(switchState != 0) || individualState[i]);
			forwardsGlitz.push_back(std::move(forGlitz));
			reverseGlitz.push_back(std::move(revGlitz));
			
			if (justPressed[i]) {
				persistentGlitz.push_back(
					GlitzPersister(mt::std::make_unique<AudioGlitzPersister>("global.switch_push_down", 10, proto->timeDirection))
				);
			}
			if (justReleased[i]) {
				persistentGlitz.push_back(
					GlitzPersister(mt::std::make_unique<AudioGlitzPersister>("global.switch_push_up", 10, proto->timeDirection))
				);
			}
		}
	}

	void StickyLaserSwitchFrameStateImpl::calcPnV(mp::std::vector<Collision> const &collisions)
	{
		/*
		beamPnV = calculateButtonPositionAndVelocity(protoBeam, collisions)
		emitterPnV = calculateButtonPositionAndVelocity(protoEmitter, collisions)
		*/
		{
			auto const [x, y, xspeed, yspeed] = snapAttachment(proto->timeDirection, proto->beam.attachment, collisions);
			beamPnV = PositionAndVelocity2D{x, y, xspeed, yspeed};
		}
		{
			auto const [x, y, xspeed, yspeed] = snapAttachment(proto->timeDirection, proto->emitter.attachment, collisions);
			emitterPnV = PositionAndVelocity2D{x, y, xspeed, yspeed};
		}
	}
	void StickyLaserSwitchFrameStateImpl::updateState(
		mt::std::map<Frame*, ObjectList<Normal>> const &departures,
		mp::std::vector<mp::std::vector<int>> const &triggerArrivals)
	{
		/*
		local oldState = triggerArrivals[stateTriggerID][1]
		state = (oldState > 0 and oldState + 1) or (checkPressed(constructDynamicArea(protoBeam, beamPnV), departures) and 1) or 0
		if state > 2 then
			state = 2
		end
		justPressed = oldState == 0 and state > 0
		*/

		assert(proto->stateTriggerID < triggerArrivals.size());
		auto const &stateTriggerValues = triggerArrivals[proto->stateTriggerID];
		int const oldState = stateTriggerValues.size() != 0 ? triggerArrivals[proto->stateTriggerID][0] : 0;

		switchState = std::min(
			oldState != 0 ?
			   oldState + 1
			 : (checkPressed(beamPnV.x, beamPnV.y, beamPnV.xspeed, beamPnV.yspeed, proto->beam.width, proto->beam.height, proto->timeDirection, departures) ? 1 : 0),
			2);

		justPressed = oldState == 0 && switchState != 0;
	}
	void StickyLaserSwitchFrameStateImpl::fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const
	{
		/*
		outputTriggers[triggerID] = {state}
		outputTriggers[stateTriggerID] = {state}
		if p.extraTriggerIDs then
			local extraTriggerIDs = p.extraTriggerIDs
			for i = 1, #extraTriggerIDs do
				outputTriggers[extraTriggerIDs[i]] = {state}
			end
		end
		*/
		outputTriggers[proto->triggerID] = mt::std::vector<int>{switchState};
		outputTriggers[proto->stateTriggerID] = mt::std::vector<int>{switchState};
		for (auto const extraTriggerID : proto->extraTriggerIDs) {
			outputTriggers[extraTriggerID] = mt::std::vector<int>{switchState};
		}
	}

	void StickyLaserSwitchFrameStateImpl::calculateGlitz(
		mt::std::vector<Glitz> &forwardsGlitz,
		mt::std::vector<Glitz> &reverseGlitz,
		mt::std::vector<GlitzPersister> &persistentGlitz) const
	{
		/*
		local forGlitzBeam, revGlitzBeam = calculateBeamGlitz(protoBeam, beamPnV, state > 0)
		if forGlitzBeam then
			table.insert(forwardsGlitz, forGlitzBeam)
			table.insert(reverseGlitz, revGlitzBeam)
		end

		local forGlitz, revGlitz = calculateButtonGlitz(protoEmitter, emitterPnV, state > 0)
		table.insert(forwardsGlitz, forGlitz)
		table.insert(reverseGlitz, revGlitz)
		if justPressed then
			table.insert(
				persistentGlitz,
				{type = "audio", key = "global.switch_push_down", duration = 10, timeDirection = timeDirection})
		end
		*/
		{
			auto maybeBeamGlitz = calculateBeamGlitz(
				DynamicArea{
					beamPnV.x,
					beamPnV.y,
					beamPnV.xspeed,
					beamPnV.yspeed,
					proto->beam.width,
					proto->beam.height,
					proto->timeDirection},
				switchState != 0);
			if (maybeBeamGlitz) {
				auto [forGlitz, revGlitz] = *maybeBeamGlitz;
				forwardsGlitz.push_back(std::move(forGlitz));
				reverseGlitz.push_back(std::move(revGlitz));
			}
		}
		{
			auto [forGlitz, revGlitz] = calculateButtonGlitz(
				DynamicArea{
					emitterPnV.x,
					emitterPnV.y,
					emitterPnV.xspeed,
					emitterPnV.yspeed,
					proto->emitter.width,
					proto->emitter.height,
					proto->timeDirection},
				switchState != 0);
			forwardsGlitz.push_back(std::move(forGlitz));
			reverseGlitz.push_back(std::move(revGlitz));
		}
		if (justPressed) {
			persistentGlitz.push_back(
				GlitzPersister(mt::std::make_unique<AudioGlitzPersister>("global.switch_push_down", 10, proto->timeDirection))
			);
		}
	}
}
