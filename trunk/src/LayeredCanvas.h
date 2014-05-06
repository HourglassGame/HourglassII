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
        Flusher(LayeredCanvas *canvas);
        LayeredCanvas *canvas;
        boost::container::vector<lc_internal::DrawCall>::iterator upperBound;
    };
    
    class LayeredCanvas {
    public:
        LayeredCanvas(Canvas &canvas);
        void drawRect(int layer, float x, float y, float width, float height, unsigned colour);
        void drawLine(int layer, float xa, float ya, float xb, float yb, float width, unsigned colour);
        void drawText(int layer, std::string const &text, float x, float y, float size, unsigned colour);
        void drawImage(int layer, std::string const &key, float x, float y, float width, float height);
        Flusher getFlusher();
        
        ~LayeredCanvas() noexcept;
    private:
        friend class Flusher;
        Canvas *canvas;
        boost::container::vector<lc_internal::DrawCall> drawCalls;
    };
}//namespace hg
#endif //HG_LAYERED_CANVAS_H
