/*
 *  OpenNIWrapper.cpp
 *  CinderKinectServer
 *
 *  Created by Leonard Souza on 02/21/2011.
 *  Copyright 2011 Leonard Souza. All rights reserved.
 *
 */

// Includes

#include "OpenNIWrapper.h"

// Class Definition

namespace cinder 
{
	
	// Constructor (Brahma)
	
	OpenNIWrapper::OpenNIWrapper(): mOpenNIProxy(new OpenNIProxy())
	{
	}

	// Destructor (Shiva)
	
	OpenNIWrapper::~OpenNIWrapper() {}
	
	// Function Definitions
	
	void OpenNIWrapper::start()
	{
		mOpenNIProxy->start();
	}
	
	void OpenNIWrapper::stop()
	{
	}
	
	void OpenNIWrapper::addGesture( std::string gesture )
	{
		mOpenNIProxy->gestureGenerator.AddGesture( gesture.c_str(), NULL ); 
	}

	void OpenNIWrapper::removeGesture( std::string gesture )
	{
		mOpenNIProxy->gestureGenerator.RemoveGesture( gesture.c_str() ); 
	}
	
	void OpenNIWrapper::listGestures()
	{
        XnUInt16 nGestures; 
        XnChar **gestures = new XnChar*[14]; 
		
		for(int i=0; i<14; i++) gestures[i] = "Gesture not supported"; 
		
		//mOpenNIProxy->gestureGenerator.EnumerateGestures
		xnEnumerateGestures( mOpenNIProxy->gestureGenerator, gestures, &nGestures );
		
		printf( "%i Gestures\n", nGestures );

		for(int i=0; i<nGestures; i++) 
		{
			cout<<gestures[i];
		}
	}
	
	void OpenNIWrapper::startHandsTracking(Vec3f position)
	{
		XnPoint3D pos;
		pos.X = position.x;
		pos.Y = position.y;
		pos.Z = position.z;
		
		mOpenNIProxy->handsGenerator.StartTracking( pos ); 
	}
	
	void OpenNIWrapper::stopHandsTracking(int userId)
	{
		mOpenNIProxy->handsGenerator.StopTracking(userId);
	}
	
	void OpenNIWrapper::stopAllHandsTracking()
	{
		mOpenNIProxy->handsGenerator.StopTrackingAll();
	}
	
	Surface8u OpenNIWrapper::getDepthSurface()
	{
		return mOpenNIProxy->getDepthSurface();
	}
	
	/*
	 *
	 * OpenNIProxy Function Definitions
	 *
	 */
	
	OpenNIWrapper::OpenNIProxy::OpenNIProxy()
	{
		status = context.Init(); 
		checkStatus();
		mThreadRunning = true;
	}
	
	OpenNIWrapper::OpenNIProxy::~OpenNIProxy()
	{
		context.Shutdown();
		mThreadRunning = false;
		mThread->join();
	}
	
	Surface8u OpenNIWrapper::OpenNIProxy::getDepthSurface()
	{
		return mDepthSurface;
	}
	
	void OpenNIWrapper::OpenNIProxy::addHandsNode()
	{
		status = handsGenerator.Create(context); 
		checkStatus();
		status = context.FindExistingNode(XN_NODE_TYPE_HANDS, handsGenerator);
		checkStatus();
		
		handsGenerator.RegisterHandCallbacks(Hand_Create, Hand_Update, Hand_Destroy, this, handsCallbackHandle); 
	}
	
	void OpenNIWrapper::OpenNIProxy::addGestureNode()
	{
		status = gestureGenerator.Create(context); 
		checkStatus();
		status = context.FindExistingNode(XN_NODE_TYPE_GESTURE, gestureGenerator);
		checkStatus();
		
		gestureGenerator.RegisterGestureCallbacks(Gesture_Recognized, Gesture_Process, this, gesturesCallbackHandle); 
	}
	
	void OpenNIWrapper::OpenNIProxy::start()
	{
		app::console() << "Starting OpenNI" << endl;
		
		EnumerationErrors errors;
		status = context.InitFromXmlFile("/Users/creativism/Documents/Dropbox/workspace/CinderKinectServer/xcode/Sample-User.xml", &errors);
		checkStatus();
		
		status = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depthGenerator);
		checkStatus();
		
		status = context.FindExistingNode(XN_NODE_TYPE_HANDS, handsGenerator);
		checkStatus();
		handsGenerator.RegisterHandCallbacks(Hand_Create, Hand_Update, Hand_Destroy, this, handsCallbackHandle); 
		
		status = context.FindExistingNode(XN_NODE_TYPE_GESTURE, gestureGenerator);
		checkStatus();		
		gestureGenerator.RegisterGestureCallbacks(Gesture_Recognized, Gesture_Process, this, gesturesCallbackHandle); 
		
		status = context.StartGeneratingAll();
		checkStatus();
		
		gestureGenerator.AddGesture("Wave", NULL); 
		gestureGenerator.AddGesture("RaiseHand", NULL); 
		gestureGenerator.AddGesture("Click", NULL); 
		
