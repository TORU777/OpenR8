/*
Copyright (c) 2004-2018 Open Robot Club. All rights reserved.
Halcon library for R7.
*/

#include <string>
#include <sstream>

#include "R7.hpp"
#include <opencv2/opencv.hpp>

#include "HalconCpp.h"
#pragma comment(lib,"halconcpp.lib")


using namespace cv;
using namespace std;
using namespace HalconCpp;


typedef struct {
	Mat cv_ImageIn;
	Mat cv_ImageOut;
	HObject ho_ImageIn;
	HObject ho_ImageResize;

} Halcon_t;


#ifdef __cplusplus
extern "C"
{
#endif


// reference: https://blog.csdn.net/lingtianyulong/article/details/54709130

cv::Mat HImageToIplImage(HObject &Hobj)
{
	//get_grayval(Image : : Row, Column : Grayval)  

	cv::Mat pImage;
	HTuple htChannels;
	HTuple     width, height;
	width = height = 0;

	ConvertImageType(Hobj, &Hobj, "byte");
	CountChannels(Hobj, &htChannels);
	HTuple cType;
	HTuple grayVal;

	if (htChannels.I() == 1)
	{
		GetImageSize(Hobj, &width, &height);

		pImage = cv::Mat(height, width, CV_8UC1);
		pImage = Mat::zeros(height, width, CV_8UC1);

		for (int i = 0; i < height.I(); ++i)
		{
			for (int j = 0; j < width.I(); ++j)
			{
				GetGrayval(Hobj, i, j, &grayVal);
				pImage.at<uchar>(i, j) = (uchar)grayVal.I();
			}

		}

	}
	else if (htChannels.I() == 3)
	{
		GetImageSize(Hobj, &width, &height);
		pImage = cv::Mat(height, width, CV_8UC3);
		for (int row = 0; row < height.I(); row++)
		{
			for (int col = 0; col < width.I(); col++)
			{
				GetGrayval(Hobj, row, col, &grayVal);

				pImage.at<uchar>(row, col * 3) = (uchar)grayVal[2].I();
				pImage.at<uchar>(row, col * 3 + 1) = (uchar)grayVal[1].I();
				pImage.at<uchar>(row, col * 3 + 2) = (uchar)grayVal[0].I();

			}
		}

	}

	return pImage;
}

HObject IplImageToHImage(cv::Mat& pImage)
{
	HObject Hobj;
	
	if (3 == pImage.channels())
	{
		cv::Mat pImageRed, pImageGreen, pImageBlue;
		std::vector<cv::Mat> sbgr(3);
		cv::split(pImage, sbgr);

		int length = pImage.rows * pImage.cols;
		uchar *dataBlue = new uchar[length];
		uchar *dataGreen = new uchar[length];
		uchar *dataRed = new uchar[length];

		int height = pImage.rows;
		int width = pImage.cols;
		for (int row = 0; row < height; row++)
		{
			uchar* ptr = pImage.ptr<uchar>(row);
			for (int col = 0; col < width; col++)
			{
				dataBlue[row * width + col] = ptr[3 * col];
				dataGreen[row * width + col] = ptr[3 * col + 1];
				dataRed[row * width + col] = ptr[3 * col + 2];
			}
		}

		GenImage3(&Hobj, "byte", width, height, (Hlong)(dataRed), (Hlong)(dataGreen), (Hlong)(dataBlue));
		delete[] dataRed;
		delete[] dataGreen;
		delete[] dataBlue;
	}
	else if (1 == pImage.channels())
	{
		int height = pImage.rows;
		int width = pImage.cols;
		uchar *dataGray = new uchar[width * height];
		memcpy(dataGray, pImage.data, width * height);
		GenImage1(&Hobj, "byte", width, height, (Hlong)(dataGray));
		delete[] dataGray;
	}

	return Hobj;
}



static int Halcon_Init(int r7Sn, int functionSn) {
	
	int res;
	void *variableObject = NULL;
	res = R7_InitVariableObject(r7Sn, functionSn, 1, sizeof(Halcon_t));
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

	Halcon_t *halconPtr = ((Halcon_t*)variableObject);
	
	//initial values
	halconPtr->cv_ImageIn = Mat();
	halconPtr->cv_ImageOut = Mat();
	halconPtr->ho_ImageIn = HObject();
	halconPtr->ho_ImageResize = HObject();
	
	return 1;
}

static int Halcon_Resize(int r7Sn, int functionSn) {
	
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

	Halcon_t *halconPtr = ((Halcon_t*)variableObject);

	if (halconPtr == NULL)
	{
		printf("Error! halconPtr == NULL\n");
		return -3;
	}

	//cv::imshow("test", halconPtr->cv_ImageIn);
	//cv::waitKey(0);

	res = R7_GetVariableMat(r7Sn, functionSn, 2, &(halconPtr->cv_ImageIn));
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_SetVariableMat = %d", res);
		return -5;
	}
	//cv::imshow("test2", halconPtr->cv_ImageIn);
	//cv::waitKey(0);
	
	//Mat cv_image(100, 100, CV_8U, Scalar(100));
	//HImage ho_Image;

	halconPtr->ho_ImageIn = IplImageToHImage(halconPtr->cv_ImageIn);

	//ho_Image = IplImageToHImage(cv_image);
	//IplImageToHImage(cv_image);
	//halconPtr->ho_ImageIn = IplImageToHImage(cv_image);

	int width = 0, height = 0;

	res = R7_GetVariableInt(r7Sn, functionSn, 3, &width);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_SetVariableDouble = %d", res);
		return -5;
	}

	res = R7_GetVariableInt(r7Sn, functionSn, 4, &height);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_SetVariableDouble = %d", res);
		return -5;
	}

	if (width <= 0 || height <= 0)
	{
		R7_Printf(r7Sn, "ERROR! Width and height should be positive values.\n");
		return -3;
	}
	
	ZoomImageSize(halconPtr->ho_ImageIn, &(halconPtr->ho_ImageResize), width, height, "constant");

	halconPtr->cv_ImageOut = HImageToIplImage(halconPtr->ho_ImageResize);

	res = R7_SetVariableMat(r7Sn, functionSn, 5, halconPtr->cv_ImageOut);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_SetVariableMat = %d", res);
		return -5;
	}
	
	return 1;
}



