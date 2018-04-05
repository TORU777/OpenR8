/*
Copyright (c) 2004-2018 Open Robot Club. All rights reserved.
OpenGL library for R7.
*/


#include "R7.hpp"

#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtCore/QString>
#include <QtGui/QOpenGLWindow>
#include <QtGui/QOpenGLFunctions>

using namespace std;
using namespace cv;


#ifdef __cplusplus
extern "C"
{
#endif

	//QApplication *app = NULL;

	class OpenGLWindow : public QOpenGLWindow
		//, protected QOpenGLFunctions
	{

	public:
		OpenGLWindow();
		Mat image;
		//QImage image;

	protected:
		void initializeGL();
		void paintGL();
	};

	OpenGLWindow::OpenGLWindow()
		: QOpenGLWindow(QOpenGLWindow::NoPartialUpdate)
	{
	}

	void OpenGLWindow::initializeGL()
	{
		printf("initializeGL ++\n");
		//initializeOpenGLFunctions();
		//printContextInformation();
		/*
		// Load image from file
		if (!image.load("image.png"))
		{
		//qDebug() << "load image fail";
		return;
		}
		*/
		printf("initializeGL --\n");
	}

	void OpenGLWindow::paintGL() {
		printf("paintGL ++");

		qWarning("OpenGLWindow::paintGL()");

		glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);

		glPushMatrix();

		glLoadIdentity();

		glEnable(GL_TEXTURE_2D);

		// Create one OpenGL texture
		GLuint textureID;
		glGenTextures(1, &textureID);

		// "Bind" the newly created texture : all future texture functions will modify this texture
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, 0x80E0, GL_UNSIGNED_BYTE, image.ptr());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


		glColor3f(1.0, 1.0, 1.0);

		glBegin(GL_QUADS);

		glTexCoord2f(0.0f, 0.0f); glVertex2f(-1, 1);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1, 1);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1, -1);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(-1, -1);

		glEnd();

		printf("paintGL --");
	}


	typedef struct {
		Mat screenShot;
		OpenGLWindow *openGLWindow = NULL;
		int status;
		int isClosingFrame;
	} OpenGL_t;

	typedef struct {
		int r7Sn = 0;
		int functionSn = 0;
	} OpenGLCallBack_t;

	/*
	static int OpenGL_NewApp(int r7Sn, int functionSn) {

	if (app == NULL) {
	int nullArgc = 0;
	app = new QApplication(nullArgc, NULL);
	}

	image = imread("image.jpg", CV_LOAD_IMAGE_COLOR);

	if (!image.data)
	{
	qWarning("Can't open the image");
	}

	return 1;
	}
	*/


	static int _stdcall OpenGL_NewWindowCallback(void *data) {

		printf("OpenGL_NewWindowCallback ++\n");
		if (true) {
			int r7Sn, functionSn;
			int res;
			void *variableObject = NULL;
			OpenGLCallBack_t *cbPtr = ((OpenGLCallBack_t *)data);
			r7Sn = cbPtr->r7Sn;
			functionSn = cbPtr->functionSn;

			printf("OpenGL_NewWindowCallback get Sn OK\n");

			res = R7_GetVariableObject(r7Sn, functionSn, 1, &variableObject);
			if (res <= 0) {
				R7_Printf(r7Sn, "ERROR! R7_GetVariableObject = %d", res);
				return -2;
			}

			OpenGL_t *openglPtr = ((OpenGL_t*)variableObject);

			//qWarning("Line: %d", __LINE__);
			if (true) {
				openglPtr->openGLWindow = new OpenGLWindow();


				QSurfaceFormat format;
				format.setRenderableType(QSurfaceFormat::OpenGL);
				format.setProfile(QSurfaceFormat::CoreProfile);
				format.setVersion(3, 3);

				openglPtr->openGLWindow->setFormat(format);


				//qWarning("Line: %d", __LINE__);

				//openglPtr->openGLWindow->image = imread("image.jpg", CV_LOAD_IMAGE_COLOR);
				//openglPtr->openGLWindow->image = imread("test.png", CV_LOAD_IMAGE_COLOR);
				openglPtr->openGLWindow->image = cv::Mat(cv::Size(500, 400), CV_8UC3, cv::Scalar(255, 0, 0));
				printf("cols = %d, rows = %d\n", openglPtr->openGLWindow->image.cols, openglPtr->openGLWindow->image.rows);

				openglPtr->openGLWindow->resize(openglPtr->openGLWindow->image.cols, openglPtr->openGLWindow->image.rows);
				printf("openglPtr->openGLWindow->setTitle \n");

				//openglPtr->openGLWindow->setTitle("OpenGL Window");
				printf("openglPtr->openGLWindow->show \n");
				openglPtr->openGLWindow->show();
				printf("openglPtr->openGLWindow->show OK\n");
				//system("PAUSE");
				//openglPtr->openGLWindow->update();
			}
		}
		printf("OpenGL_NewWindowCallback --\n");

		/*
		res = R7_GetVariableObject(r7Sn, functionSn, 1, &variableObject);
		if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableObject = %d", res);
		return -2;
		}

		OpenGL_t *openglPtr = ((OpenGL_t*)variableObject);

		openglPtr->image = Mat();

		openglPtr->screenShot = Mat();

		openglPtr->status = 0;

		openglPtr->isClosingFrame = 0;

		char command[R7_STRING_SIZE];
		command[0] = '\0';

		res = R7_GetVariableString(r7Sn, functionSn, 2, command, R7_STRING_SIZE);
		if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableString = %d", res);
		return -4;
		}

		int strSize = (int)(strlen(command));

		command[strSize] = '\0';

		openglPtr->openGLFrame = new OpenGLFrame(openglPtr, wxString::FromUTF8(command));

		openglPtr->openGLFrame->Show();
		*/
		return 1;
	}

	static int OpenGL_NewWindow(int r7Sn, int functionSn) {

		//qWarning("OpenGL_NewWindow");



		//openglPtr->app->exec();

		//openglPtr->openGLWindow.show();
		printf("OpenGL_NewWindow ++\n");

		OpenGLCallBack_t *cbPtr = (OpenGLCallBack_t*)malloc(sizeof(OpenGLCallBack_t));

		cbPtr->r7Sn = r7Sn;

		cbPtr->functionSn = functionSn;

		int res;
		void *variableObject = NULL;
		res = R7_InitVariableObject(r7Sn, functionSn, 1, sizeof(OpenGL_t));
		if (res <= 0) {
			R7_Printf(r7Sn, "ERROR! R7_InitVariableObject = %d", res);
			return -1;
		}

		res = R7_GetVariableObject(r7Sn, functionSn, 1, &variableObject);

		//OpenGL_t *openglPtr = ((OpenGL_t*)variableObject);
		//openglPtr->openGLWindow = new OpenGLWindow();

		//printf("OpenGL_NewWindow R7_ProcessQtPendingEvents ++\n");
		//R7_ProcessQtPendingEvents();
		//printf("OpenGL_NewWindow R7_ProcessQtPendingEvents --\n");
		
		R7_QueueQtEvent((R7CallbackHandler)OpenGL_NewWindowCallback, (void*)cbPtr);
		printf("OpenGL_NewWindow R7_QueueQtEvent OK\n");
		//printf("OpenGL_NewWindow pre R7_ProcessQtPendingEvents\n");
		//R7_ProcessQtPendingEvents();
		//printf("OpenGL_NewWindow R7_ProcessQtPendingEvents OK\n");
		printf("OpenGL_NewWindow --\n");
		return 1;
	}

	static int OpenGL_ShowWindow(int r7Sn, int functionSn) {
		//test
		//int argc = 0;
		//QApplication *app = new QApplication(argc, NULL);
		//QApplication *app;
		//app = (QApplication* )R7_GetWxApp();
		//OpenGLWindow *openGLWindow = new OpenGLWindow();
		//openGLWindow->resize(500, 500);
		//openGLWindow->show();
		//app->exec();
		return 1;
	}

	static int OpenGL_HideWindow(int r7Sn, int functionSn) {

		return 1;
	}

	static int OpenGL_ShowImage(int r7Sn, int functionSn) {

		return 1;
	}

	static int OpenGL_GetImage(int r7Sn, int functionSn) {

		return 1;
	}

	/*
	static int OpenGL_ExecApp(int r7Sn, int functionSn) {

	qWarning("OpenGL_ExecApp");



	return 1;
	}
	*/


	R7_API int R7Library_Init(void) {
		// Register your functions in this API.
		//OpenGL_NewApp
		//OpenGL_ExecApp
		//R7_RegisterFunction("OpenGL_NewApp", (R7Function_t)&OpenGL_NewApp);
		R7_RegisterFunction("OpenGL_NewWindow", (R7Function_t)&OpenGL_NewWindow);
		R7_RegisterFunction("OpenGL_ShowWindow", (R7Function_t)&OpenGL_ShowWindow);
		R7_RegisterFunction("OpenGL_HideWindow", (R7Function_t)&OpenGL_HideWindow);
		R7_RegisterFunction("OpenGL_ShowImage", (R7Function_t)&OpenGL_ShowImage);
		R7_RegisterFunction("OpenGL_GetImage", (R7Function_t)&OpenGL_GetImage);
		//R7_RegisterFunction("OpenGL_ExecApp", (R7Function_t)&OpenGL_ExecApp);


		// OpenGL_NewApp
		// OpenGL_NewWindow
		// OpenGL_NewWindow
		// OpenGL_ExecApp

		return 1;
	}

	R7_API int R7Library_Close(void) {
		// If you have something to do before close R7(ex: free memory), you should handle them in this API.


		return 1;
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
		json_t *variable;
		json_t *variableObject;

		functionGroupArray = json_array();
		json_object_set_new(root, "functionGroups", functionGroupArray);

		functionGroup = json_object();
		functionGroupObject = json_object();
		json_object_set_new(functionGroup, "functionGroup", functionGroupObject);
		json_object_set_new(functionGroupObject, "name", json_string("OpenGL"));
		json_array_append(functionGroupArray, functionGroup);

		functionArray = json_array();
		json_object_set_new(functionGroupObject, "functions", functionArray);

		// OpenGL_NewApp
		/*
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("OpenGL_NewApp"));
		json_object_set_new(functionObject, "doc", json_string(""));
		json_array_append(functionArray, function);
		variableArray = json_array();
		json_object_set_new(functionObject, "variables", variableArray);
		*/

		// OpenGL_NewWindow
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("OpenGL_NewWindow"));
		json_object_set_new(functionObject, "doc", json_string(""));
		json_array_append(functionArray, function);
		variableArray = json_array();
		json_object_set_new(functionObject, "variables", variableArray);
		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("OpenGLWindow"));
		json_object_set_new(variableObject, "type", json_string("object"));
		json_object_set_new(variableObject, "direction", json_string("OUT"));
		json_array_append(variableArray, variable);
		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("OpenGLWindowTitle"));
		json_object_set_new(variableObject, "type", json_string("string"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		// OpenGL_ShowWindow
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("OpenGL_ShowWindow"));
		json_object_set_new(functionObject, "doc", json_string(""));
		json_array_append(functionArray, function);
		variableArray = json_array();
		json_object_set_new(functionObject, "variables", variableArray);
		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("OpenGLWindow"));
		json_object_set_new(variableObject, "type", json_string("object"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		// OpenGL_HideWindow
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("OpenGL_HideWindow"));
		json_object_set_new(functionObject, "doc", json_string(""));
		json_array_append(functionArray, function);
		variableArray = json_array();
		json_object_set_new(functionObject, "variables", variableArray);
		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("OpenGLWindow"));
		json_object_set_new(variableObject, "type", json_string("object"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		// OpenGL_ShowImage
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("OpenGL_ShowImage"));
		json_object_set_new(functionObject, "doc", json_string(""));
		json_array_append(functionArray, function);
		variableArray = json_array();
		json_object_set_new(functionObject, "variables", variableArray);
		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("OpenGLWindow"));
		json_object_set_new(variableObject, "type", json_string("object"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);
		variable = json_object();
		variableObject = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("Image"));
		json_object_set_new(variableObject, "type", json_string("image"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		// OpenGL_GetImage
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("OpenGL_GetImage"));
		json_object_set_new(functionObject, "doc", json_string(""));
		json_array_append(functionArray, function);
		variableArray = json_array();
		json_object_set_new(functionObject, "variables", variableArray);
		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("OpenGLWindow"));
		json_object_set_new(variableObject, "type", json_string("object"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);
		variable = json_object();
		variableObject = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("Image"));
		json_object_set_new(variableObject, "type", json_string("image"));
		json_object_set_new(variableObject, "direction", json_string("OUT"));
		json_array_append(variableArray, variable);

		// OpenGL_ExecApp
		/*
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("OpenGL_ExecApp"));
		json_object_set_new(functionObject, "doc", json_string(""));
		json_array_append(functionArray, function);
		variableArray = json_array();
		json_object_set_new(functionObject, "variables", variableArray);
		*/

		sprintf_s(str, strSize, "%s", json_dumps(root, 0));

		json_decref(root);

		return 1;
	}

#ifdef __cplusplus
}
#endif
