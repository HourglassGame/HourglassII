#ifndef HG_CANVAS_H
#define HG_CANVAS_H
#include <string>
namespace hg {
    class Canvas {
    public:
        virtual void drawRect(int x, int y, int width, int height, unsigned colour) = 0;
        virtual void drawLine(int xa, int ya, int xb, int yb, int width, unsigned colour) = 0;
        virtual void drawText(std::string const& text, int x, int y, int size, unsigned colour) = 0;
    protected:
        ~Canvas() {}
    };
}//namespace hg
#endif //HG_CANVAS_H
