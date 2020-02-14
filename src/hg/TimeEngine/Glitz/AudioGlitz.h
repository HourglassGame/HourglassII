#ifndef HG_AUDIO_GLITZ_H
#define HG_AUDIO_GLITZ_H
#include "hg/mt/std/string"
#include <tuple>
#include <boost/polymorphic_cast.hpp>
#include "GlitzImplementation.h"
#include "hg/FrontEnd/LayeredCanvas.h"
#include <gsl/gsl>
namespace hg {
//TODO - perhaps separate this into
//       something other than a
//       GlitzImplementation,
//       since GlitzImplementation is designed
//       for graphics rather than sound.
class AudioGlitz final : public GlitzImplementation {
    auto comparison_tuple() const -> decltype(auto) {
        return std::tie(key, n);
    }
public:
    AudioGlitz(mt::std::string key, std::size_t n) :
        key(std::move(key)),
        n(std::move(n))
    {}

    virtual void display(LayeredCanvas &canvas) const override {
        std::string key_str(std::begin(key), std::end(key));
        canvas.playSound(key_str, gsl::narrow<int>(n));
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
        return comparison_tuple() < actual_other.comparison_tuple();
    }
    virtual bool operator==(GlitzImplementation const &o) const override {
        AudioGlitz const &actual_other(*boost::polymorphic_downcast<AudioGlitz const*>(&o));
        return comparison_tuple() == actual_other.comparison_tuple();
    }
private:
    virtual int order_ranking() const override {
        return -1;
    }
    mt::std::string key;
    std::size_t n; //`n` gives the index into the sound;
                   //that is, the exact part of the sound
                   //that must be played in the given frame.
                   //Sounds are divided into 1/hg::FRAMERATE second chunks
    //FrameID startFrame; //maybe?

};
}
#endif // HG_AUDIO_GLITZ_H
