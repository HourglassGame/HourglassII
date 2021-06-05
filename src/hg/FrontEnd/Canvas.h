#ifndef HG_CANVAS_H
#define HG_CANVAS_H
#include <string>
namespace hg {
	class Canvas {
	public:
		virtual void playSound(std::string const &key, int n) = 0;
		virtual void drawRect(float x, float y, float width, float height, unsigned colour) = 0;
		virtual void drawCircle(float x, float y, float radius, unsigned colour) = 0;
		virtual void drawLine(float xa, float ya, float xb, float yb, float width, unsigned colour) = 0;
		virtual void drawText(std::string const &text, float x, float y, float size, unsigned colour) = 0;
		virtual void drawImage(std::string const &key, float x, float y, float width, float height) = 0;
		virtual void flushFrame() = 0;
	protected:
		~Canvas() noexcept {}
	};
}//namespace hg
#endif //HG_CANVAS_H
