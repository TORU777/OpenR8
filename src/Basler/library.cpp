/*
Copyright (c) 2004-2017 Open Robot Club. All rights reserved.
Basler library for R7.
*/


#include <string>
#include <sstream>

#include <opencv2/opencv.hpp>

#include "MvCameraControl.h"

#include "R7.hpp"


using namespace cv;
using namespace std;


#define BASLER_STRING_SIZE (32)
#define MAX_BUF_SIZE    (1920*1080*3)


typedef struct {
	unsigned int cameraSN;

	void* handle;

	
	Mat grabbedImage;

} Basler_t;


#ifdef __cplusplus
extern "C"
{
#endif

/*
int fc2_StrToInt(string s, unsigned int &value) {
	if (s.empty()) {
		return -1;
	}

	int v;
	stringstream ss(s);
	ss >> v;

	if (v < 0) {
		return -2;
	}

	value = v;
	
	return 1;
}
*/

static int Basler_Init(int r7Sn, int functionSn) {
	int res;
	void *variableObject = NULL;
	res = R7_InitVariableObject(r7Sn, functionSn, 1, sizeof(Basler_t));
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_InitVariableObject = %d", res);
		return -1;
	}
	res = R7_GetVariableObject(r7Sn, functionSn, 1, &variableObject);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableObject = %d", res);
		return -2;
	}
	if (variableObject == NULL) {
		R7_Printf(r7Sn, "ERROR! variableObject == NULL");
		return -3;
	}

	Basler_t *baslerPtr = ((Basler_t*)variableObject);

	//initial values
	baslerPtr->cameraSN = 0;
	baslerPtr->grabbedImage = Mat();
	baslerPtr->handle = NULL;
		
	return 1;
}

