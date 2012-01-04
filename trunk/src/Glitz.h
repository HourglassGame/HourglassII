#ifndef HG_GLITZ_H
#define HG_GLITZ_H
namespace hg {
class Glitz {
public:
    Glitz(
        int x, int y,
        int width, int height,
        unsigned colour) :
            x_(x), y_(y),
            width_(width), height_(height),
            colour_(colour) {}

    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getWidth()  const { return width_; }
    int getHeight() const { return height_; }

    unsigned getColour() const { return colour_; }
        
private:
    int x_;
    int y_;
    int width_;
    int height_;
    
    //Colour packed as |RRRRRRRR|GGGGGGGG|BBBBBBBB|*unused*|
    //Why? -- because lua is all ints, and I can't be bothered with a better interface for such a temporary thing.
    unsigned colour_;

};
}//namespace hg
#endif //HG_GLITZ_H
