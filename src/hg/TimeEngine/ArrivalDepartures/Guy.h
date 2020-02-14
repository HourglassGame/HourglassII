#ifndef HG_GUY_H
#define HG_GUY_H
#include "TimeDirection.h"
#include "Ability.h"
#include "FacingDirection.h"
#include <boost/operators.hpp>
#include "hg/mt/std/map"
#include <cstdlib>
#include "SortWeakerThanEquality_fwd.h"
#include "ConstPtr_of_fwd.h"
#include <type_traits>
#include <gsl/gsl>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/algorithm/count_if.hpp>
namespace hg {
template<typename ToMatch, typename ToConstQualify>
using MatchConstness =
    typename std::conditional_t <std::is_const_v<ToMatch>, ToConstQualify const, ToConstQualify>;

template<typename T>
class PickupsIt : public boost::iterator_facade<
      PickupsIt<T>
    , std::pair<Ability const, MatchConstness<T, int>&>
    , boost::forward_traversal_tag
    , std::pair<Ability const, MatchConstness<T, int>&>> {
private:
    static Ability nextAbility(Ability const ab) {
        switch (ab) {
            case Ability::TIME_JUMP: return Ability::TIME_REVERSE;
            case Ability::TIME_REVERSE: return Ability::TIME_GUN;
            case Ability::TIME_GUN: return Ability::TIME_PAUSE;
            case Ability::TIME_PAUSE: return Ability::NO_ABILITY;
            case Ability::NO_ABILITY: Expects(false); break;
        }
        Expects(false);
    }
    static Ability autoIncr(T &pickups, Ability currentAbility){
        while (currentAbility != Ability::NO_ABILITY) {
            if (pickups[currentAbility] != 0) break;
            currentAbility = nextAbility(currentAbility);
        }
        return currentAbility;
    }
private:
    T *pickups;
    Ability currentAbility;
public:

    PickupsIt(T &pickups, Ability const currentAbility) :
        pickups(&pickups), currentAbility(autoIncr(pickups, currentAbility))
    {}
    std::pair<Ability const, decltype((*pickups)[currentAbility])> dereference() const {
        return {currentAbility, (*pickups)[currentAbility]};
    }
    bool equal(PickupsIt const &o) const {
        return pickups == o.pickups && currentAbility == o.currentAbility;
    }
    void increment() {
        currentAbility = autoIncr(*pickups, nextAbility(currentAbility));
    }
};

class Pickups final : boost::totally_ordered<Pickups> {
    struct NeqZero{
        bool operator()(int a) const {
            return a != 0;
        }
    };
    template<typename PickupsT>
    static MatchConstness<PickupsT, int> &operatorIdxImpl(PickupsT &p, Ability const ab) {
        Expects(ab != Ability::NO_ABILITY);
        switch (ab) {
            case Ability::TIME_JUMP: return p.pickups[0];
            case Ability::TIME_REVERSE : return p.pickups[1];
            case Ability::TIME_GUN : return p.pickups[2];
            case Ability::TIME_PAUSE : return p.pickups[3];
            case Ability::NO_ABILITY: Expects(false); break;
        }
        Expects(false);
    }
    template<typename PickupsT>
    static PickupsIt<MatchConstness<PickupsT, Pickups>> findImpl(PickupsT &p, Ability const ab) {
        auto const it{ PickupsIt<MatchConstness<PickupsT, Pickups>>(p, ab) };
        if (it != p.end() && it->first == ab) return it;
        else return p.end();
    }
    auto comparison_tuple() const noexcept {
        return std::tie(pickups);
    }
public:
    explicit Pickups() noexcept : pickups() {}
    PickupsIt<Pickups> begin() {
        return PickupsIt<Pickups>(*this, Ability::TIME_JUMP);
    }
    PickupsIt<Pickups> end() {
        return PickupsIt<Pickups>(*this, Ability::NO_ABILITY);
    }
    PickupsIt<Pickups const> begin() const {
        return PickupsIt<Pickups const>(*this, Ability::TIME_JUMP);
    }
    PickupsIt<Pickups const> end() const {
        return PickupsIt<Pickups const>(*this, Ability::NO_ABILITY);
    }

    PickupsIt<Pickups const> find(Ability const ab) const {
        return findImpl(*this, ab);
    }

    PickupsIt<Pickups> find(Ability const ab) {
        return findImpl(*this, ab);
    }

    std::size_t size() const {
        return boost::count_if(pickups, [](int const a) { return a != 0; });
    }

    int const &operator[](Ability const ab) const noexcept {
        return operatorIdxImpl(*this, ab);
    }
    int &operator[](Ability const ab) noexcept {
        return operatorIdxImpl(*this, ab);
    }

