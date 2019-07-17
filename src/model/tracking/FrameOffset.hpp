#ifndef MODEL_FRAME_OFFSET
#define MODEL_FRAME_OFFSET

namespace model {

    class FrameOffset {
        public:
            int dx = 0;
            int dy = 0;

        public:
            FrameOffset() {}

            explicit FrameOffset(int dx, int dy) : dx(dx), dy(dy) {}
    };
}

#endif // MODEL_FRAME_OFFSET