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
	
	void OpenNIWrapper::OpenNIProxy::addUserNode()
	{
		status = userGenerator.Create(context); 
		checkStatus();
		status = context.FindExistingNode(XN_NODE_TYPE_USER, userGenerator);
		checkStatus();
		
		if (userGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON)) 
		{
			userGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, userCallbackHandle);
			userGenerator.GetSkeletonCap().RegisterCalibrationCallbacks(UserCalibration_CalibrationStart, UserCalibration_CalibrationEnd, NULL, calibrationCallbackHandle);
			
			if (userGenerator.GetSkeletonCap().NeedPoseForCalibration()) 
			{
				mNeedPose = TRUE;
				if (userGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
				{
					userGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks(UserPose_PoseDetected, NULL, NULL, poseCallbackHandle);
					userGenerator.GetSkeletonCap().GetCalibrationPose(mStrPose);
					userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
				}
				else 
				{
					app::console() << "Pose required, but not supported" << endl;
				}
			}
		}
		else
		{
			app::console() << "Supplied user generator doesn't support skeleton" << endl;
		}
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
		
		mNeedPose = FALSE;
		mJointConfidence = 0.5;
		mNumUsers = 15;
		
		EnumerationErrors errors;
		status = context.InitFromXmlFile("/Users/creativism/Documents/Dropbox/workspace/CinderKinectServer/xcode/Sample-User.xml", &errors);
		checkStatus();
		
		// Depth Generator
		status = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depthGenerator);
		checkStatus();
		
		// Hands Generator
		status = context.FindExistingNode(XN_NODE_TYPE_HANDS, handsGenerator);
		checkStatus();
		handsGenerator.RegisterHandCallbacks(Hand_Create, Hand_Update, Hand_Destroy, this, handsCallbackHandle); 
		
		// Gesture Generator
		status = context.FindExistingNode(XN_NODE_TYPE_GESTURE, gestureGenerator);
		checkStatus();		
		gestureGenerator.RegisterGestureCallbacks(Gesture_Recognized, Gesture_Process, this, gesturesCallbackHandle); 
		
		// User Generator (skeleton and poses)
		status = context.FindExistingNode(XN_NODE_TYPE_USER, userGenerator);
		checkStatus();	
		
		if (userGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON)) 
		{
			userGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, this, userCallbackHandle);
			userGenerator.GetSkeletonCap().RegisterCalibrationCallbacks(UserCalibration_CalibrationStart, UserCalibration_CalibrationEnd, this, calibrationCallbackHandle);
			
			if (userGenerator.GetSkeletonCap().NeedPoseForCalibration()) 
			{
				mNeedPose = TRUE;
				if (userGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
				{
					userGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks(UserPose_PoseDetected, NULL, this, poseCallbackHandle);
					userGenerator.GetSkeletonCap().GetCalibrationPose(mStrPose);
					userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
				}
				else 
				{
					app::console() << "Pose required, but not supported" << endl;
				}
			}
		}
		else
		{
			app::console() << "Supplied user generator doesn't support skeleton" << endl;
		}
		
		// Start Generating all
		status = context.StartGeneratingAll();
		checkStatus();
		
		// Add gestures to look for
		gestureGenerator.AddGesture("Wave", NULL); 
		gestureGenerator.AddGesture("RaiseHand", NULL); 
		gestureGenerator.AddGesture("Click", NULL); 
		gestureGenerator.AddGesture("RiseHand", NULL); 
		gestureGenerator.AddGesture("Swipe_Right", NULL); 
		gestureGenerator.AddGesture("Swipe_Left", NULL); 
		gestureGenerator.AddGesture("Right", NULL); 
		
		// Start thread
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
			
			updateSkeletons();
		}
	}
	
	void OpenNIWrapper::OpenNIProxy::updateSkeletons()
	{
		XnUserID aUsers[MAX_USERS];
		XnUInt16 nUsers = MAX_USERS;
		userGenerator.GetUsers(aUsers, nUsers);
		
		for (int i = 0; i < nUsers; ++i)
		{
			if (userGenerator.GetSkeletonCap().IsTracking(aUsers[i]))
			{
				Skeleton skeleton;

				getJoint(aUsers[i], XN_SKEL_HEAD, skeleton.head);
				getJoint(aUsers[i], XN_SKEL_NECK, skeleton.neck);
				getJoint(aUsers[i], XN_SKEL_WAIST, skeleton.waist);
				getJoint(aUsers[i], XN_SKEL_TORSO, skeleton.torso);
				
				getJoint(aUsers[i], XN_SKEL_LEFT_COLLAR, skeleton.leftCollar);
				getJoint(aUsers[i], XN_SKEL_LEFT_SHOULDER, skeleton.leftShoulder);
				getJoint(aUsers[i], XN_SKEL_LEFT_ELBOW, skeleton.leftElbow);
				getJoint(aUsers[i], XN_SKEL_LEFT_WRIST, skeleton.leftWrist);
				getJoint(aUsers[i], XN_SKEL_LEFT_HAND, skeleton.leftHand);
				getJoint(aUsers[i], XN_SKEL_LEFT_FINGERTIP, skeleton.leftFingertip);

				getJoint(aUsers[i], XN_SKEL_RIGHT_COLLAR, skeleton.rightCollar);
				getJoint(aUsers[i], XN_SKEL_RIGHT_SHOULDER, skeleton.rightShoulder);
				getJoint(aUsers[i], XN_SKEL_RIGHT_ELBOW, skeleton.rightElbow);
				getJoint(aUsers[i], XN_SKEL_RIGHT_WRIST, skeleton.rightWrist);
				getJoint(aUsers[i], XN_SKEL_RIGHT_HAND, skeleton.rightHand);
				getJoint(aUsers[i], XN_SKEL_RIGHT_FINGERTIP, skeleton.rightFingertip);
				
				getJoint(aUsers[i], XN_SKEL_LEFT_HIP, skeleton.leftHip);
				getJoint(aUsers[i], XN_SKEL_LEFT_KNEE, skeleton.leftKnee);
				getJoint(aUsers[i], XN_SKEL_LEFT_ANKLE, skeleton.leftAnkle);
				getJoint(aUsers[i], XN_SKEL_LEFT_FOOT, skeleton.leftFoot);
				
				getJoint(aUsers[i], XN_SKEL_RIGHT_HIP, skeleton.rightHip);
				getJoint(aUsers[i], XN_SKEL_RIGHT_KNEE, skeleton.rightKnee);
				getJoint(aUsers[i], XN_SKEL_RIGHT_ANKLE, skeleton.rightAnkle);
				getJoint(aUsers[i], XN_SKEL_RIGHT_FOOT, skeleton.rightFoot);
				
				skeletonCallbacks.call( SkeletonEvent(aUsers[i], skeleton) ); 
			}
		}
	}
	
	void OpenNIWrapper::OpenNIProxy::getJoint(XnUserID player, XnSkeletonJoint jointType, Vec3f &jointPosition)
	{
		XnSkeletonJointPosition joint;
		userGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, jointType, joint);
		if (joint.fConfidence < JOINT_CONFIDENCE) return;
		
		XnPoint3D point[1];
		point[0] = joint.position;
		
		depthGenerator.ConvertProjectiveToRealWorld(1, point, point);
        float _x, _y, _z;
        _x = point[0].X;
        _y = point[0].Y;
        _z = point[0].Z;
        
		jointPosition = Vec3f(_x, _y, _z);
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
        mOpenNIProxy->handBeganCallbacks.call(HandEvent(nId, pPosition->X, pPosition->Y, pPosition->Z)); 
	} 
	void XN_CALLBACK_TYPE OpenNIWrapper::Hand_Update(xn::HandsGenerator& generator, XnUserID nId, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie) 
	{ 
		OpenNIWrapper::OpenNIProxy *mOpenNIProxy = reinterpret_cast<OpenNIWrapper::OpenNIProxy*>( pCookie );
        mOpenNIProxy->handMovedCallbacks.call(HandEvent(nId, pPosition->X, pPosition->Y, pPosition->Z)); 
	} 
	
	void XN_CALLBACK_TYPE OpenNIWrapper::Hand_Destroy(xn::HandsGenerator& generator, XnUserID nId, XnFloat fTime, void* pCookie)
	{ 
		OpenNIWrapper::OpenNIProxy *mOpenNIProxy = reinterpret_cast<OpenNIWrapper::OpenNIProxy*>( pCookie );
        mOpenNIProxy->handEndedCallbacks.call(HandEvent(nId, 0, 0, 0)); 
	} 
	
	// NITE Callbacks
	
	void XN_CALLBACK_TYPE OpenNIWrapper::User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
	{
		OpenNIWrapper::OpenNIProxy *mOpenNIProxy = reinterpret_cast<OpenNIWrapper::OpenNIProxy*>( pCookie );
		mOpenNIProxy->newUserCallbacks.call(UserEvent(nId, "newUser")); 

		if (mOpenNIProxy->mNeedPose)
		{
			mOpenNIProxy->userGenerator.GetPoseDetectionCap().StartPoseDetection(mOpenNIProxy->mStrPose, nId);
		}
		else
		{
			mOpenNIProxy->userGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
		}
	}
	
	void XN_CALLBACK_TYPE OpenNIWrapper::User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
	{
		OpenNIWrapper::OpenNIProxy *mOpenNIProxy = reinterpret_cast<OpenNIWrapper::OpenNIProxy*>( pCookie );
		mOpenNIProxy->userLostCallbacks.call(UserEvent(nId, "lostUser")); 
	}

	void XN_CALLBACK_TYPE OpenNIWrapper::UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
	{
		OpenNIWrapper::OpenNIProxy *mOpenNIProxy = reinterpret_cast<OpenNIWrapper::OpenNIProxy*>( pCookie );
		mOpenNIProxy->userPoseCallbacks.call(UserEvent(nId, strPose)); 
		
		mOpenNIProxy->userGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
		mOpenNIProxy->userGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
	}
	
	void XN_CALLBACK_TYPE OpenNIWrapper::UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
	{
		OpenNIWrapper::OpenNIProxy *mOpenNIProxy = reinterpret_cast<OpenNIWrapper::OpenNIProxy*>( pCookie );
		mOpenNIProxy->userCalibrationStartCallbacks.call(UserEvent(nId, "calibrationStart")); 
	}
	
	void XN_CALLBACK_TYPE OpenNIWrapper::UserCalibration_CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie)
	{
		OpenNIWrapper::OpenNIProxy *mOpenNIProxy = reinterpret_cast<OpenNIWrapper::OpenNIProxy*>( pCookie );
		mOpenNIProxy->userCalibrationCallbacks.call(UserEvent(nId, "calibrationEnd")); 
		
		if (bSuccess)
		{
			// Calibration succeeded
			
			app::console() << "Calibration complete, start tracking user " <<  nId <<endl;
			mOpenNIProxy->userGenerator.GetSkeletonCap().StartTracking(nId);
		}
		else
		{
			// Calibration failed
			app::console() << "Calibration failed for user " << nId << endl;
			if (mOpenNIProxy->mNeedPose){
				mOpenNIProxy->userGenerator.GetPoseDetectionCap().StartPoseDetection(mOpenNIProxy->mStrPose, nId);
			}
			else
			{
				mOpenNIProxy->userGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
			}
		}
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