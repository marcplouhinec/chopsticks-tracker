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
            static Rectangle getBoundingBox(Rectangle& rect1, Rectangle& rect2) {
                int x1 = std::min(rect1.x, rect2.x);
                int y1 = std::min(rect1.y, rect2.y);
                int x2 = std::max(rect1.x + rect1.width, rect2.x + rect2.width);
                int y2 = std::max(rect1.y + rect1.height, rect2.y + rect2.height);
                return Rectangle(x1, y1, x2 - x1, y2 - y1);
            }

            static Rectangle getIntersection(Rectangle& rect1, Rectangle& rect2) {
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

        public:
            Rectangle() {}
            
            explicit Rectangle(int x, int y, int width, int height) :
                x(x), y(y), width(width), height(height) {}

            int area() {
                return width * height;
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