static int Basler_Open(int r7Sn, int functionSn) {
	int res;
	void *variableObject = NULL;
	res = R7_GetVariableObject(r7Sn, functionSn, 1, &variableObject);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableObject = %d", res);
		return -1;
	}
	if (variableObject == NULL) {
		R7_Printf(r7Sn, "ERROR! variableObject == NULL");
		return -2;
	}

	Basler_t *baslerPtr = ((Basler_t*)variableObject);


	//Enumerate all the corresponding devices of specified transport protocol in the subnet.
	int nRet = -1;
	void* m_handle = NULL;
	unsigned int nTLayerType = MV_GIGE_DEVICE | MV_USB_DEVICE;
	MV_CC_DEVICE_INFO_LIST m_stDevList = { 0 };
	nRet = MV_CC_EnumDevices(nTLayerType, &m_stDevList);
	if (MV_OK != nRet)
	{
		R7_Printf(r7Sn, "error: EnumDevices fail [%x]\n", nRet);
		return -1;
	}

	int i = 0;
	if (m_stDevList.nDeviceNum == 0)
	{
		R7_Printf(r7Sn, "no camera found!\n");
		return -1;
	}

	//Selecte device
	int nDeviceIndex = 0;
	res = R7_GetVariableInt(r7Sn, functionSn, 2, &nDeviceIndex);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableInt = %d", res);
		return -3;
	}

	MV_CC_DEVICE_INFO m_stDevInfo = { 0 };
	memcpy(&m_stDevInfo, m_stDevList.pDeviceInfo[nDeviceIndex], sizeof(MV_CC_DEVICE_INFO));

	nRet = MV_CC_CreateHandle(&m_handle, &m_stDevInfo);

	if (MV_OK != nRet)
	{
		R7_Printf(r7Sn, "error: CreateHandle fail [%x]\n", nRet);
		return -1;
	}

	//Connect to device
	unsigned int nAccessMode = MV_ACCESS_Exclusive;
	unsigned short nSwitchoverKey = 0;
	nRet = MV_CC_OpenDevice(m_handle, nAccessMode, nSwitchoverKey);
	if (MV_OK != nRet)
	{
		R7_Printf(r7Sn, "error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	//Start image acquisition
	nRet = MV_CC_StartGrabbing(m_handle);
	if (MV_OK != nRet)
	{
		R7_Printf(r7Sn, "error: StartGrabbing fail [%x]\n", nRet);
		return -1;
	}

	// Set values
	baslerPtr->handle = m_handle;
	baslerPtr->cameraSN = nDeviceIndex;

	return 1;
}

static int Basler_Grab(int r7Sn, int functionSn) {
	int res;
	void *variableObject = NULL;
	res = R7_GetVariableObject(r7Sn, functionSn, 1, &variableObject);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableObject = %d", res);
		return -1;
	}
	if (variableObject == NULL) {
		R7_Printf(r7Sn, "ERROR! variableObject == NULL");
		return -2;
	}

	Basler_t *baslerPtr = ((Basler_t*)variableObject);

	int nRet = -1;
	void* m_handle = NULL;

	// Get values
	m_handle = baslerPtr->handle;

	//Get the size of one frame data
	MVCC_INTVALUE stIntvalue = { 0 };
	nRet = MV_CC_GetIntValue(m_handle, "PayloadSize", &stIntvalue);
	if (nRet != MV_OK)
	{
		R7_Printf(r7Sn, "Get PayloadSize failed! nRet [%x]\n", nRet);
		return -1;
	}
	int nBufSize = stIntvalue.nCurValue + 2048; //One frame data size + reserved bytes (handled in SDK)

	//unsigned int    nTestFrameSize = 0;
	unsigned char*  pFrameBuf = NULL;
	pFrameBuf = (unsigned char*)malloc(nBufSize);


	MV_FRAME_OUT_INFO_EX stInfo;
	memset(&stInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));

	nRet = MV_CC_GetOneFrameTimeout(m_handle, pFrameBuf, nBufSize, &stInfo, 1000);
	if (MV_OK != nRet)
	{
		R7_Printf(r7Sn, "error: No data [%x]\n", nRet);
		return -1;
	}

	//Picture data input and output parameters
	MV_SAVE_IMAGE_PARAM stParam;

	//Source data                 
	stParam.pData = pFrameBuf;              //Original image data 
	stParam.nDataLen = stInfo.nFrameLen;    //Original image data length
	stParam.enPixelType = stInfo.enPixelType;  //Original image data pixel format
	stParam.nWidth = stInfo.nWidth;       //Image width
	stParam.nHeight = stInfo.nHeight;      //Image height

										   //Target data
	stParam.enImageType = MV_Image_Jpeg;            //Image type to be saved, converting to JPEG format
	stParam.nBufferSize = MAX_BUF_SIZE;             //Storage node size
	unsigned char* pImage = (unsigned char*)malloc(MAX_BUF_SIZE);
	stParam.pImageBuffer = pImage;                   //Output data buffer, saving converted picture data

	nRet = MV_CC_SaveImage(&stParam);
	if (MV_OK != nRet)
	{
		R7_Printf(r7Sn, "error: MV_CC_SaveImage failed [%x]\n", nRet);
		return -1;
	}

	//R7_Printf(r7Sn, "stInfo.nWidth = %d\n", stInfo.nWidth);
	//R7_Printf(r7Sn, "stInfo.nHeight = %d\n", stInfo.nHeight);

	// image size: 1280 x 1024
	Mat imgbuf( Size(stInfo.nWidth, stInfo.nHeight), CV_8UC3, pImage);
	Mat img = imdecode(imgbuf, CV_LOAD_IMAGE_COLOR);

	baslerPtr->grabbedImage = img.clone();

	if (img.data == NULL)
	{
		R7_Printf(r7Sn, "error: imdecode failed");
		return -1;
	}

	//imshow("Basler", img);

	res = R7_SetVariableMat(r7Sn, functionSn, 2, baslerPtr->grabbedImage);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_SetVariableMat = %d", res);
		return -5;
	}

	return 1;
}

static int Basler_Release(int r7Sn, int functionSn) {
	int res;
	void *variableObject = NULL;
	res = R7_GetVariableObject(r7Sn, functionSn, 1, &variableObject);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableObject = %d", res);
		return -1;
	}
	if (variableObject == NULL) {
		R7_Printf(r7Sn, "ERROR! variableObject == NULL");
		return -2;
	}

	Basler_t *baslerPtr = ((Basler_t*)variableObject);

	int nRet = -1;
	void* m_handle = NULL;

	// Get values
	m_handle = baslerPtr->handle;

	//Stop image acquisition
	nRet = MV_CC_StopGrabbing(m_handle);
	if (MV_OK != nRet)
	{
		R7_Printf(r7Sn, "error: StopGrabbing fail [%x]\n", nRet);
		return -1;
	}

	//Close device, release resource.
	nRet = MV_CC_CloseDevice(m_handle);
	if (MV_OK != nRet)
	{
		R7_Printf(r7Sn, "error: CloseDevice fail [%x]\n", nRet);
		return -1;
	}

	//Destroy handle, release resource
	nRet = MV_CC_DestroyHandle(m_handle);
	if (MV_OK != nRet)
	{
		R7_Printf(r7Sn, "error: DestroyHandle fail [%x]\n", nRet);
		return -1;
	}

	res = R7_ReleaseVariableObject(r7Sn, functionSn, 1);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_ReleaseVariableObject = %d", res);
		return -5;
	}

	return 1;
}

