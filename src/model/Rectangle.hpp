#ifndef MODEL_RECTANGLE
#define MODEL_RECTANGLE

#include <limits>
#include <math.h>
#include <cmath>

namespace model {

    class Rectangle {
        public:
            double x;
            double y;
            double width;
            double height;

        public:
            static Rectangle getBoundingBox(const Rectangle& rect1, const Rectangle& rect2) {
                double x1 = std::min(rect1.x, rect2.x);
                double y1 = std::min(rect1.y, rect2.y);
                double x2 = std::max(rect1.x + rect1.width, rect2.x + rect2.width);
                double y2 = std::max(rect1.y + rect1.height, rect2.y + rect2.height);
                return Rectangle(x1, y1, x2 - x1, y2 - y1);
            }

            static Rectangle getIntersection(const Rectangle& rect1, const Rectangle& rect2) {
                double x1 = std::max(rect1.x, rect2.x);
                double y1 = std::max(rect1.y, rect2.y);
                double x2 = std::min(rect1.x + rect1.width, rect2.x + rect2.width);
                double y2 = std::min(rect1.y + rect1.height, rect2.y + rect2.height);

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
            
            explicit Rectangle(double x, double y, double width, double height) :
                x(x), y(y), width(width), height(height) {}

            double area() const {
                return width * height;
            }

            double centerX() const {
                return x + width / 2.0;
            }

            double centerY() const {
                return y + height / 2.0;
            }

            bool isOverlappingWith(const Rectangle& other) const {
                return x < other.x + other.width
                    && x + width > other.x
                    && y < other.y + other.height
                    && y + height > other.y;
            }

            bool operator== (const Rectangle& other) const {
                return std::abs(x - other.x) < std::numeric_limits<double>::epsilon()
                    && std::abs(y - other.y) < std::numeric_limits<double>::epsilon()
                    && std::abs(width - other.width) < std::numeric_limits<double>::epsilon()
                    && std::abs(height - other.height) < std::numeric_limits<double>::epsilon();
            }

            bool operator!= (const Rectangle& other) const {
                return !(*this == other);
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
                    res = res * 31 + std::hash<double>()( o.x );
                    res = res * 31 + std::hash<double>()( o.y );
                    res = res * 31 + std::hash<double>()( o.width );
                    res = res * 31 + std::hash<double>()( o.height );
                    return res;
                }
            };
    };
}

#endif // MODEL_RECTANGLE