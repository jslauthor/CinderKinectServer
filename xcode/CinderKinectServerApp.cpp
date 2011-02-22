#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
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

	void gestureRecognized( GestureEvent event );
	void gestureProcessed( GestureEvent event );
	void handBegan( HandEvent event );
	void handMoved( HandEvent event );
	void handEnded( HandEvent event );
	
	OpenNIWrapper* mOpenNIWrapper;
};

/*
 *
 * Event Listeners (Call backs)
 *
 */

void CinderKinectServerApp::gestureRecognized( GestureEvent event )
{
	app::console() <<  "Gesture Recognized: " << event.getGesture() << " " << event.getX() << " " << event.getY() << " " << event.getZ() << endl;
	mOpenNIWrapper->startHandsTracking(Vec3f( event.getX(), event.getY(), event.getZ() ));
}

void CinderKinectServerApp::gestureProcessed( GestureEvent event )
{
	app::console() <<  "Gesture Recognized: " << event.getGesture() << " " << event.getX() << " " << event.getY() << " " << event.getZ() << endl;
}

void CinderKinectServerApp::handBegan( HandEvent event )
{
	app::console() << "handBegan" << endl;
}

void CinderKinectServerApp::handMoved( HandEvent event )
{
	app::console() <<  "Hand Moving: " << event.getId() << " " << event.getX() << " " << event.getY() << " " << event.getZ() << endl;
}

void CinderKinectServerApp::handEnded( HandEvent event )
{
	app::console() << "handEnded" << endl;
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

void CinderKinectServerApp::setup()
{
	mOpenNIWrapper = new OpenNIWrapper();
	mOpenNIWrapper->start();

	mOpenNIWrapper->registerGestureRecognized(this, &CinderKinectServerApp::gestureRecognized);
	mOpenNIWrapper->registerGestureProcessed(this, &CinderKinectServerApp::gestureProcessed);
	mOpenNIWrapper->registerHandBegan(this, &CinderKinectServerApp::handBegan);
	mOpenNIWrapper->registerHandMoved(this, &CinderKinectServerApp::handMoved);
	mOpenNIWrapper->registerHandEnded(this, &CinderKinectServerApp::handEnded);
	
	//mOpenNIWrapper->listGestures();
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

