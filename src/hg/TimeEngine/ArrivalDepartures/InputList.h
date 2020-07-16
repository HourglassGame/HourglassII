#ifndef HG_INPUT_LIST_H
#define HG_INPUT_LIST_H
#include "hg/TimeEngine/FrameID.h"
#include "Ability.h"

#include <boost/operators.hpp>
#include <istream>
#include <ostream>
#include <boost/serialization/version.hpp>

#include <cassert>

namespace boost {
	namespace serialization {
		class access;
	}
}

namespace hg {
class InputList;
std::ostream &operator<<(std::ostream &os, InputList const &toPrint);
std::istream &operator>>(std::istream &is, InputList &toRead);
class GuyInput final : boost::equality_comparable<GuyInput> {
public:
	GuyInput() noexcept;
	GuyInput(
		bool moveLeft,
		bool moveRight,
		bool jump,
		bool boxAction,
		bool boxLeft,
		bool boxRight,
		bool portalUsed,
		bool abilityUsed,
		Ability abilityCursor,
		FrameID timeCursor,
		int xCursor,
		int yCursor) noexcept;

	bool operator==(GuyInput const &o) const noexcept;

	bool getMoveLeft()   const noexcept { return moveLeft; }
	bool getMoveRight()  const noexcept { return moveRight; }
	bool getjump()       const noexcept { return jump; }
	bool getBoxAction()  const noexcept { return boxAction; }
	bool getBoxLeft()    const noexcept { return boxLeft; }
	bool getBoxRight()   const noexcept { return boxRight; }
	bool getPortalUsed() const noexcept { return portalUsed; }

	bool getAbilityUsed() const noexcept { return abilityUsed; }
	Ability getAbilityCursor() const noexcept { return abilityCursor; }
	FrameID getTimeCursor() const noexcept { return timeCursor; }
	int getXCursor() const noexcept { return xCursor; }
	int getYCursor() const noexcept { return yCursor; }

	bool getPauseActionTaken() const noexcept { return portalUsed || abilityUsed || boxAction; }
	bool getActionPause() const noexcept { return abilityCursor != Ability::NO_ABILITY && !abilityUsed; }
	bool getActionTaken() const noexcept { return moveLeft || moveRight || jump || boxAction || boxLeft || boxRight || portalUsed || abilityUsed; }
private:
	friend class InputList;
	friend std::ostream &operator<<(std::ostream &os, InputList const &toPrint);
	friend std::istream &operator>>(std::istream &is, InputList &toRead);

	bool moveLeft;
	bool moveRight;
	bool jump;
	bool boxAction;
	bool boxLeft;
	bool boxRight;
	bool portalUsed;
	bool abilityUsed;
	Ability abilityCursor;
	FrameID timeCursor;
	int xCursor;
	int yCursor;

};
class InputList final : boost::equality_comparable<InputList>
{
public:
	InputList()  noexcept :
		guyInput(), relativeGuyIndex(0)
	{}

	InputList(GuyInput const& guyInput, std::ptrdiff_t relativeGuyIndex) noexcept :
		guyInput(guyInput), relativeGuyIndex(relativeGuyIndex)
	{}

	bool operator==(InputList const &o) const noexcept {
		return guyInput == o.guyInput
			&& relativeGuyIndex == o.relativeGuyIndex;
	}
	GuyInput const &getGuyInput() const noexcept {
		return guyInput;
	}
	std::size_t getRelativeGuyIndex() const noexcept {
		return relativeGuyIndex;
	}

private:
	//Crappy serialization
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, unsigned int const version)
	{
		ar & guyInput.moveLeft;
		ar & guyInput.moveRight;
		ar & guyInput.jump;
		ar & guyInput.boxAction;
		ar & guyInput.portalUsed;
		if (version < 2) {
			//Loading only, as version < current version.
			ar & guyInput.abilityCursor;
			guyInput.abilityUsed = guyInput.abilityCursor != Ability::NO_ABILITY;
			ar & guyInput.timeCursor;
			if (version == 0) {
				int frameIdParamCount;
				ar & frameIdParamCount;
			}
			if (version >= 1) {
				ar & guyInput.xCursor;
				ar & guyInput.yCursor;
			}
		}
		else {
			ar & guyInput.abilityUsed;
			ar & guyInput.abilityCursor;
			ar & guyInput.timeCursor;
			ar & guyInput.xCursor;
			ar & guyInput.yCursor;
		}
		if (version < 3) {
			//Before it was possible to edit old GuyInputs, the InputList would
			//always alter the latest GuyInput.
			relativeGuyIndex = 0;
		}
		else {
			ar & relativeGuyIndex;
		}
		if (version < 4) {
			guyInput.boxLeft = false;
			guyInput.boxRight = false;
		}
		else {
			ar & guyInput.boxLeft;
			ar & guyInput.boxRight;
		}
	}

	GuyInput guyInput;
	std::size_t relativeGuyIndex; //Relative index of guy to override input of. (counts backwards from max guy index at time of input)

	//more crappy serialization
	inline friend std::ostream &operator<<(std::ostream &os, InputList const &toPrint)
	{
		os << toPrint.guyInput.moveLeft << " ";
		os << toPrint.guyInput.moveRight << " ";
		os << toPrint.guyInput.jump << " ";
		os << toPrint.guyInput.boxAction << " ";
		os << toPrint.guyInput.boxLeft << " ";
		os << toPrint.guyInput.boxRight << " ";
		os << toPrint.guyInput.portalUsed << " ";
		os << toPrint.guyInput.abilityUsed << " ";
		os << static_cast<int>(toPrint.guyInput.abilityCursor) << " ";
		os << toPrint.guyInput.timeCursor << " ";
		os << toPrint.guyInput.xCursor << " ";
		os << toPrint.guyInput.yCursor << " ";
		os << toPrint.relativeGuyIndex;
		return os;
	}
	inline friend std::istream &operator>>(std::istream &is, InputList &toRead)
	{
		is >> toRead.guyInput.moveLeft;
		is >> toRead.guyInput.moveRight;
		is >> toRead.guyInput.jump;
		is >> toRead.guyInput.boxAction;
		is >> toRead.guyInput.boxLeft;
		is >> toRead.guyInput.boxRight;
		is >> toRead.guyInput.portalUsed;
		is >> toRead.guyInput.abilityUsed;
		int abilityCursor;
		is >> abilityCursor;
		toRead.guyInput.abilityCursor = static_cast<Ability>(abilityCursor);
		is >> toRead.guyInput.timeCursor;
		is >> toRead.guyInput.xCursor;
		is >> toRead.guyInput.yCursor;
		is >> toRead.relativeGuyIndex;
		return is;
	}
};
}
BOOST_CLASS_VERSION(hg::InputList, 4)
#endif //HG_INPUT_LIST_H
