#ifndef MODEL_VIDEO_PROPERTIES
#define MODEL_VIDEO_PROPERTIES

namespace model {

    class VideoProperties {
        public:
            int nbFrames;
            int fps;
            int frameWidth;
            int frameHeight;
        
        public:
            VideoProperties() {}

            VideoProperties(int nbFrames, int fps, int frameWidth, int frameHeight) :
                nbFrames(nbFrames), fps(fps), frameWidth(frameWidth), frameHeight(frameHeight) {}
    };
}

#endif // MODEL_VIDEO_PROPERTIES