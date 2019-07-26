#ifndef MODEL_FRAME_OFFSET
#define MODEL_FRAME_OFFSET

namespace model {

    class FrameOffset {
        public:
            double dx = 0;
            double dy = 0;

        public:
            FrameOffset() {}

            explicit FrameOffset(double dx, double dy) : dx(dx), dy(dy) {}

            FrameOffset& operator+=(const FrameOffset& other) {
                *this = *this + other;
                return *this;
            }

            FrameOffset operator+(const FrameOffset& other) const {
                return FrameOffset(dx + other.dx, dy + other.dy);
            }

            FrameOffset& operator-=(const FrameOffset& other) {
                *this = *this + other;
                return *this;
            }

            FrameOffset operator-(const FrameOffset& other) const {
                return FrameOffset(dx - other.dx, dy - other.dy);
            }
    };
}

#endif // MODEL_FRAME_OFFSET