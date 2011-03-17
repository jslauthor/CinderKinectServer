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
            TUIO_EVENT      = 3
        };
    
        enum SkeletalEvent 
        { 
            USER_SKELETON_UPDATE = 0
        };
};

