/*
					   __  .__              ________ 
   ______ ____   _____/  |_|__| ____   ____/   __   \
  /  ___// __ \_/ ___\   __\  |/  _ \ /    \____    /
  \___ \\  ___/\  \___|  | |  (  <_> )   |  \ /    / 
 /____  >\___  >\___  >__| |__|\____/|___|  //____/  .co.uk
      \/     \/     \/                    \/         
 
 THE GHOST IN THE CSH
 
 
 CinderOpenNI.cpp | Part of PhantomLimb | Created 18/01/2011
 
 Copyright (c) 2010 Benjamin Blundell, www.section9.co.uk
 *** Section9 ***
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Section9 nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***********************************************************************/

#include "CinderOpenNI.h"

#define CHECK_RC(nRetVal, what)										\
if (nRetVal != XN_STATUS_OK)									\
{																\
printf("%s failed: %s\n", what, xnGetStatusString(nRetVal));\
return nRetVal;												\
}

XnFloat Colors[][3] =
{
	{0,1,1},
	{0,0,1},
	{0,1,0},
	{1,1,0},
	{1,0,0},
	{1,.5,0},
	{.5,1,0},
	{0,.5,1},
	{.5,0,1},
	{1,1,.5},
	{1,1,1}
};
XnUInt32 nColors = 10;

unsigned int getClosestPowerOfTwo(unsigned int n)
{
	unsigned int m = 2;
	while(m < n) m<<=1;
	
	return m;
}


CinderOpenNISkeleton* CinderOpenNISkeleton::gCinderOpenNISkeleton = NULL;

CinderOpenNISkeleton* CinderOpenNISkeleton::getInstance(){
	if (!gCinderOpenNISkeleton){
		gCinderOpenNISkeleton = new CinderOpenNISkeleton();
	}
	return gCinderOpenNISkeleton;
}

/* Drawing methods */


