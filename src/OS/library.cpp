/*
Copyright (c) 2004-2018 Open Robot Club. All rights reserved.
OS library for R7.
*/


#include "R7.hpp"
#include <ShellApi.h>


using namespace std;


#ifdef __cplusplus
extern "C"
{
#endif


static int OS_OpenDir(int r7Sn, int functionSn) {

	int result = 0;
	char fileName[R7_STRING_SIZE];

	R7_GetVariableString(r7Sn, functionSn, 1, fileName, R7_STRING_SIZE);

	if (fileName[0] == '\0') {
		//R7_Printf(r7Sn, "\nERROR! fileName\n");
		result = -1;
		//R7_SetVariableInt(r7Sn, functionSn, 1, result);
		return result;
	}

	ShellExecuteA(NULL, "open", fileName, NULL, NULL, SW_SHOWDEFAULT);

	return 1;
}

static int OS_Program(int r7Sn, int functionSn) {

	int result = 0;
	char programName[R7_STRING_SIZE];
	char parameter[R7_STRING_SIZE];

	R7_GetVariableString(r7Sn, functionSn, 1, programName, R7_STRING_SIZE);
	R7_GetVariableString(r7Sn, functionSn, 2, parameter, R7_STRING_SIZE);

	if (programName[0] == '\0') {
		//R7_Printf(r7Sn, "\nERROR! programName\n");
		result = -1;
		//R7_SetVariableInt(r7Sn, functionSn, 1, result);
		return result;
	}

	if (parameter[0] == '\0') {
		//result = -1;
		//return result;
		*parameter = NULL;
	}

	ShellExecuteA(NULL, "open", programName, parameter, NULL, SW_SHOWNORMAL);

	return 1;
}

static int OS_PrintDoc(int r7Sn, int functionSn) {

	int result = 0;
	char fileName[R7_STRING_SIZE];

	R7_GetVariableString(r7Sn, functionSn, 1, fileName, R7_STRING_SIZE);

	if (fileName[0] == '\0') {
		//R7_Printf(r7Sn, "\nERROR! fileName\n");
		result = -1;
		//R7_SetVariableInt(r7Sn, functionSn, 1, result);
		return result;
	}

	ShellExecuteA(NULL, "print", fileName, NULL, NULL, 0);

	return 1;
}

static int OS_OpenWebpage(int r7Sn, int functionSn) {

	int result = 0;
	char fileName[R7_STRING_SIZE];

	R7_GetVariableString(r7Sn, functionSn, 1, fileName, R7_STRING_SIZE);

	if (fileName[0] == '\0') {
		//R7_Printf(r7Sn, "\nERROR! fileName\n");
		result = -1;
		//R7_SetVariableInt(r7Sn, functionSn, 1, result);
		return result;
	}

	ShellExecuteA(NULL, "open", fileName, NULL, NULL, 0);

	return 1;
}

static int OS_Exit(int r7Sn, int functionSn) {

	int res = 0, errorCode = 0;
	res = R7_GetVariableInt(r7Sn, functionSn, 1, &errorCode);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableInt = %d", res);
		return -3;
	}

	exit(errorCode);

	return 1;
}

R7_API int R7Library_Init(void) {
	//SetConsoleOutputCP(65001);
	//setlocale(LC_ALL, "en_US.UTF-8");

	// Register your functions in this API.

	R7_RegisterFunction("OS_OpenDir", (R7Function_t)&OS_OpenDir);
	R7_RegisterFunction("OS_Program", (R7Function_t)&OS_Program);
	R7_RegisterFunction("OS_PrintDoc", (R7Function_t)&OS_PrintDoc);
	R7_RegisterFunction("OS_OpenWebpage", (R7Function_t)&OS_OpenWebpage);
	R7_RegisterFunction("OS_Exit", (R7Function_t)&OS_Exit);

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
	json_object_set_new(functionGroupObject, "name", json_string("OS"));
	json_array_append(functionGroupArray, functionGroup);

	functionArray = json_array();
	json_object_set_new(functionGroupObject, "functions", functionArray);

	//OS_OpenDir
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("OS_OpenDir"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);

	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string("dirName"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "option", json_string("IN, FOLDER_PATH"));
	json_array_append(variableArray, variable);

	//OS_Program
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("OS_Program"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);

	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string("programName"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "option", json_string("IN, FILE_PATH"));
	json_array_append(variableArray, variable);

	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string("parameter"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "option", json_string("IN, FILE_PATH"));
	json_array_append(variableArray, variable);
	
	//OS_PrintDoc
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("OS_PrintDoc"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);

	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string("filePath"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "option", json_string("IN, FILE_PATH"));
	json_array_append(variableArray, variable);


	//OS_OpenWebpage
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("OS_OpenWebpage"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);

	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string("url"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "option", json_string("IN"));
	json_array_append(variableArray, variable);

	//OS_Exit
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("OS_Exit"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);

	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string("errorCode"));
	json_object_set_new(variableObject, "type", json_string("Int"));
	json_object_set_new(variableObject, "option", json_string("IN"));
	json_array_append(variableArray, variable);


	sprintf_s(str, strSize, "%s", json_dumps(root, 0));

	json_decref(root);

	return 1;
}

#ifdef __cplusplus
}
#endif