		mThread = std::shared_ptr<boost::thread>(new boost::thread(&OpenNIWrapper::OpenNIProxy::threadedFunc, this));
	}
	
	void OpenNIWrapper::OpenNIProxy::drawDepthGL()
	{
		int width =  mDepthMD.XRes();
		int height = mDepthMD.YRes();
	
		const XnDepthPixel* pDepth = mDepthMD.Data();
		const XnLabel* pLabels = mSceneMD.Data();
		
		// Calculate the accumulative histogram
		memset(pDepthHist, 0, MAX_DEPTH*sizeof(float));
		
		unsigned int nX = 0;
		unsigned int nValue = 0;
		unsigned int nIndex = 0;
		unsigned int nY = 0;
		unsigned int nNumberOfPoints = 0;
		unsigned int nHistValue = 0;
		
		if (mDepthSurface == 0)
		{
			app::console() << "Initialising Buffer" << endl;
			mDepthSurface = Surface8u( width, height, false );
		}

		for (nY=0; nY<height; nY++)
		{
			for (nX=0; nX<width; nX++)
			{
				nValue = *pDepth;
				
				if (nValue != 0)
				{
					pDepthHist[nValue]++;
					nNumberOfPoints++;
				}
				
				pDepth++;
			}
		}
		
		for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
		{
			pDepthHist[nIndex] += pDepthHist[nIndex-1];
		}
		
		if (nNumberOfPoints)
		{
			for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
			{
				pDepthHist[nIndex] = 
				(unsigned int) (256 * (1.0f - (pDepthHist[nIndex] / nNumberOfPoints)));
			}
		}
		
		pDepth = mDepthMD.Data();
		
		Area area( 0, 0, width,height );
		
		Surface::Iter iter = mDepthSurface.getIter( area );
		while( iter.line() ) {
			while( iter.pixel() ) {
				iter.r() = 0;
				iter.g() = 0;
				iter.b() = 0;
				
				nValue = *pDepth;
				if (nValue != 0)
				{
					nHistValue = pDepthHist[nValue];
					
					iter.r() = nHistValue; 
					iter.g() = nHistValue;
					iter.b() = nHistValue;
				}
				
				pDepth++;
				pLabels++;
				
			}
		}
	}
	
	void OpenNIWrapper::OpenNIProxy::threadedFunc()
	{
		while(mThreadRunning)
		{
			boost::mutex::scoped_lock lock(mMutex); 
			status = context.WaitAndUpdateAll();
			checkStatus();
			
			depthGenerator.GetMetaData(mDepthMD);
			drawDepthGL();
		}
	}
	
	/* 
	 *
	 * Callbacks
	 *
	 */
	
	void XN_CALLBACK_TYPE OpenNIWrapper::Gesture_Recognized(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie)
	{
		OpenNIWrapper::OpenNIProxy *mOpenNIProxy = reinterpret_cast<OpenNIWrapper::OpenNIProxy*>( pCookie );
		mOpenNIProxy->gestureRecognizedCallbacks.call( GestureEvent( strGesture, pEndPosition->X, pEndPosition->Y, pEndPosition->Z ) ); 
	}
	
	void XN_CALLBACK_TYPE OpenNIWrapper::Gesture_Process(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pPosition, XnFloat fProgress, void* pCookie)
	{ 
		OpenNIWrapper::OpenNIProxy *mOpenNIProxy = reinterpret_cast<OpenNIWrapper::OpenNIProxy*>( pCookie );
		mOpenNIProxy->gestureProcessedCallbacks.call( GestureEvent( strGesture, pPosition->X, pPosition->Y, pPosition->Z ) ); 
	}
	
	void XN_CALLBACK_TYPE OpenNIWrapper::Hand_Create(xn::HandsGenerator& generator, XnUserID nId, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie)
	{ 
		OpenNIWrapper::OpenNIProxy *mOpenNIProxy = reinterpret_cast<OpenNIWrapper::OpenNIProxy*>( pCookie );
        mOpenNIProxy->handBeganCallbacks.call(HandEvent(nId,  pPosition->X, pPosition->Y, pPosition->Z)); 
	} 
	void XN_CALLBACK_TYPE OpenNIWrapper::Hand_Update(xn::HandsGenerator& generator, XnUserID nId, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie) 
	{ 
		OpenNIWrapper::OpenNIProxy *mOpenNIProxy = reinterpret_cast<OpenNIWrapper::OpenNIProxy*>( pCookie );
        mOpenNIProxy->handMovedCallbacks.call(HandEvent(nId,  pPosition->X, pPosition->Y, pPosition->Z)); 
	} 
	
	void XN_CALLBACK_TYPE OpenNIWrapper::Hand_Destroy(xn::HandsGenerator& generator, XnUserID nId, XnFloat fTime, void* pCookie)
	{ 
		OpenNIWrapper::OpenNIProxy *mOpenNIProxy = reinterpret_cast<OpenNIWrapper::OpenNIProxy*>( pCookie );
        mOpenNIProxy->handEndedCallbacks.call(HandEvent(nId,  0, 0, 0)); 
	} 
	
	/*
	 *
	 * Utility Functions
	 *
	 */
	
	void OpenNIWrapper::OpenNIProxy::checkStatus()
	{
		if(status != XN_STATUS_OK)
		{
			printf("OpenNI Error: %s\n", xnGetStatusString(status)); 
		}
	}
};