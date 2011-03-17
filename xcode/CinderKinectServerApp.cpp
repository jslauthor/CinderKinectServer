#include "CinderKinectServerApp.h"

/*
 *
 * Event Listeners (Call backs)
 *
 */

void CinderKinectServerApp::gestureRecognized( GestureEvent event )
{
	//app::console() <<  "Gesture Recognized: " << event.getGesture() << " " << event.getX() << " " << event.getY() << " " << event.getZ() << endl;
	
	if (boost::iequals(event.getGesture(), "Wave"))
	{
		mOpenNIWrapper->startHandsTracking(Vec3f( event.getX(), event.getY(), event.getZ() ));
	}
}

void CinderKinectServerApp::gestureProcessed( GestureEvent event )
{
	//app::console() <<  "Gesture Recognized: " << event.getGesture() << " " << event.getX() << " " << event.getY() << " " << event.getZ() << endl;
}

void CinderKinectServerApp::handBegan( HandEvent event )
{
	//app::console() << "handBegan" << endl;
}

void CinderKinectServerApp::handMoved( HandEvent event )
{
	//app::console() <<  "Hand Moving: " << event.getId() << " " << event.getX() << " " << event.getY() << " " << event.getZ() << endl;
}

void CinderKinectServerApp::handEnded( HandEvent event )
{
	//app::console() << "handEnded" << endl;
}

void CinderKinectServerApp::newUserFound( UserEvent event )
{
	app::console() << "New user found: " << event.getUserID() << endl;
}

void CinderKinectServerApp::userLost( UserEvent event )
{
	app::console() << "User " << event.getUserID() << " lost" << endl;
}

void CinderKinectServerApp::userPoseDetected( UserEvent event )
{
	app::console() << "User " << event.getUserID() << " posed: " << event.getStatus() << endl;
}

void CinderKinectServerApp::userCalibrationStart( UserEvent event )
{
	app::console() << "User " << event.getUserID() << " calibration started" << endl;
}

void CinderKinectServerApp::userCalibrationEnd( UserEvent event )
{
	app::console() << "User " << event.getUserID() << " calibration ended" << endl;
}

void CinderKinectServerApp::userSkeletonUpdate( SkeletonEvent event )
{
	Skeleton skeleton = event.getSkeleton();
    int userID = event.getUserID();
    
    int size = 4 + 12 * 24;
    unsigned char *msg = new unsigned char[size];
    memcpy(msg, &userID, 4);
    
    if (skeleton.head != 0) putJoint(skeleton.head, msg + 4 + 12 * 0);
    if (skeleton.neck != 0) putJoint(skeleton.neck, msg + 4 + 12 * 1);
    if (skeleton.waist != 0) putJoint(skeleton.waist, msg + 4 + 12 * 2);
    if (skeleton.torso != 0) putJoint(skeleton.torso, msg + 4 + 12 * 3);
    
    if (skeleton.leftCollar != 0) putJoint(skeleton.leftCollar, msg + 4 + 12 * 4);
    if (skeleton.leftShoulder != 0) putJoint(skeleton.leftShoulder, msg + 4 + 12 * 5);
    if (skeleton.leftElbow != 0) putJoint(skeleton.leftElbow, msg + 4 + 12 * 6);
    if (skeleton.leftWrist != 0) putJoint(skeleton.leftWrist, msg + 4 + 12 * 7);
    if (skeleton.leftHand != 0) putJoint(skeleton.leftHand, msg + 4 + 12 * 8);
    if (skeleton.leftFingertip != 0) putJoint(skeleton.leftFingertip, msg + 4 + 12 * 9);

    if (skeleton.rightCollar != 0) putJoint(skeleton.rightCollar, msg + 4 + 12 * 10);
    if (skeleton.rightShoulder != 0) putJoint(skeleton.rightShoulder, msg + 4 + 12 * 11);
    if (skeleton.rightElbow != 0) putJoint(skeleton.rightElbow, msg + 4 + 12 * 12);
    if (skeleton.rightWrist != 0) putJoint(skeleton.rightWrist, msg + 4 + 12 * 13);
    if (skeleton.rightHand != 0) putJoint(skeleton.rightHand, msg + 4 + 12 * 14);
    if (skeleton.rightFingertip != 0) putJoint(skeleton.rightFingertip, msg + 4 + 12 * 15);
	
    if (skeleton.leftHip != 0) putJoint(skeleton.leftHip, msg + 4 + 12 * 16);
    if (skeleton.leftKnee != 0) putJoint(skeleton.leftKnee, msg + 4 + 12 * 17);
    if (skeleton.leftAnkle != 0) putJoint(skeleton.leftAnkle, msg + 4 + 12 * 18);
    if (skeleton.leftFoot != 0) putJoint(skeleton.leftFoot, msg + 4 + 12 * 19);
    
    if (skeleton.rightHip != 0) putJoint(skeleton.rightHip, msg + 4 + 12 * 20);
    if (skeleton.rightKnee != 0) putJoint(skeleton.rightKnee, msg + 4 + 12 * 21);
    if (skeleton.rightAnkle != 0) putJoint(skeleton.rightAnkle, msg + 4 + 12 * 22);
    if (skeleton.rightFoot != 0) putJoint(skeleton.rightFoot, msg + 4 + 12 * 23);
    
    server->sendMessage(SKELETAL_EVENT, USER_SKELETON_UPDATE, msg, size);
}