    bool operator==(Pickups const &o) const noexcept {
        return comparison_tuple() == o.comparison_tuple();
    }
    bool operator<(Pickups const &second) const noexcept {
        return comparison_tuple() < second.comparison_tuple();
    }
private:
    std::array<int, 4> pickups;
};

class Guy final : boost::totally_ordered<Guy>
{
public:
    Guy(std::size_t index,
        int x, int y,
        int xspeed, int yspeed,
        int walkSpeed,
        int jumpHold,
        int width, int height,
        int jumpSpeed,

        int illegalPortal,
        int arrivalBasis,
        int supported,
        int supportedSpeed,

        Pickups pickups,
        FacingDirection facing,

        bool boxCarrying,
        int boxCarrySize,
        TimeDirection boxCarryDirection,

        TimeDirection timeDirection,
        bool timePaused);
    
    std::size_t getIndex() const { return index; }
    int getX()         const { return x; }
    int getY()         const { return y; }
    int getXspeed()    const { return xspeed; }
    int getYspeed()    const { return yspeed; }
    int getWalkSpeed() const { return walkSpeed; }
    int getJumpHold()  const { return jumpHold; }
    int getWidth()     const { return width; }
    int getHeight()    const { return height; }
    int getJumpSpeed() const { return jumpSpeed; }
    
    int getIllegalPortal()    const { return illegalPortal; }
    int getArrivalBasis()     const { return arrivalBasis; }
    int getSupported()        const { return supported; }
    int getSupportedSpeed()   const { return supportedSpeed; }

    Pickups const &getPickups() const { return pickups; }

    FacingDirection getFacing()        const { return facing; }

    bool getBoxCarrying()  const { return boxCarrying; }
    int getBoxCarrySize()  const { return boxCarrySize; }
    TimeDirection 
        getBoxCarryDirection() const { return boxCarryDirection; }

    TimeDirection
        getTimeDirection() const { return timeDirection; }
    bool getTimePaused()    const { return timePaused; }


    bool operator==(Guy const &o) const;
    bool operator<(Guy const &second) const;
    
private:
    std::size_t index;
    int x;
    int y;
    int xspeed;
    int yspeed;
    int walkSpeed;
    int jumpHold;
    int width;
    int height;
    int jumpSpeed;

    int illegalPortal;
    int arrivalBasis;
    int supported;
    int supportedSpeed;

    Pickups pickups;
    FacingDirection facing; // <- 0, -> 1

    bool boxCarrying;
    int boxCarrySize;
    TimeDirection boxCarryDirection;

    TimeDirection timeDirection;
    bool timePaused;

    auto equality_tuple() const -> decltype(auto)
    {
        return std::tie(
            index, 
            x, y, xspeed, yspeed, walkSpeed, jumpHold, width, height, jumpSpeed,
            illegalPortal, arrivalBasis, supported, supportedSpeed,
            pickups, facing,
            boxCarrying, boxCarrySize, boxCarryDirection,
            timeDirection, timePaused);
    }
};

class GuyConstPtr final : boost::totally_ordered<GuyConstPtr>
{
public:
    GuyConstPtr(Guy const &guy) : guy_(&guy) {}
    typedef Guy base_type;
    Guy const &get() const   { return *guy_; }
    
    std::size_t getIndex() const { return guy_->getIndex(); }
    int getX()         const { return guy_->getX(); }
    int getY()         const { return guy_->getY(); }
    int getXspeed()    const { return guy_->getXspeed(); }
    int getYspeed()    const { return guy_->getYspeed(); }
    int getWalkSpeed() const { return guy_->getWalkSpeed(); }
    int getJumpHold()  const { return guy_->getJumpHold(); }
    int getWidth()     const { return guy_->getWidth(); }
    int getHeight()    const { return guy_->getHeight(); }
    int getJumpSpeed() const { return guy_->getJumpSpeed(); }

    int getIllegalPortal()    const { return guy_->getIllegalPortal(); }
    int getArrivalBasis()     const { return guy_->getArrivalBasis(); }
    int getSupported()        const { return guy_->getSupported(); }
    int getSupportedSpeed()   const { return guy_->getSupportedSpeed(); }

    Pickups const &getPickups() const { return guy_->getPickups();}

    FacingDirection getFacing()        const { return guy_->getFacing();}

    bool getBoxCarrying()  const { return guy_->getBoxCarrying(); }
    int getBoxCarrySize()  const { return guy_->getBoxCarrySize(); }
    TimeDirection 
        getBoxCarryDirection() const { return guy_->getBoxCarryDirection(); }

    TimeDirection
        getTimeDirection() const { return guy_->getTimeDirection(); }
    bool getTimePaused()    const { return guy_->getTimePaused(); }

    bool operator==(GuyConstPtr const &o) const { return *guy_ == *o.guy_; }
    bool operator<(GuyConstPtr const &o) const { return *guy_ < *o.guy_; }
private:
    Guy const *guy_;
};

template<>
struct ConstPtr_of<Guy> {
    typedef GuyConstPtr type;
};

template<>
struct sort_weaker_than_equality<Guy> final
{
    static bool const value = true;
};

template<>
struct sort_weaker_than_equality<GuyConstPtr> final
{
    static bool const value = true;
};
}
#endif //HG_GUY_H
