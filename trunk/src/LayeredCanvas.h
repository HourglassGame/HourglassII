#ifndef HG_LAYERED_CANVAS_H
#define HG_LAYERED_CANVAS_H
#include <boost/container/vector.hpp>
namespace hg {
    class Canvas;
    class LayeredCanvas;
    namespace lc_internal {
        class DrawCall;
    }
    /*
        Layers:
        
        500 -- Boxes
        600 -- Guys
        
    */

    class Flusher {
    public:
        void partialFlush(int upperLimit);
    private:
        friend class LayeredCanvas;
        Flusher(LayeredCanvas* canvas);
        LayeredCanvas* canvas;
        boost::container::vector<lc_internal::DrawCall>::iterator upperBound;
    };
    
    class LayeredCanvas {
    public:
        LayeredCanvas(Canvas& canvas);
        void drawRect(int layer, int x, int y, int width, int height, unsigned colour);
        void drawLine(int layer, int xa, int ya, int xb, int yb, int width, unsigned colour);
        void drawText(int layer, std::string const& text, int x, int y, int size, unsigned colour);
        Flusher getFlusher();
        
        ~LayeredCanvas();
    private:
        friend class Flusher;
        Canvas* canvas;
        boost::container::vector<lc_internal::DrawCall> drawCalls;
    };
}//namespace hg
#endif //HG_LAYERED_CANVAS_H
