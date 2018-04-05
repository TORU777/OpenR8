/*
Copyright (c) 2004-2018 Open Robot Club. All rights reserved.
CURL library for R7.
*/


#include "R7.hpp"
#include <curl\curl.h>
#define CURL_STATICLIB

using namespace std;

//Ws2_32.lib//wldap32.lib
#ifdef __cplusplus
extern "C"
{
#endif

	//Do Http post
	struct curl_httppost* post = NULL;
	struct curl_httppost* last = NULL;

	struct CurlMemoryStruct {
		char *memory;
		size_t size;
		size_t length;
	};

	struct CurlBinaryStruct {
		char *binaryBuffer;
		char *fileName;
		char *name;
	};

	typedef struct {
		CURL *curl;
		struct CurlMemoryStruct chunk;
		vector <CurlBinaryStruct> binary;

	} CURL_t;


	static size_t curl_WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
		size_t realSize = size * nmemb;
		struct CurlMemoryStruct *mem = (struct CurlMemoryStruct *)userp;
		int newLength = int(mem->length + realSize);// +1;
		if (mem->size < newLength) {
			//mem->memory = (char *)realloc(mem->memory, newLength);
			//if (mem->memory == NULL) {
			//	/* out of memory! */
			//	return 0;
			//}

			char *bufTemp = mem->memory;
			mem->memory = (char *)malloc(newLength);
			if (mem->memory == NULL) {
				return 0;
			}
			memcpy(mem->memory, bufTemp, mem->length);
			free(bufTemp);
			mem->size = newLength;
		}

		memcpy(&(mem->memory[mem->length]), contents, realSize);
		mem->length += realSize;

		return realSize;
	}


	static int CURL_Init(int r7Sn, int functionSn) {
		//CURL_t *curl = (CURL_t *)calloc(1, sizeof(CURL_t));
		//R7_GetVariableObject(r7Sn, functionSn, 2, curl);

		int result = 1;
		void *variableObject = NULL;
		result = R7_InitVariableObject(r7Sn, functionSn, 2, sizeof(CURL_t));
		if (result <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		result = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (result <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		CURL_t *curl = ((CURL_t*)variableObject);


		curl->chunk.memory = (char*)malloc(R7_PAGE_SIZE);// will be grown as needed by realloc above
		if (curl->chunk.memory == NULL) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}
		curl->chunk.size = R7_PAGE_SIZE;
		curl->chunk.length = 0;

		curl->curl = curl_easy_init();
		if (curl == NULL) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, curl);
		return 1;
	}


	static int CURL_Set(int r7Sn, int functionSn) {
		int result = 1;
		void *variableObject = NULL;
		result = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (result <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		CURL_t *curl = ((CURL_t*)variableObject);

		char url[R7_STRING_SIZE];
		R7_GetVariableString(r7Sn, functionSn, 3, url, R7_STRING_SIZE);

		int TimeOut = 10;
		R7_GetVariableInt(r7Sn, functionSn, 4, &TimeOut);

		curl_easy_setopt(curl->curl, CURLOPT_URL, url);
		curl_easy_setopt(curl->curl, CURLOPT_TIMEOUT, TimeOut);
		curl_easy_setopt(curl->curl, CURLOPT_CONNECTTIMEOUT, TimeOut);

		//Send all data to this function.
		curl_easy_setopt(curl->curl, CURLOPT_WRITEFUNCTION, curl_WriteMemoryCallback);

		//We pass our 'chunk' struct to the callback function.
		curl_easy_setopt(curl->curl, CURLOPT_WRITEDATA, (void*)&curl->chunk);

		//Some servers don't like requests that are made without a user-agent field, so we provide one.
		curl_easy_setopt(curl->curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, curl);
		return 1;
	}


	static int CURL_FormAddString(int r7Sn, int functionSn) {
		int result = 1;
		void *variableObject = NULL;
		result = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (result <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		CURL_t *curl = ((CURL_t*)variableObject);

		int nameSize = R7_GetVariableStringSize(r7Sn, functionSn, 3);//4 byte  -> nameSize = 5
		char *name = (char*)malloc(nameSize);
		R7_GetVariableString(r7Sn, functionSn, 3, name, nameSize);

		int strSize = R7_GetVariableStringSize(r7Sn, functionSn, 4);
		char *str = (char*)malloc(strSize);
		R7_GetVariableString(r7Sn, functionSn, 4, str, strSize);

		curl_formadd(&post, &last, CURLFORM_COPYNAME, name,
			CURLFORM_COPYCONTENTS, str,
			CURLFORM_END);

		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, curl);
		return 1;
	}

	static int CURL_FormAddBinary(int r7Sn, int functionSn) {
		int result = 1;
		void *variableObject = NULL;
		result = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (result <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		CURL_t *curl = ((CURL_t*)variableObject);

		int nameSize = R7_GetVariableStringSize(r7Sn, functionSn, 3);
		char *name = (char*)malloc(nameSize);
		R7_GetVariableString(r7Sn, functionSn, 3, name, nameSize);

		int fileNameSize = R7_GetVariableStringSize(r7Sn, functionSn, 4);
		char *fileName = (char*)malloc(fileNameSize);
		R7_GetVariableString(r7Sn, functionSn, 4, fileName, fileNameSize);

		int binarySize = R7_GetVariableBinarySize(r7Sn, functionSn, 5);
		void *binary = (void*)malloc(binarySize);
		R7_GetVariableBinary(r7Sn, functionSn, 5, binary, binarySize);

		CurlBinaryStruct temp;
		temp.binaryBuffer = (char*)binary;
		temp.fileName = fileName; 
		temp.name = name;
		curl->binary.push_back(temp);
		int ptr = int(curl->binary.size()) - 1;

		curl_formadd(&post, &last,
			CURLFORM_COPYNAME, curl->binary[ptr].name,
			CURLFORM_BUFFER, curl->binary[ptr].fileName,
			CURLFORM_BUFFERPTR, curl->binary[ptr].binaryBuffer,
			CURLFORM_BUFFERLENGTH, binarySize,
			CURLFORM_END);

		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, curl);

		return 1;
	}

	static int CURL_Perform(int r7Sn, int functionSn) {
		int result = 1;
		void *variableObject = NULL;
		result = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (result <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		CURL_t *curl = ((CURL_t*)variableObject);

		//Set the form info.
		curl_easy_setopt(curl->curl, CURLOPT_HTTPPOST, post);

		CURLcode code;
		code = curl_easy_perform(curl->curl);
		if (code != CURLE_OK) {
			//strcpy(curlResult, curl_easy_strerror(code));
			R7_SetVariableInt(r7Sn, functionSn, 1, -19);
			return 1;
		}

		int strSize = R7_GetVariableStringSize(r7Sn, functionSn, 3);
		char *temp = (char *)malloc(strSize);
		if (R7_GetVariableString(r7Sn, functionSn, 3, temp, strSize)) {
			if (curl->chunk.length > 0) {
				char *str = (char *)malloc(curl->chunk.length + 1);
				memcpy(str, curl->chunk.memory, curl->chunk.length);
				str[curl->chunk.length] = '\0';
				R7_SetVariableString(r7Sn, functionSn, 3, str);
				free(str);
			}
			else {
				R7_SetVariableString(r7Sn, functionSn, 3, "");
			}
		}
		free(temp);

		int binarySize = R7_GetVariableBinarySize(r7Sn, functionSn, 4);
		char *binary = (char *)malloc(binarySize);
		if (R7_GetVariableBinary(r7Sn, functionSn, 4, binary, binarySize)) {
			if (int(curl->chunk.length) > 0) {
				R7_SetVariableBinary(r7Sn, functionSn, 4, curl->chunk.memory, int(curl->chunk.length));
			}
			else {
				R7_SetVariableBinary(r7Sn, functionSn, 4, NULL, 0);
			}
		}
		free(binary);

		result = (int)curl->chunk.length;

		// Always cleanup. 
		curl_easy_cleanup(curl->curl);
		curl_formfree(post);

		if (curl->chunk.memory) {
			free(curl->chunk.memory);
		}

		std::vector<CurlBinaryStruct> vec;
		curl->binary.swap(std::vector<CurlBinaryStruct>());
		curl->binary.clear();//size  capacity

		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, curl);
		return 1;
	}

	R7_API int R7Library_Init(void) {

		//CURL		curl_global_init(CURL_GLOBAL_ALL);

		// Register your functions in this API.
		R7_RegisterFunction("CURL_Init", (R7Function_t)&CURL_Init);
		R7_RegisterFunction("CURL_Set", (R7Function_t)&CURL_Set);
		R7_RegisterFunction("CURL_FormAddString", (R7Function_t)&CURL_FormAddString);
		R7_RegisterFunction("CURL_FormAddBinary", (R7Function_t)&CURL_FormAddBinary);
		R7_RegisterFunction("CURL_Perform", (R7Function_t)&CURL_Perform);

		return 1;
	}


	R7_API int R7Library_Close(void) {
		// If you have something to do before close R7(ex: free memory), you should handle them in this API.

		//CURL		curl_global_cleanup();

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
		json_object_set_new(functionGroupObject, "name", json_string("CURL"));
		json_array_append(functionGroupArray, functionGroup);

		functionArray = json_array();
		json_object_set_new(functionGroupObject, "functions", functionArray);

		//CURL_Init
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("CURL_Init"));
		json_object_set_new(functionObject, "doc", json_string(""));
		json_array_append(functionArray, function);
		variableArray = json_array();
		json_object_set_new(functionObject, "variables", variableArray);
		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("result"));
		json_object_set_new(variableObject, "type", json_string("Int"));
		json_object_set_new(variableObject, "direction", json_string("OUT"));
		json_array_append(variableArray, variable);
		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("curl"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);

		//CURL_Set
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("CURL_Set"));
		json_object_set_new(functionObject, "doc", json_string(""));
		json_array_append(functionArray, function);
		variableArray = json_array();
		json_object_set_new(functionObject, "variables", variableArray);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("result"));
		json_object_set_new(variableObject, "type", json_string("Int"));
		json_object_set_new(variableObject, "direction", json_string("OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("curl"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("url"));
		json_object_set_new(variableObject, "type", json_string("String"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("timeOut"));
		json_object_set_new(variableObject, "type", json_string("Int"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		//CURL_FormAddString
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("CURL_FormAddString"));
		json_object_set_new(functionObject, "doc", json_string(""));
		json_array_append(functionArray, function);
		variableArray = json_array();
		json_object_set_new(functionObject, "variables", variableArray);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("result"));
		json_object_set_new(variableObject, "type", json_string("Int"));
		json_object_set_new(variableObject, "direction", json_string("OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("curl"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("name"));
		json_object_set_new(variableObject, "type", json_string("String"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("string"));
		json_object_set_new(variableObject, "type", json_string("String"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		//CURL_FormAddBinary
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("CURL_FormAddBinary"));
		json_object_set_new(functionObject, "doc", json_string(""));
		json_array_append(functionArray, function);
		variableArray = json_array();
		json_object_set_new(functionObject, "variables", variableArray);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("result"));
		json_object_set_new(variableObject, "type", json_string("Int"));
		json_object_set_new(variableObject, "direction", json_string("OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("curl"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN,OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("name"));
		json_object_set_new(variableObject, "type", json_string("String"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("fileName"));
		json_object_set_new(variableObject, "type", json_string("String"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("binary"));
		json_object_set_new(variableObject, "type", json_string("Binary"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);


		//CURL_Perform
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("CURL_Perform"));
		json_object_set_new(functionObject, "doc", json_string(""));
		json_array_append(functionArray, function);
		variableArray = json_array();
		json_object_set_new(functionObject, "variables", variableArray);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("result"));
		json_object_set_new(variableObject, "type", json_string("Int"));
		json_object_set_new(variableObject, "direction", json_string("OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("curl"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("string"));
		json_object_set_new(variableObject, "type", json_string("String"));
		json_object_set_new(variableObject, "direction", json_string("OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("binary"));
		json_object_set_new(variableObject, "type", json_string("Binary"));
		json_object_set_new(variableObject, "direction", json_string("OUT"));
		json_array_append(variableArray, variable);


		sprintf_s(str, strSize, "%s", json_dumps(root, 0));

		json_decref(root);

		return 1;
	}

#ifdef __cplusplus
}


#endif