void CinderOpenNISkeleton::drawDebug(Font font)  {
	
	XnUserID aUsers[15];
	XnUInt16 nUsers = 15;
	
	
	gCinderOpenNISkeleton->mUserGenerator.GetUsers(aUsers, nUsers);
			
	for (int i = 0; i < nUsers; ++i){
		
		// Do the labels first
		
		XnPoint3D com;
		gCinderOpenNISkeleton->mUserGenerator.GetCoM(aUsers[i], com);
		gCinderOpenNISkeleton->mDepthGenerator.ConvertRealWorldToProjective(1, &com, &com);
	
		glColor4f(1-Colors[i%nColors][0], 1-Colors[i%nColors][1], 1-Colors[i%nColors][2], 1);
		
		if (gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().IsTracking(aUsers[i]))
		{
			// Tracking
			gl::drawString("Tracking", Vec2i(com.X, com.Y), Color::white(), font );

		}
		else if (gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().IsCalibrating(aUsers[i]))
		{
			gl::drawString("Calibrating", Vec2i(com.X, com.Y), Color::white(), font );
		}
		else
		{
			gl::drawString("Looking for Pose", Vec2i(com.X, com.Y), Color::white(), font );
		}
	
		
		
		if( gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().IsTracking(aUsers[i])){ 
			
			app::console() << "Is Tracking User" << endl;
			
			glLineWidth(6.0);
			glBegin(GL_LINES);
			glColor4f(1-Colors[aUsers[i]%nColors][0], 
					  1-Colors[aUsers[i]%nColors][1], 
					  1-Colors[aUsers[i]%nColors][2], 1);
			
			
			drawLimbDebug(aUsers[i], XN_SKEL_HEAD, XN_SKEL_NECK);
			
			drawLimbDebug(aUsers[i], XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER);
			drawLimbDebug(aUsers[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW);
			drawLimbDebug(aUsers[i], XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND);
			
			drawLimbDebug(aUsers[i], XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER);
			drawLimbDebug(aUsers[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW);
			drawLimbDebug(aUsers[i], XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);
			
			drawLimbDebug(aUsers[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO);
			drawLimbDebug(aUsers[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO);
			
			drawLimbDebug(aUsers[i], XN_SKEL_TORSO, XN_SKEL_LEFT_HIP);
			drawLimbDebug(aUsers[i], XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE);
			drawLimbDebug(aUsers[i], XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT);
			
			drawLimbDebug(aUsers[i], XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP);
			drawLimbDebug(aUsers[i], XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE);
			drawLimbDebug(aUsers[i], XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT);
			
			drawLimbDebug(aUsers[i]	, XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP);
			
			glEnd();
		}
	}
}


void CinderOpenNISkeleton::drawLimbDebug(XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2)
{
	if (!gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().IsTracking(player)){
		app::console() << "not tracked" << endl;
		return;
	}
	
	XnSkeletonJointPosition joint1, joint2;
	gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint1, joint1);
	gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint2, joint2);
	
	if (joint1.fConfidence < gCinderOpenNISkeleton->mJointConfidence || joint2.fConfidence < gCinderOpenNISkeleton->mJointConfidence)
		return;
	
	XnPoint3D pt[2];
	pt[0] = joint1.position;
	pt[1] = joint2.position;
	
	gCinderOpenNISkeleton->mDepthGenerator.ConvertRealWorldToProjective(2, pt, pt);
	glVertex3i(pt[0].X, pt[0].Y, 0);
	glVertex3i(pt[1].X, pt[1].Y, 0);
	
	app::console() << "X: " << pt[0].X << " Y: " << pt[0].Y << " Z: " << pt[0].Z << endl;
}



Surface8u CinderOpenNISkeleton::getDepthSurface() {

	return gCinderOpenNISkeleton->mDepthSurface;
}

void CinderOpenNISkeleton::setDepthSurface() {
	int texWidth =  gCinderOpenNISkeleton->mDepthMD.XRes();
	int texHeight = gCinderOpenNISkeleton->mDepthMD.YRes();
	
	const XnDepthPixel* pDepth = gCinderOpenNISkeleton->mDepthMD.Data();
	const XnLabel* pLabels = gCinderOpenNISkeleton->mSceneMD.Data();
	
	// Calculate the accumulative histogram -  whatever that means
	memset(gCinderOpenNISkeleton->pDepthHist, 0, MAX_DEPTH*sizeof(float));
	
	unsigned int nX = 0;
	unsigned int nValue = 0;
	unsigned int nIndex = 0;
	unsigned int nY = 0;
	unsigned int nNumberOfPoints = 0;
	unsigned int nHistValue = 0;
	
	if(!gCinderOpenNISkeleton->bInitialized)
	{
	
		gCinderOpenNISkeleton->pDepthTexBuf = new unsigned char[texWidth*texHeight*3];
		
		app::console() << "Initialised Buffer" <<endl;
		
		gCinderOpenNISkeleton->bInitialized = true;
		
		gCinderOpenNISkeleton->mDepthSurface = Surface8u( texWidth, texHeight, false ); // width, height, alpha?

	}
	
	
	for (nY=0; nY<texHeight; nY++)
	{
		for (nX=0; nX<texWidth; nX++)
		{
			nValue = *pDepth;
			
			if (nValue != 0)
			{
				gCinderOpenNISkeleton->pDepthHist[nValue]++;
				nNumberOfPoints++;
			}
			
			pDepth++;
		}
	}
	
	for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
	{
		gCinderOpenNISkeleton->pDepthHist[nIndex] += gCinderOpenNISkeleton->pDepthHist[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
		{
			gCinderOpenNISkeleton->pDepthHist[nIndex] = 
			(unsigned int)(256 * (1.0f - (gCinderOpenNISkeleton->pDepthHist[nIndex] / nNumberOfPoints)));
		}
	}
	
	pDepth = gCinderOpenNISkeleton->mDepthMD.Data();

	Area area( 0, 0, texWidth, texHeight );

	Surface::Iter iter = gCinderOpenNISkeleton->mDepthSurface.getIter( area );
	while( iter.line() ) {
		while( iter.pixel() ) {
			iter.r() = 0;
			iter.g() = 0;
			iter.b() = 0;
			
			if ( *pLabels != 0) // Buh?
			{
				nValue = *pDepth;
				XnLabel label = *pLabels;
				XnUInt32 nColorID = label % nColors;
				if (label == 0)
				{
					nColorID = nColors;
				}
				
				if (nValue != 0)
				{
					nHistValue = gCinderOpenNISkeleton->pDepthHist[nValue];
					
					iter.r() = nHistValue * Colors[nColorID][0]; 
					iter.g() = nHistValue * Colors[nColorID][1];
					iter.b() = nHistValue * Colors[nColorID][2];
				}
				
				//app::console() << "PLABEL with Colour: " << label % nColors << endl;
			}
			else {
				nValue = *pDepth;
				//app::console() << "nValue: " << nValue << endl;
				if (nValue != 0)
				{
					nHistValue = gCinderOpenNISkeleton->pDepthHist[nValue];
					
					//app::console() << "nHistValue: " << nHistValue << endl;
					
					iter.r() = nHistValue; 
					iter.g() = nHistValue;
					iter.b() = nHistValue;
				}
			}
			
			pDepth++;
			pLabels++;
			
		}
	}

	gl::clear( Color( 0, 0, 0 ) );
	gl::draw(gl::Texture(gCinderOpenNISkeleton->mDepthSurface));
}



/* Callbacks - Probably un-classable */

void XN_CALLBACK_TYPE CinderOpenNISkeleton::User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	app::console() <<  "New User " << nId << endl;
	// New user found
	if (gCinderOpenNISkeleton->mNeedPose)
	{
		gCinderOpenNISkeleton->mUserGenerator.GetPoseDetectionCap().StartPoseDetection(gCinderOpenNISkeleton->mStrPose, nId);
	}
	else
	{
		gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
	}
}

// Callback: An existing user was lost
void XN_CALLBACK_TYPE CinderOpenNISkeleton::User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	app::console() <<  "Lost user" << nId;
}

// Callback: Detected a pose
void XN_CALLBACK_TYPE CinderOpenNISkeleton::UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	app::console() <<  "Pose " << gCinderOpenNISkeleton->mStrPose << " detected for user " << nId <<endl;
	gCinderOpenNISkeleton->mUserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
	gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

// Callback: Started calibration
void XN_CALLBACK_TYPE CinderOpenNISkeleton::UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
	app::console() << "Calibration started for user " << nId << endl;

}

