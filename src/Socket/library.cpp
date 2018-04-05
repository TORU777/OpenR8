/*
Copyright (c) 2004-2018 Open Robot Club. All rights reserved.
Socket library for R7.
*/

//WinSock2.h and WS2tcpip.h is include before windows.h
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "R7.hpp"

#pragma comment(lib,"ws2_32")



typedef struct {
	//WSADATA wsaData;

	SOCKET ListenSocket;
	SOCKET ClientSocket;

	struct addrinfo *resultAddr = NULL;
	struct addrinfo hints;
	//void *variableObject;
}SOCKET_VARIABLE_t;

//static SOCKET_VARIABLE_t socketVariables;

#ifdef __cplusplus
extern "C"
{
#endif


	static int Socket_Init(int r7Sn, int functionSn) {
		//return -1 : R7_GetVariableObject fail
		//return 1: success

		// output socket
		int result = 1;

		void *variableObject = NULL;
		result = R7_InitVariableObject(r7Sn, functionSn, 2, sizeof(SOCKET_VARIABLE_t));
		if (result <= 0) {
			result = -1;
			R7_SetVariableInt(r7Sn, functionSn, 1, result);
			return result;
		}

		int res = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (res <= 0) {
			result = -2;
			R7_SetVariableInt(r7Sn, functionSn, 1, result);
			return result;
		}

		SOCKET_VARIABLE_t *socketVariables = (SOCKET_VARIABLE_t*)variableObject;
		socketVariables->ClientSocket = INVALID_SOCKET;
		socketVariables->ListenSocket = INVALID_SOCKET;
		socketVariables->resultAddr = NULL;
		memset(&socketVariables->hints, 0, sizeof(&socketVariables->hints));

		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, socketVariables);
		return 1;
	}


	static int Socket_Bind(int r7Sn, int functionSn) {
		//return -1 : R7_GetVariableObject fail
		//return -2 : getaddrinfo fail
		//return -3 : socket
		//return -4 : bind failed
		//return 1: success

		// output socket
		int result = 1;

		void *variableObject = NULL;
		int res = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (res <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		SOCKET_VARIABLE_t *socketVariables = ((SOCKET_VARIABLE_t*)variableObject);

		// input TCP
		int IsTcp = 1;
		R7_GetVariableInt(r7Sn, functionSn, 3, &IsTcp);

		int port = 80;
		R7_GetVariableInt(r7Sn, functionSn, 4, &port);

		socketVariables->hints.ai_family = AF_INET;
		socketVariables->hints.ai_socktype = SOCK_STREAM;
		if (IsTcp == 1) {
			socketVariables->hints.ai_protocol = IPPROTO_TCP;
		}
		else {
			socketVariables->hints.ai_protocol = IPPROTO_UDP;
		}

		socketVariables->hints.ai_flags = AI_PASSIVE;


		char portString[20];
		_itoa(port, portString, 10);


		// Resolve the server address and port
		int iResult = getaddrinfo(NULL, portString, &socketVariables->hints, &socketVariables->resultAddr);
		if (iResult != 0) {
			R7_Printf(r7Sn, "getaddrinfo failed with error: %d\n", iResult);
			WSACleanup();
			result = -2;
			R7_SetVariableInt(r7Sn, functionSn, 1, result);
			return -2;
		}


		// Create a SOCKET for connecting to server
		socketVariables->ListenSocket = socket(socketVariables->resultAddr->ai_family, socketVariables->resultAddr->ai_socktype, socketVariables->resultAddr->ai_protocol);
		if (socketVariables->ListenSocket == INVALID_SOCKET) {
			R7_Printf(r7Sn, "socket failed with error: %ld\n", WSAGetLastError());
			freeaddrinfo(socketVariables->resultAddr);
			WSACleanup();
			result = -3;
			R7_SetVariableInt(r7Sn, functionSn, 1, result);
			return  -3;
		}


		sockaddr_in service;

		service.sin_family = AF_INET;
		//service.sin_addr.s_addr = INADDR_ANY;
		inet_pton(AF_INET, "127.0.0.1", &(service.sin_addr));
		service.sin_port = port;


		if (bind(socketVariables->ListenSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
			R7_Printf(r7Sn, "bind failed");
			closesocket(socketVariables->ListenSocket);
			result = -4;
			R7_SetVariableInt(r7Sn, functionSn, 1, result);
			return -4;
		}

		freeaddrinfo(socketVariables->resultAddr);

		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, socketVariables);
		return 1;
	}


	static int Socket_Listen(int r7Sn, int functionSn) {
		//return -1 : R7_GetVariableObject fail
		//return -2 : listen fail
		//return -3 : accept fail
		//return 1 : seccess

		// output socket
		int result = 1;

		void *variableObject = NULL;
		int res = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (res <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		SOCKET_VARIABLE_t *socketVariables = ((SOCKET_VARIABLE_t*)variableObject);

		// input TCP
		int Backlog = 1000;
		R7_GetVariableInt(r7Sn, functionSn, 3, &Backlog);

		//listen mode
		int iResult = listen(socketVariables->ListenSocket, Backlog);
		if (iResult == SOCKET_ERROR) {
			R7_Printf(r7Sn, "listen failed with error: %d\n", WSAGetLastError());
			closesocket(socketVariables->ListenSocket);
			WSACleanup();
			result = -2;
			R7_SetVariableInt(r7Sn, functionSn, 1, result);
			return -2;
		}

		// Accept a client socket
		socketVariables->ClientSocket = accept(socketVariables->ListenSocket, NULL, NULL);
		if (socketVariables->ClientSocket == INVALID_SOCKET) {
			R7_Printf(r7Sn, "accept failed with error: %d\n", WSAGetLastError());
			closesocket(socketVariables->ListenSocket);
			WSACleanup();
			result = -3;
			R7_SetVariableInt(r7Sn, functionSn, 1, result);
			return -3;
		}

		// No longer need server socket
		closesocket(socketVariables->ListenSocket);


		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, socketVariables);
		return 1;
	}

	static int Socket_Read(int r7Sn, int functionSn) {
		//return -1 : R7_GetVariableObject fail
		//return -2 : listen fail
		//return 1 : seccess

		// output socket
		int result = 0;

		void *variableObject = NULL;
		int res = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (res <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		SOCKET_VARIABLE_t *socketVariables = ((SOCKET_VARIABLE_t*)variableObject);

		//input binary
		int binarySize = R7_GetVariableBinarySize(r7Sn, functionSn, 3);

		unsigned char *binary;
		binary = (unsigned char *)malloc(binarySize);
		R7_GetVariableBinary(r7Sn, functionSn, 3, binary, binarySize);

		result = send(socketVariables->ClientSocket, (char*)binary, binarySize, 0);

		if (result <= 0) {
			R7_Printf(r7Sn, "send fail = %d", result);
			R7_SetVariableInt(r7Sn, functionSn, 1, result);
			return -2;
		}

		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, socketVariables);
		free(binary);
		return 1;
	}

	static int Socket_Write(int r7Sn, int functionSn) {
		//return -1 : R7_GetVariableObject fail
		//return -2 : recv fail
		//return 1 : seccess

		int result = R7_STRING_SIZE;

		void *variableObject = NULL;
		int res = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (res <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		SOCKET_VARIABLE_t *socketVariables = ((SOCKET_VARIABLE_t*)variableObject);

		//ouyput binary
		char* dataGet = (char *)malloc(R7_STRING_SIZE);
		char* dataSave = (char *)malloc(R7_STRING_SIZE);

		int iResult = 0;
		int total = 0;
		int count = 0;

		while ((iResult = recv(socketVariables->ClientSocket, dataGet, result, 0)) > 0) {
			//printf("iResult = %d\n", iResult);
			total += iResult;

			char* bufTemp = dataSave;

			dataSave = (char *)malloc(total + iResult);
			if (dataSave == NULL) {
				return 0;
			}

			if (count > 0) {
				memcpy(dataSave, bufTemp, total - iResult);
			}

			memcpy(dataSave + total - iResult, dataGet, iResult);

			free(bufTemp);
			count++;
			if (iResult < result) {
				free(dataGet);
				break;
			}
		}
		if (iResult < 0) {
			R7_Printf(r7Sn, "recv fail");
			R7_SetVariableInt(r7Sn, functionSn, 1, iResult);
			return -2;
		}
		//if (iResult > 0) {
		//	//printf("Bytes : %s\n", binary);
		//	printf("Bytes size : %d\n", (int)sizeof(dataGet));
		//}

		R7_SetVariableInt(r7Sn, functionSn, 1, total);
		R7_SetVariableBinary(r7Sn, functionSn, 3, (unsigned char*)dataSave, total);
		R7_SetVariableObject(r7Sn, functionSn, 2, socketVariables);
		free(dataSave);
		return 1;
	}

	static int Socket_Close(int r7Sn, int functionSn) {
		//return -1 : R7_GetVariableObject fail
		//return -2 : recv fail
		//return 1 : seccess

		// output socket
		int result = 0;

		void *variableObject = NULL;
		int res = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (res <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		SOCKET_VARIABLE_t *socketVariables = ((SOCKET_VARIABLE_t*)variableObject);

		int iResult = shutdown(socketVariables->ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			R7_Printf(r7Sn, "shutdown failed with error = %d", WSAGetLastError());
			result = -2;
			R7_SetVariableInt(r7Sn, functionSn, 1, result);
			closesocket(socketVariables->ClientSocket);
			WSACleanup();
			return -2;
		}

		WSACleanup();
		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, socketVariables);
		return 1;
	}
	//debug client
	static int Socket_Content(int r7Sn, int functionSn) {
		// output socket
		int result = 0;

		void *variableObject = NULL;
		int res = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (res <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		SOCKET_VARIABLE_t *socketVariables = ((SOCKET_VARIABLE_t*)variableObject);

		// Create a socket.
		SOCKET mSocket;
		mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		socketVariables->ClientSocket = mSocket;
		if (mSocket == INVALID_SOCKET) {
			//printf("Error at socket(): %ldn", WSAGetLastError());
			WSACleanup();
			return -10;
		}

		sockaddr_in clientService;

		clientService.sin_family = AF_INET;
		inet_pton(AF_INET, "127.0.0.1", &(clientService.sin_addr));
		//clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
		clientService.sin_port = 888;

		if (connect(mSocket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
			//printf("Failed to connect.n");
			WSACleanup();
			return -1;
		}


		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, socketVariables);
		return 1;
	}


	R7_API int R7Library_Init(void) {
		SetConsoleOutputCP(65001);
		setlocale(LC_ALL, "en_US.UTF-8");

		//Register your functions in this API.
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			//printf("WSAStartup failed with error: %d\n", iResult);
			return -1;
		}
		R7_RegisterFunction("Socket_Content", (R7Function_t)&Socket_Content);


		R7_RegisterFunction("Socket_Init", (R7Function_t)&Socket_Init);
		R7_RegisterFunction("Socket_Bind", (R7Function_t)&Socket_Bind);
		R7_RegisterFunction("Socket_Listen", (R7Function_t)&Socket_Listen);
		R7_RegisterFunction("Socket_Read", (R7Function_t)&Socket_Read);
		R7_RegisterFunction("Socket_Write", (R7Function_t)&Socket_Write);
		R7_RegisterFunction("Socket_Close", (R7Function_t)&Socket_Close);

		return 1;
	}


	R7_API int R7Library_Close(void) {
		// If you have something to do before close R7(ex: free memory), you should handle them in this API.

		WSACleanup();

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
		json_object_set_new(functionGroupObject, "name", json_string("Socket"));
		json_array_append(functionGroupArray, functionGroup);

		functionArray = json_array();
		json_object_set_new(functionGroupObject, "functions", functionArray);

		//Socket_Init
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("Socket_Init"));
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
		json_object_set_new(variableObject, "name", json_string("socket"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);


		//Socket_Bind
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("Socket_Bind"));
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
		json_object_set_new(variableObject, "name", json_string("socket"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("isTcp"));
		json_object_set_new(variableObject, "type", json_string("Int"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("port"));
		json_object_set_new(variableObject, "type", json_string("Int"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);


		//Socket_Listen
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("Socket_Listen"));
		json_object_set_new(functionObject, "doc", json_string(""));
		json_array_append(functionArray, function);
		variableArray = json_array();
		json_object_set_new(functionObject, "variables", variableArray);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("result"));
		json_object_set_new(variableObject, "type", json_string("int"));
		json_object_set_new(variableObject, "direction", json_string("OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("socket"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("backlog"));
		json_object_set_new(variableObject, "type", json_string("Int"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		//Socket_Read
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("Socket_Read"));
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
		json_object_set_new(variableObject, "name", json_string("socket"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("binary"));
		json_object_set_new(variableObject, "type", json_string("Binary"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		//Socket_Write
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("Socket_Write"));
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
		json_object_set_new(variableObject, "name", json_string("socket"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("binary"));
		json_object_set_new(variableObject, "type", json_string("Binary"));
		json_object_set_new(variableObject, "direction", json_string("OUT"));
		json_array_append(variableArray, variable);


		//Socket_Close
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("Socket_Close"));
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
		json_object_set_new(variableObject, "name", json_string("socket"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);

		//Socket_Content
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("Socket_Content"));
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
		json_object_set_new(variableObject, "name", json_string("socket"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);


		sprintf_s(str, strSize, "%s", json_dumps(root, 0));

		json_decref(root);

		return 1;
	}

#ifdef __cplusplus
}
#endif
