#ifndef MODEL_RECTANGLE
#define MODEL_RECTANGLE

#include <math.h>

namespace model {

    class Rectangle {
        public:
            int x;
            int y;
            int width;
            int height;

        public:
            static Rectangle getBoundingBox(const Rectangle& rect1, const Rectangle& rect2) {
                int x1 = std::min(rect1.x, rect2.x);
                int y1 = std::min(rect1.y, rect2.y);
                int x2 = std::max(rect1.x + rect1.width, rect2.x + rect2.width);
                int y2 = std::max(rect1.y + rect1.height, rect2.y + rect2.height);
                return Rectangle(x1, y1, x2 - x1, y2 - y1);
            }

            static Rectangle getIntersection(const Rectangle& rect1, const Rectangle& rect2) {
                int x1 = std::max(rect1.x, rect2.x);
                int y1 = std::max(rect1.y, rect2.y);
                int x2 = std::min(rect1.x + rect1.width, rect2.x + rect2.width);
                int y2 = std::min(rect1.y + rect1.height, rect2.y + rect2.height);

                if (x1 < x2 && y1 < y2) {
                    return Rectangle(x1, y1, x2 - x1, y2 - y1);
                } else {
                    return Rectangle(0, 0, 0, 0);
                }
            }

            static double distanceBetweenTopLeftPoints(const Rectangle& rect1, const Rectangle& rect2) {
                double dx = rect1.x - rect2.x;
                double dy = rect1.y - rect2.y;
                return sqrt(pow(dx, 2.0) + pow(dy, 2.0));
            }

        public:
            Rectangle() {}
            
            explicit Rectangle(int x, int y, int width, int height) :
                x(x), y(y), width(width), height(height) {}

            int area() const {
                return width * height;
            }

            int centerX() const {
                return x + std::round(((double) width) / 2.0);
            }

            int centerY() const {
                return y + std::round(((double) height) / 2.0);
            }

            bool isOverlappingWith(Rectangle& other) {
                return x < other.x + other.width
                    && x + width > other.x
                    && y < other.y + other.height
                    && y + height > other.y;
            }

            bool operator== (const Rectangle& other) const {
                return x == other.x
                    && y == other.y
                    && width == other.width
                    && height == other.height;
            }

            bool operator!= (const Rectangle& other) const {
                return x != other.x
                    || y != other.y
                    || width != other.width
                    || height != other.height;
            }

            bool operator< (const Rectangle& other) const {
                if (width != other.width) {
                    return width < other.width;
                }
                if (height != other.height) {
                    return height < other.height;
                }
                if (x != other.x) {
                    return x < other.x;
                }
                return y < other.y;
            }

            struct Hasher
            {
                std::size_t operator()(const Rectangle& o) const
                {
                    std::size_t res = 17;
                    res = res * 31 + std::hash<int>()( o.x );
                    res = res * 31 + std::hash<int>()( o.y );
                    res = res * 31 + std::hash<int>()( o.width );
                    res = res * 31 + std::hash<int>()( o.height );
                    return res;
                }
            };
    };
}

#endif // MODEL_RECTANGLE