// Callback: Finished calibration
void XN_CALLBACK_TYPE CinderOpenNISkeleton::UserCalibration_CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie)
{
	if (bSuccess)
	{
		// Calibration succeeded
		
		app::console() << "Calibration complete, start tracking user " <<  nId <<endl;
		gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().StartTracking(nId);
	}
	else
	{
		// Calibration failed
		app::console() << "Calibration failed for user " << nId << endl;
		if (gCinderOpenNISkeleton->mNeedPose){
			gCinderOpenNISkeleton->mUserGenerator.GetPoseDetectionCap().StartPoseDetection(gCinderOpenNISkeleton->mStrPose, nId);
		}
		else
		{
			gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
		}
	}
}

CinderOpenNISkeleton::CinderOpenNISkeleton() {
	mNeedPose = FALSE;
	//gCinderOpenNISkeleton->mStrPose[20];
	pDepthTexBuf = NULL;
	bInitialized = false;
	mJointConfidence = 0.5;
	mNumUsers = 15;
}

void CinderOpenNISkeleton::shutDown() {
	gCinderOpenNISkeleton->mContext.Shutdown();
}

void CinderOpenNISkeleton::update() {	

	gCinderOpenNISkeleton->mContext.WaitAndUpdateAll();
	gCinderOpenNISkeleton->mDepthGenerator.GetMetaData(gCinderOpenNISkeleton->mDepthMD);
	gCinderOpenNISkeleton->mUserGenerator.GetUserPixels(0, gCinderOpenNISkeleton->mSceneMD);
	
	setDepthSurface();
}


bool CinderOpenNISkeleton::setup(string path) {

	XnStatus nRetVal = XN_STATUS_OK;
	xn::EnumerationErrors errors;
	nRetVal = gCinderOpenNISkeleton->mContext.InitFromXmlFile(path.c_str(), &errors);

	if (nRetVal == XN_STATUS_NO_NODE_PRESENT) {
		XnChar strError[1024];
		errors.ToString(strError, 1024);
		app::console() << strError << endl;
		return false;
	}
	else if (nRetVal != XN_STATUS_OK)
	{
		app::console() << "Open failed: " << xnGetStatusString(nRetVal) << endl;
		return false;
	}

	
	nRetVal = gCinderOpenNISkeleton->mContext.FindExistingNode(XN_NODE_TYPE_DEPTH, gCinderOpenNISkeleton->mDepthGenerator);
	CHECK_RC(nRetVal, "Find depth generator");
	nRetVal = gCinderOpenNISkeleton->mContext.FindExistingNode(XN_NODE_TYPE_USER, gCinderOpenNISkeleton->mUserGenerator);
	
	if (nRetVal != XN_STATUS_OK) {
		nRetVal = gCinderOpenNISkeleton->mUserGenerator.Create(gCinderOpenNISkeleton->mContext);
		CHECK_RC(nRetVal, "Find user generator");
		return false;
	}
	
	
	XnCallbackHandle hUserCallbacks, hCalibrationCallbacks, hPoseCallbacks;
	
	if (!gCinderOpenNISkeleton->mUserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON)) {
		app::console() << "Supplied user generator doesn't support skeleton\n" << endl;
		return false;
	}
	
	gCinderOpenNISkeleton->mUserGenerator.RegisterUserCallbacks(CinderOpenNISkeleton::User_NewUser, User_LostUser, NULL, hUserCallbacks);
	gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().RegisterCalibrationCallbacks(UserCalibration_CalibrationStart, UserCalibration_CalibrationEnd, NULL, hCalibrationCallbacks);
	
	if (gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().NeedPoseForCalibration()) {
		gCinderOpenNISkeleton->mNeedPose = TRUE;
		if (!gCinderOpenNISkeleton->mUserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
		{
			app::console() << "Pose required, but not supported" << endl;
			return false;
		}
		gCinderOpenNISkeleton->mUserGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks(UserPose_PoseDetected, NULL, NULL, hPoseCallbacks);
		gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().GetCalibrationPose(gCinderOpenNISkeleton->mStrPose);
	}
	
	gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
	
	nRetVal = gCinderOpenNISkeleton->mContext.StartGeneratingAll();
	CHECK_RC(nRetVal, "StartGenerating");
	return true;
}
