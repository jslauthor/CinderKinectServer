/*
 *  OpenNIWrapper.h
 *  CinderKinectServer
 *
 *  Created by Leonard Souza on 02/21/2011.
 *  Copyright 2011 Leonard Souza. All rights reserved.
 *
 */

#pragma once

// Cinder Includes

#include "Cinder/app/App.h"
#include "cinder/Cinder.h"
#include "cinder/Thread.h"
#include "cinder/Vector.h"
#include "cinder/Area.h"
#include "cinder/Exception.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Surface.h"
#include "cinder/Stream.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Function.h"
#include "cinder/Font.h"

// Standard Includes

#include <map>

// OpenNI Includes

#include <XnOpenNI.h>
#include <XnCppWrapper.h>
#include <XnCodecIDs.h>
#include <XnHash.h>
#include <XnLog.h>

// Namespace Usages

using namespace xn;
using namespace std;
using namespace ci;
using namespace cinder;

// Constants

#define MAX_DEPTH 10000

// Class Declarations

namespace cinder 
{
	class HandEvent 
	{
		public:
			HandEvent(int id, float x, float y, float z) : mId(id), mX(x), mY(y), mZ(z) {}
			
			int			getId() const { return mId; }
			
			float		getX() const { return mX; }
			float		getY() const { return mY; }
			float		getZ() const { return mZ; }
			
			Vec3f		getPos() const { return Vec3f( mX, mY, mZ ); }
			
			float	mX, mY, mZ;
			int		mId;
	};
	
	class GestureEvent 
	{
		public:
			GestureEvent(std::string gesture, float x, float y, float z, float progress = 1.0f) : mGesture(gesture), mX(x), mY(y), mZ(z), mProgress(progress) {} 
			
			std::string getGesture() const { return mGesture; }
			
			float		getX() const { return mX; }
			float		getY() const { return mY; }
			float		getZ() const { return mZ; }
			
			Vec3f		getPos() const { return Vec3f( mX, mY, mZ ); }
			
			float getProgress() const { return mProgress; }
			
			std::string mGesture;
			float		mProgress;
			float		mX, mY, mZ;
	};

	/*
	class UserEvent 
	{
		public:
			//UserEvent(int userID, std::string status, SkeletalEvent event) : 
	};
	
	class SkeletalEvent 
	{
		
	};
	*/
	 
	class OpenNIWrapper 
	{		
		public:
			OpenNIWrapper();
			~OpenNIWrapper();

			void start();
			void stop();
			
			void addGesture(std::string gesture);
			void removeGesture(std::string gesture);
			void listGestures();
			
			void startHandsTracking(Vec3f position);
			void stopHandsTracking(int userId);
			void stopAllHandsTracking();
			
			Surface8u getDepthSurface();
		
			template<typename T>
			CallbackId		registerGestureRecognized( T *obj, void (T::*callback)(GestureEvent) ) { if(!mOpenNIProxy->gestureGenerator.IsValid()) mOpenNIProxy->addGestureNode(); return mOpenNIProxy->gestureRecognizedCallbacks.registerCb( std::bind1st( std::mem_fun( callback ), obj ) ); }
			void			unregisterGestureRecognized( CallbackId id ) { mOpenNIProxy->gestureRecognizedCallbacks.unregisterCb( id ); }
			
			template<typename T>
			CallbackId		registerGestureProcessed( T *obj, void (T::*callback)(GestureEvent) ) { if(!mOpenNIProxy->gestureGenerator.IsValid()) mOpenNIProxy->addGestureNode(); return mOpenNIProxy->gestureProcessedCallbacks.registerCb( std::bind1st( std::mem_fun( callback ), obj ) ); }
			void			unregisterGestureProcessed( CallbackId id ) { mOpenNIProxy->gestureProcessedCallbacks.unregisterCb( id ); }
			
			template<typename T>
			CallbackId		registerHandBegan( T *obj, void (T::*callback)(HandEvent) ) { if(!mOpenNIProxy->handsGenerator.IsValid()) mOpenNIProxy->addHandsNode(); return mOpenNIProxy->handBeganCallbacks.registerCb( std::bind1st( std::mem_fun( callback ), obj ) ); }
			void			unregisterHandBegan( CallbackId id ) { mOpenNIProxy->handBeganCallbacks.unregisterCb( id ); }
			
			template<typename T>
			CallbackId		registerHandMoved( T *obj, void (T::*callback)(HandEvent) ) { if(!mOpenNIProxy->handsGenerator.IsValid()) mOpenNIProxy->addHandsNode(); return mOpenNIProxy->handMovedCallbacks.registerCb( std::bind1st( std::mem_fun( callback ), obj ) ); }
			void			unregisterHandMoved( CallbackId id ) { mOpenNIProxy->handMovedCallbacks.unregisterCb( id ); }
			
			template<typename T>
			CallbackId		registerHandEnded( T *obj, void (T::*callback)(HandEvent) ) { if(!mOpenNIProxy->handsGenerator.IsValid()) mOpenNIProxy->addHandsNode(); return mOpenNIProxy->handEndedCallbacks.registerCb( std::bind1st( std::mem_fun( callback ), obj ) ); }
			void			unregisterHandEnded( CallbackId id ) { mOpenNIProxy->handEndedCallbacks.unregisterCb( id ); }
			
		protected:
			static void XN_CALLBACK_TYPE Gesture_Recognized(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie);
			static void XN_CALLBACK_TYPE Gesture_Process(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pPosition, XnFloat fProgress, void* pCookie);
			
			static void XN_CALLBACK_TYPE Hand_Create(xn::HandsGenerator& generator, XnUserID nId, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie);
			static void XN_CALLBACK_TYPE Hand_Update(xn::HandsGenerator& generator, XnUserID nId, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie);
			static void XN_CALLBACK_TYPE Hand_Destroy(xn::HandsGenerator& generator, XnUserID nId, XnFloat fTime, void* pCookie);
			
			struct OpenNIProxy 
			{
				OpenNIProxy();
				~OpenNIProxy();
				
				void addHandsNode();
				void addGestureNode();
				
				void start();
				
				void threadedFunc();
				void checkStatus();
				void drawDepthGL();
				
				Surface8u getDepthSurface();
				
				Context								context;
				XnStatus							status;
				
				HandsGenerator						handsGenerator;
				GestureGenerator					gestureGenerator; 
				DepthGenerator						depthGenerator;
				UserGenerator						userGenerator;

				Surface8u							mDepthSurface;
				
				float								pDepthHist[MAX_DEPTH];
				SceneMetaData						mSceneMD;
				DepthMetaData						mDepthMD;
				
				std::shared_ptr<std::thread>		mThread;
				std::mutex							mMutex;
				
				bool								mThreadRunning;
				
				XnCallbackHandle					gesturesCallbackHandle;
				XnCallbackHandle					handsCallbackHandle;
				
				CallbackMgr<void (GestureEvent)>	gestureRecognizedCallbacks;
				CallbackMgr<void (GestureEvent)>	gestureProcessedCallbacks;
				
				CallbackMgr<void (HandEvent)> 		handBeganCallbacks;
				CallbackMgr<void (HandEvent)> 		handMovedCallbacks;
				CallbackMgr<void (HandEvent)> 		handEndedCallbacks;
			};
			
			std::shared_ptr<OpenNIProxy>	mOpenNIProxy;
	};	
}