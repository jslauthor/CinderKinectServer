#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Thread.h"
#include "UDPServer.h"
#include "OpenNIWrapper.h"

using namespace cinder;
using namespace ci;
using namespace ci::app;
using namespace std;

class CinderKinectServerApp : public AppBasic 
{
    public:
        void prepareSettings( Settings *settings );
        void setup();
        void mouseDown( MouseEvent event );	
        void update();
        void draw();
        void serverThreadFunc();
        void putJoint(Vec3f joint, unsigned char *dest);
    
        void gestureRecognized( GestureEvent event );
        void gestureProcessed( GestureEvent event );
        void handBegan( HandEvent event );
        void handMoved( HandEvent event );
        void handEnded( HandEvent event );
        void newUserFound( UserEvent event );
        void userLost( UserEvent event );
        void userPoseDetected( UserEvent event );
        void userCalibrationStart( UserEvent event );
        void userCalibrationEnd( UserEvent event );
        void userSkeletonUpdate( SkeletonEvent event );
        
        OpenNIWrapper* mOpenNIWrapper;
    
        std::shared_ptr<std::thread>    serverThread;    

        boost::asio::io_service         io_service;
        UDPServer*                      server;
        std::mutex						mMutex;
    
        enum EventType 
        { 
            SKELETAL_EVENT  = 0, 
            GESTURE_EVENT   = 1, 
            HAND_EVENT      = 2,
            USER_EVENT      = 3
        };

        enum GestureEventType
        {
            GESTURE_RECOGNIZED = 0,
            GESTURE_PROCESSED = 1
        };
    
        enum HandEventType
        {
            HAND_BEGAN = 0,
            HAND_MOVED = 1,
            HAND_ENDED = 2
        };
    
        enum UserEventType
        {
            NEW_USER_FOUND = 0,
            USER_POSE_DETECTED = 1,
            USER_CALIBRATION_START = 2,
            USER_CALIBRATION_SUCCEEDED = 3,
            USER_CALIBRATION_FAILED = 4,
            USER_LOST = 5
        };
    
        enum SkeletalEventType
        { 
            USER_SKELETON_UPDATE = 0
        };
    
        enum GESTURE_TYPE
        {
            WAVE = 0,
            RAISE_HAND = 1,
            SWIPE_LEFT = 2,
            SWIPE_RIGHT = 3,
            CLICK = 4,
        };
};

