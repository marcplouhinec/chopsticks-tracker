#ifndef MODEL_TRACKING_STATUS
#define MODEL_TRACKING_STATUS

namespace model {

    enum class TrackingStatus {
        DETECTED, DETECTED_ONCE, NOT_DETECTED, HIDDEN_BY_ARM, LOST
    };

}

#endif // MODEL_TRACKING_STATUS