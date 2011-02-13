#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "CinderOpenNI.h"

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
	
	gl::Texture myImage; 
	Font font;
};

void CinderKinectServerApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1024, 768 );
    settings->setFrameRate( 60.0f );
}

void CinderKinectServerApp::setup()
{

	myImage = gl::Texture( loadImage( loadResource( "image.jpg" ) ) );
	
	
	font = Font(loadResource("Arial.ttf"), 15.0f);
	CinderOpenNISkeleton::getInstance()->setup("/Users/creativism/Documents/Dropbox/workspace/CinderKinectServer/xcode/Sample-User.xml");
}

void CinderKinectServerApp::mouseDown( MouseEvent event )
{
}

void CinderKinectServerApp::update()
{
	CinderOpenNISkeleton::getInstance()->update();

}

void CinderKinectServerApp::draw()
{
	// clear out the window with black
	//gl::clear( Color( 0, 0, 0 ) ); 
	//gl::draw( myImage, getWindowBounds() );
	CinderOpenNISkeleton::getInstance()->drawDebug(font);
	
}

CINDER_APP_BASIC( CinderKinectServerApp, RendererGl )

