#ifndef HG_LAYERED_CANVAS_H
#define HG_LAYERED_CANVAS_H
#include <boost/container/vector.hpp>
namespace hg {
    class Canvas;
    namespace lc_internal {
        class DrawCall;
    }
    /*
        Layers:
        
        500 -- Boxes
        600 -- Guys
        
    */
    
    class LayeredCanvas {
    public:
        LayeredCanvas(Canvas& canvas);
        void drawRect(int layer, int x, int y, int width, int height, unsigned colour);
        void drawLine(int layer, int xa, int ya, int xb, int yb, int width, unsigned colour);
        void flush();
        ~LayeredCanvas();
    private:
        Canvas* canvas;
        boost::container::vector<lc_internal::DrawCall> drawCalls;
    };
}//namespace hg
#endif //HG_LAYERED_CANVAS_H