/*
static int FC2_Retrieve(int r7Sn, int functionSn) {
	int res;
	void *variableObject = NULL;
	res = R7_GetVariableObject(r7Sn, functionSn, 1, &variableObject);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableObject = %d", res);
		return -1;
	}
	if (variableObject == NULL) {
		R7_Printf(r7Sn, "ERROR! variableObject == NULL");
		return -2;
	}

	FC2_t *fc2Ptr = ((FC2_t*)variableObject);

	if (fc2Ptr->context == NULL) {
		R7_Printf(r7Sn, "ERROR! Camera is not opened");
		return -3;
	}

	if (fc2Ptr->grabbedImage.empty()) {
		R7_Printf(r7Sn, "ERROR! Did not grab image");
		return -4;
	}
	
	res = R7_SetVariableMat(r7Sn, functionSn, 2, fc2Ptr->grabbedImage);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_SetVariableMat = %d", res);
		return -5;
	}

	return 1;
}
*/

static int Basler_SetGain(int r7Sn, int functionSn) {
	int res;
	void *variableObject = NULL;
	res = R7_GetVariableObject(r7Sn, functionSn, 1, &variableObject);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableObject = %d", res);
		return -1;
	}
	if (variableObject == NULL) {
		R7_Printf(r7Sn, "ERROR! variableObject == NULL");
		return -2;
	}

	Basler_t *baslerPtr = ((Basler_t*)variableObject);

	int nRet = -1;
	void* m_handle = NULL;

	// Get values
	m_handle = baslerPtr->handle;

	//Get device gain
	MVCC_FLOATVALUE struFloatValue = { 0 };

	nRet = MV_CC_GetGain(m_handle, &struFloatValue);
	if (MV_OK != nRet)
	{
		R7_Printf(r7Sn, "error: GetGainValue fail [%x]\n", nRet);
		return -1;
	}

	//Set device gain

	//R7_Printf(r7Sn, "struFloatValue.fMax = %.3f\n", struFloatValue.fMax);
	//R7_Printf(r7Sn, "struFloatValue.fMin = %.3f\n", struFloatValue.fMin);
	//R7_Printf(r7Sn, "struFloatValue.fCurValue = %.3f\n", struFloatValue.fCurValue);

	//float fValue = struFloatValue.fMax;    //Set as max. gain value

	
	float gain = 1.0;
	res = R7_GetVariableFloat(r7Sn, functionSn, 2, &gain);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableFloat = %d", res);
		return -3;
	}
	if (gain < 0) {
		R7_Printf(r7Sn, "ERROR! Gain value should be zero or positive");
		return -4;
	}

	if (gain > struFloatValue.fMax)
		gain = struFloatValue.fMax;
	else if (gain < struFloatValue.fMin)
		gain = struFloatValue.fMin;
	
	nRet = MV_CC_SetGain(m_handle, gain);
	if (MV_OK != nRet)
	{
	R7_Printf(r7Sn, "error: SetGain fail [%x]\n", nRet);
	return -1;
	}
	
	R7_Printf(r7Sn, "Current gain = %.3f\n", gain);
	
	return 1;
}


static int Basler_SetExposureTime(int r7Sn, int functionSn) {
	int res;
	void *variableObject = NULL;
	res = R7_GetVariableObject(r7Sn, functionSn, 1, &variableObject);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableObject = %d", res);
		return -1;
	}
	if (variableObject == NULL) {
		R7_Printf(r7Sn, "ERROR! variableObject == NULL");
		return -2;
	}

	Basler_t *baslerPtr = ((Basler_t*)variableObject);

	int nRet = -1;
	void* m_handle = NULL;

	// Get values
	m_handle = baslerPtr->handle;

	float exposure = 0;
	res = R7_GetVariableFloat(r7Sn, functionSn, 2, &exposure);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableFloat = %d", res);
		return -3;
	}
	if (exposure <= 0) {
		R7_Printf(r7Sn, "ERROR! Exposure time value should be positive");
		return -4;
	}
	
	//Get device exposure
	MVCC_FLOATVALUE struFloatValue = { 0 };

	nRet = MV_CC_GetExposureTime(m_handle, &struFloatValue);
	if (MV_OK != nRet)
	{
		R7_Printf(r7Sn, "error: GetGainValue fail [%x]\n", nRet);
		return -1;
	}

	//R7_Printf(r7Sn, "Current exposure time = %.3f\n", struFloatValue.fCurValue);

	//Set device exposure
	//float fValue = struFloatValue.fMax;    //Set as max. gain value
	if (exposure > struFloatValue.fMax)
		exposure = struFloatValue.fMax;
	else if (exposure < struFloatValue.fMin)
		exposure = struFloatValue.fMin;

	nRet = MV_CC_SetExposureTime(m_handle, exposure);
	if (MV_OK != nRet)
	{
		R7_Printf(r7Sn, "error: SetExposureTime fail [%x]\n", nRet);
		return -1;
	}

	R7_Printf(r7Sn, "Current exposure time = %.3f\n", exposure);

	return 1;
}