void CinderKinectServerApp::putJoint(Vec3f joint, unsigned char *dest)
{
    memcpy(dest, &joint.x, 4);
    memcpy(dest+4, &joint.y, 4);
    memcpy(dest+8, &joint.z, 4);
}

/*
 *
 * Cinder App Lifecycle Functions
 *
 */

void CinderKinectServerApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1024, 768 );
    settings->setFrameRate( 60.0f );
}

void CinderKinectServerApp::serverThreadFunc()
{
    boost::asio::io_service::work RunForever(io_service); 
    io_service.run(); 
}

void CinderKinectServerApp::setup()
{
    try
    {
        app::console() << "Starting server" << endl;
        server = new UDPServer(io_service);
        serverThread = std::shared_ptr<boost::thread>(new boost::thread(&CinderKinectServerApp::serverThreadFunc, this));
        app::console() << "Server started" << endl;
    }
    catch (std::exception& e)
    {
        app::console() << e.what() << endl;
    }

	mOpenNIWrapper = new OpenNIWrapper();
	mOpenNIWrapper->start();

	mOpenNIWrapper->registerGestureRecognized(this, &CinderKinectServerApp::gestureRecognized);
	mOpenNIWrapper->registerGestureProcessed(this, &CinderKinectServerApp::gestureProcessed);
	mOpenNIWrapper->registerHandBegan(this, &CinderKinectServerApp::handBegan);
	mOpenNIWrapper->registerHandMoved(this, &CinderKinectServerApp::handMoved);
	mOpenNIWrapper->registerHandEnded(this, &CinderKinectServerApp::handEnded);
	mOpenNIWrapper->registerNewUser(this, &CinderKinectServerApp::newUserFound);
	mOpenNIWrapper->registerUserLost(this, &CinderKinectServerApp::userLost);
	mOpenNIWrapper->registerUserPoseDetected(this, &CinderKinectServerApp::userPoseDetected);
	mOpenNIWrapper->registerUserStartCalibration(this, &CinderKinectServerApp::userCalibrationStart);
	mOpenNIWrapper->registerUserCalibration(this, &CinderKinectServerApp::userCalibrationEnd);
	mOpenNIWrapper->registerSkeletons(this, &CinderKinectServerApp::userSkeletonUpdate);
}

void CinderKinectServerApp::mouseDown( MouseEvent event )
{
}

void CinderKinectServerApp::update()
{
}

void CinderKinectServerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	
	if (mOpenNIWrapper->getDepthSurface() != 0)
	{
		gl::draw(gl::Texture(mOpenNIWrapper->getDepthSurface()), getWindowBounds());	
	}
}

CINDER_APP_BASIC( CinderKinectServerApp, RendererGl )