R7_API int R7Library_Init(void) {
	// Register your functions in this API.

	R7_RegisterFunction("Halcon_Init", (R7Function_t)&Halcon_Init);
	R7_RegisterFunction("Halcon_Resize", (R7Function_t)&Halcon_Resize);
	
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
	printf("Holcon: R7Library_GetSupportList \n");
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
	json_object_set_new(functionGroupObject, "name", json_string("Halcon"));
	json_array_append(functionGroupArray, functionGroup);

	functionArray = json_array();
	json_object_set_new(functionGroupObject, "functions", functionArray);


	// Halcon_Init
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("Halcon_Init"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);
	r7_AppendVariable(variableArray, "halconObject", "object", "INOUT");

	// Halcon_Resize
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("Halcon_Resize"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);
	r7_AppendVariable(variableArray, "halconObject", "object", "IN");
	r7_AppendVariable(variableArray, "image", "image", "IN");
	r7_AppendVariable(variableArray, "width", "int", "IN");
	r7_AppendVariable(variableArray, "height", "int", "IN");
	r7_AppendVariable(variableArray, "imageResize", "image", "OUT");

	/*
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
	*/

	sprintf_s(str, strSize, "%s", json_dumps(root, 0));

	printf("Holcon: str =  %s \n", str);
	printf("Holcon: json_dumps =  %s \n", json_dumps(root, 0));


	json_decref(root);

	return 1;
}

#ifdef __cplusplus
}
#endif