R7_API int R7Library_Init(void) {
	// Register your functions in this API.

	R7_RegisterFunction("Basler_Grab", (R7Function_t)&Basler_Grab);
	R7_RegisterFunction("Basler_Init", (R7Function_t)&Basler_Init);
	R7_RegisterFunction("Basler_Open", (R7Function_t)&Basler_Open);
	R7_RegisterFunction("Basler_Release", (R7Function_t)&Basler_Release);
	//R7_RegisterFunction("Basler_Retrieve", (R7Function_t)&Basler_Retrieve);
	R7_RegisterFunction("Basler_SetGain", (R7Function_t)&Basler_SetGain);
	R7_RegisterFunction("Basler_SetExposureTime", (R7Function_t)&Basler_SetExposureTime);
		
	return 1;
}

R7_API int R7Library_Close(void) {
	// If you have something to do before close R7(ex: free mesmory), you should handle them in this API.
	return 1;
}

inline void r7_AppendVariable(json_t *variableArray, const char *name, const char *type, const char *option) {
	if (!variableArray) {
		return;
	}

	json_t *variable;
	json_t *variableObject;

	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string(name));
	json_object_set_new(variableObject, "type", json_string(type));
	json_object_set_new(variableObject, "option", json_string(option));
	json_array_append(variableArray, variable);

	return;
}

R7_API int R7Library_GetSupportList(char *str, int strSize) {
	// Define your functions and parameters in this API.

	json_t *root = json_object();
	json_t *functionGroupArray;
	json_t *functionGroup;
	json_t *functionGroupObject;
	json_t *functionArray;
	json_t *function;
	json_t *functionObject;
	json_t *variableArray;

	functionGroupArray = json_array();
	json_object_set_new(root, "functionGroups", functionGroupArray);

	functionGroup = json_object();
	functionGroupObject = json_object();
	json_object_set_new(functionGroup, "functionGroup", functionGroupObject);
	json_object_set_new(functionGroupObject, "name", json_string("Basler"));
	json_array_append(functionGroupArray, functionGroup);

	functionArray = json_array();
	json_object_set_new(functionGroupObject, "functions", functionArray);


	// Basler_Init
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("Basler_Init"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);
	r7_AppendVariable(variableArray, "baslerObject", "object", "INOUT");

	// Basler_Open
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("Basler_Open"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);
	r7_AppendVariable(variableArray, "baslerObject", "object", "IN");
	r7_AppendVariable(variableArray, "cameraNum", "int", "IN");

	// Basler_Grab
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("Basler_Grab"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);
	r7_AppendVariable(variableArray, "baslerObject", "object", "IN");
	r7_AppendVariable(variableArray, "grabbedImage", "image", "OUT");

	// Basler_Release
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("Basler_Release"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);
	r7_AppendVariable(variableArray, "baslerObject", "object", "IN");

	/*
	// Basler_Retrieve
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("Basler_Retrieve"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);
	r7_AppendVariable(variableArray, "baslerObject", "object", "IN");
	r7_AppendVariable(variableArray, "GrabbedImage", "image", "OUT");
	*/

	// Basler_SetGain
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("Basler_SetGain"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);
	r7_AppendVariable(variableArray, "baslerObject", "object", "IN");
	r7_AppendVariable(variableArray, "gain", "float", "IN");

	
	// Basler_SetExposureTime
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("Basler_SetExposureTime"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);
	r7_AppendVariable(variableArray, "baslerObject", "object", "IN");
	r7_AppendVariable(variableArray, "exposureTime", "float", "IN");
	

	sprintf_s(str, strSize, "%s", json_dumps(root, 0));

	json_decref(root);

	return 1;
}

#ifdef __cplusplus
}
#endif
