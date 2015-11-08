#ifndef HG_AUDIO_GLITZ_H
#define HG_AUDIO_GLITZ_H
#include <string>
#include <tuple>
#include <boost/cast.hpp>
#include "GlitzImplementation.h"
#include "LayeredCanvas.h"
namespace hg {
//TODO - perhaps separate this into
//       something other than a
//       GlitzImplementation,
//       since GlitzImplementation is designed
//       for graphics rather than sound.
class AudioGlitz final : public GlitzImplementation {
public:
    AudioGlitz(std::string key, std::size_t n) :
        key(std::move(key)),
        n(std::move(n))
    {}

    virtual void display(LayeredCanvas &canvas) const override {
        canvas.playSound(key, static_cast<int>(n));
    }
    virtual std::size_t clone_size() const override {
        return sizeof *this;
    }
    virtual AudioGlitz *perform_clone(void *memory) const override {
        return new (memory) AudioGlitz(*this);
    }

    virtual ~AudioGlitz() noexcept override {}
    
    //Each glitz-implementation must provide a
    //unique return value for order_ranking.
    //This is used to sort glitz for arrival/departure analysis
    //(for persistent glitz).
    virtual bool operator<(GlitzImplementation const &o) const override {
        AudioGlitz const &actual_other(*boost::polymorphic_downcast<AudioGlitz const*>(&o));
        return as_tie() < actual_other.as_tie();
    }
    virtual bool operator==(GlitzImplementation const &o) const override {
        AudioGlitz const &actual_other(*boost::polymorphic_downcast<AudioGlitz const*>(&o));
        return as_tie() == actual_other.as_tie();
    }
private:
    virtual int order_ranking() const override {
        return -1;
    }
    std::string key;
    std::size_t n; //`n` gives the index into the sound;
                   //that is, the exact part of the sound
                   //that must be played in the given frame.
                   //Sounds are divided into 1/60 second chunks
    //FrameID startFrame; //maybe?

    auto as_tie() const -> decltype(std::tie(key,n)) {
        return std::tie(key,n);
    }
};
}
#endif // HG_AUDIO_GLITZ_H
