#ifndef HG_AUDIO_GLITZ_MANAGER_H
#define HG_AUDIO_GLITZ_MANAGER_H
#include "AudioGlitz.h"
#include "GlobalConst.h"
#include <vector>
#include <string>
#include <map>
#include <boost/container/map.hpp>
#include <tuple>
#include <boost/operators.hpp>
#include <SFML/Audio/Sound.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/fusion/sequence/comparison.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <gsl/gsl>
namespace hg {
enum class AudioAction {
    Start,
    Stop
};

struct AudioGlitzObject final : boost::totally_ordered<AudioGlitzObject> {
private:
    auto comparison_tuple() const -> decltype(auto)
    {
        return std::tie(key, index);
    }
public:
    AudioGlitzObject(std::string key, int index)
    : boost::totally_ordered<AudioGlitzObject>(),
      key(std::move(key)),
      index(std::move(index))
    {
    }

    std::string key;
    int index;
    bool operator==(AudioGlitzObject const &o) const {
        return comparison_tuple() == o.comparison_tuple();
    }
    bool operator<(AudioGlitzObject const &o) const {
        return comparison_tuple() < o.comparison_tuple();
    }
};

struct AudioStateChange final {
    AudioAction action;
    AudioGlitzObject glitz;
};

struct AudioPlayingState final {
public:
    void runStep(std::vector<AudioStateChange> const &diff) {
        for (auto const &change: diff) {
            switch(change.action) {
            case AudioAction::Start:{
                currentAudio.insert(
                    std::make_pair(
                        AudioGlitzObject(change.glitz.key, i-change.glitz.index),
                        makeSoundForGlitz(change.glitz)))->second.play();
            break;}
            case AudioAction::Stop: {
                auto to_delete_iterator
                    = currentAudio.find(AudioGlitzObject(change.glitz.key, i-change.glitz.index));
                to_delete_iterator->second.stop();
                currentAudio.erase(to_delete_iterator);
            break;}
            }
        }
        ++i;
    }
    
    ~AudioPlayingState() noexcept {
        for (auto &sound: currentAudio | boost::adaptors::map_values) {
            sound.stop();
        }
    }

    AudioPlayingState(boost::container::map<std::string, sf::SoundBuffer> const& soundBuffers) :
        i(0),
        soundBuffers(&soundBuffers),
        currentAudio()
    {}

    
    AudioPlayingState(AudioPlayingState &&o) noexcept :
        i(o.i),
        soundBuffers(std::move(o.soundBuffers)),
        currentAudio(std::move(o.currentAudio))
    {
        o.currentAudio.clear();
    }
    //Move implemented with `swap` rather than `move`,
    //since it is critical that `o` gets the state of `*this`
    //after the move.
    AudioPlayingState &operator=(AudioPlayingState &&o) noexcept {
        std::swap(i, o.i);
        std::swap(soundBuffers, o.soundBuffers);
        std::swap(currentAudio, o.currentAudio);
        return *this;
    }
    
    //Disallow copies, since it is unlikely that
    //you suddenly want additional sounds to be playing.
    //If you suddenly really want that, feel free to implement it.
    AudioPlayingState(AudioPlayingState const&) = delete;
    AudioPlayingState &operator=(AudioPlayingState const&) = delete;
private:
    sf::Sound makeSoundForGlitz(AudioGlitzObject const &glitz) const {
        Expects(soundBuffers->find(glitz.key) != soundBuffers->end());
        sf::Sound newSound(soundBuffers->find(glitz.key)->second);
        newSound.setPlayingOffset(sf::seconds(glitz.index * 1.f/float{hg::FRAMERATE}));
        return newSound;
    }

    int i;
    boost::container::map<std::string, sf::SoundBuffer> const *soundBuffers;
    std::multimap<AudioGlitzObject, sf::Sound> currentAudio;
};

class AudioGlitzManager final {
public:
    template<typename RandomAccessAudioGlitzObjectRange>
    std::vector<AudioStateChange> updatePlayingState(RandomAccessAudioGlitzObjectRange const &newAudios) {
        //TODO: This is very similar to frame update logic. Perhaps the two can be merged in a generic function.
        incrementCurrentAudio();
        auto oi(std::begin(currentAudio));
        auto const oend(std::end(currentAudio));
    
        auto ni(std::begin(newAudios));
        auto const nend(std::end(newAudios));
    
        std::vector<AudioStateChange> playingStateDiff;
        auto removeOldAudio = [&] {
            playingStateDiff.push_back(AudioStateChange{AudioAction::Stop, *oi});
            ++oi;
        };
        auto addNewAudio = [&] {
            playingStateDiff.push_back(AudioStateChange{AudioAction::Start, *ni});
            ++ni;
        };
        auto keepAudio = [&] {
            ++oi;
            ++ni;
        };

        auto checkOICompletion = [&] {
            if (oi == oend) {
                while (ni != nend) addNewAudio();
                return true;
            }
            return false;
        };
        auto checkNICompletion = [&] {
            if (ni == nend) {
                while (oi != oend) removeOldAudio();
                return true;
            }
            return false;
        };

        if (!(checkOICompletion() || checkNICompletion())) {
            while (true) {
                if (*oi < *ni) {
                    removeOldAudio();
                    if (checkOICompletion()) break;
                }
                else if (*oi > *ni) {
                    addNewAudio();
                    if (checkNICompletion()) break;
                }
                else {
                    Expects(*oi == *ni);
                    keepAudio();
                    if (checkOICompletion() || checkNICompletion()) break;
                }
            }
        }
        
        currentAudio = std::move(newAudios);
        return playingStateDiff;
    }
private:

    void incrementCurrentAudio() {
        for (auto & audio: currentAudio) ++audio.index;
    }
    std::vector<AudioGlitzObject> currentAudio;
};
}
#endif //HG_AUDIO_GLITZ_MANAGER_H
