/*
Copyright (c) 2004-2018 Open Robot Club. All rights reserved.
Httpd library for R7.
*/


//WinSock2.h and WS2tcpip.h is include before windows.h
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <direct.h>
#include <map>
#include <io.h>
#include "R7.hpp"

#include <QtCore/QString>
#include <QtCore/QUrl>

#pragma comment(lib,"ws2_32")


typedef struct {
	WSADATA wsaData;
	SOCKET socketListen;
	int port;
	char *documentRoot;
	int threadNum;
	int backlogNum;
	int toStop;
	int timeout;
	std::map<QString, QString> *mapHeader;
} HttpdObject_t;


#ifdef __cplusplus
extern "C"
{
#endif


	static int Httpd_Init(int r7Sn, int functionSn) {
		int result = 1;

		void *variableObject = NULL;
		result = R7_InitVariableObject(r7Sn, functionSn, 2, sizeof(HttpdObject_t));
		if (result <= 0) {
			result = -1;
			R7_SetVariableInt(r7Sn, functionSn, 1, result);
			return result;
		}

		result = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (result <= 0) {
			result = -2;
			R7_SetVariableInt(r7Sn, functionSn, 1, result);
			return result;
		}

		HttpdObject_t *httpdObject = (HttpdObject_t*)variableObject;

		httpdObject->socketListen = INVALID_SOCKET;
		httpdObject->port = 80;
		httpdObject->documentRoot = "";
		httpdObject->threadNum = 1024;
		httpdObject->backlogNum = 1024;
		httpdObject->toStop = 0;
		httpdObject->timeout = 3;
		httpdObject->mapHeader = new std::map<QString, QString>();
		
		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, httpdObject);

		return 1;
	}

	static int Httpd_Set(int r7Sn, int functionSn) {
		int result = 1;

		void *variableObject = NULL;
		result = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (result <= 0) {
			result = -1;
			R7_SetVariableInt(r7Sn, functionSn, 1, result);
			return result;
		}

		HttpdObject_t *httpdObject = (HttpdObject_t*)variableObject;

		R7_GetVariableInt(r7Sn, functionSn, 3, &httpdObject->port);

		int stringSize = R7_GetVariableStringSize(r7Sn, functionSn, 4);
		httpdObject->documentRoot = (char *)malloc(stringSize);
		R7_GetVariableString(r7Sn, functionSn, 4, httpdObject->documentRoot, stringSize);

		R7_GetVariableInt(r7Sn, functionSn, 5, &httpdObject->threadNum);
		R7_GetVariableInt(r7Sn, functionSn, 6, &httpdObject->backlogNum);
		R7_GetVariableInt(r7Sn, functionSn, 7, &httpdObject->timeout);
		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, httpdObject);
		free(httpdObject->documentRoot);

		//R7_Printf(r7Sn, "port = %d\n", httpdVariable->port);
		//R7_Printf(r7Sn, "documentRoot = %s\n", httpdVariable->documentRoot);
		//R7_Printf(r7Sn, "threadNum = %d\n", httpdVariable->threadNum);
		//R7_Printf(r7Sn, "backlogNum = %d\n", httpdVariable->backlogNum);

		return 1;
	}

	static int Httpd_Start(int r7Sn, int functionSn) {
		// TODO Test IE, Firefox  

		int res = 1;

		void *variableObject = NULL;
		res = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (res <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, res);
			return -1;
		}

		HttpdObject_t *httpdObject = ((HttpdObject_t*)variableObject);

		/*QString qstrWorkSpacePath;
		QDir dir;
		qstrWorkSpacePath = dir.currentPath();
		printf("path =%s\n", qstrWorkSpacePath.toUtf8().data());*/

		QString qstrDocumentRoot = QString::fromUtf8(httpdObject->documentRoot);
		//printf("httpdObject->documentRoot =%s\n", qstrDocumentRoot.toUtf8().data());

		char workSpacePath[R7_STRING_SIZE];
		R7_GetWorkspacePath(r7Sn, workSpacePath, R7_STRING_SIZE);

		QString qstrWorkSpacePath = QString::fromUtf8(workSpacePath);
		printf("workSpacePath = %s\n", qstrWorkSpacePath.toUtf8().data());

		res = WSAStartup(MAKEWORD(2, 2), &httpdObject->wsaData);
		if (res != 0) {
			//printf("WSAStartup failed with error: %d\n", iResult);
			R7_SetVariableInt(r7Sn, functionSn, 1, res);
			return -1;
		}

		//Bind
		// input TCP
		struct addrinfo *resultAddr = NULL;
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));

		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		char portString[20];
		_itoa(httpdObject->port, portString, 10);

		// Resolve the server address and port
		res = getaddrinfo(NULL, portString, &hints, &resultAddr);
		if (res != 0) {
			R7_Printf(r7Sn, "getaddrinfo failed with error: %d\n", res);
			WSACleanup();
			res = -1;
			R7_SetVariableInt(r7Sn, functionSn, 1, res);
			return res;
		}

		// Create a SOCKET for connecting to server
		httpdObject->socketListen = socket(resultAddr->ai_family, resultAddr->ai_socktype, resultAddr->ai_protocol);
		if (httpdObject->socketListen == INVALID_SOCKET) {
			R7_Printf(r7Sn, "socket failed with error: %ld\n", WSAGetLastError());
			freeaddrinfo(resultAddr);
			WSACleanup();
			res = -1;
			R7_SetVariableInt(r7Sn, functionSn, 1, res);
			return  res;
		}

		sockaddr_in service;
		memset(&service, 0, sizeof(struct sockaddr_in));
		service.sin_family = AF_INET;
		//inet_pton(AF_INET, "127.0.0.1", &(service.sin_addr));
		service.sin_port = htons(httpdObject->port);
		service.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(httpdObject->socketListen, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
			R7_Printf(r7Sn, "bind failed");
			closesocket(httpdObject->socketListen);
			res = -1;
			R7_SetVariableInt(r7Sn, functionSn, 1, res);
			return res;
		}

		// Listen Port 80 Backlog 1024
		// res
		res = listen(httpdObject->socketListen, httpdObject->backlogNum);
		if (res == SOCKET_ERROR) {
			R7_Printf(r7Sn, "listen failed with error: %d\n", WSAGetLastError());
			closesocket(httpdObject->socketListen);
			WSACleanup();
			res = -1;
			R7_SetVariableInt(r7Sn, functionSn, 1, res);
			return -res;
		}

		printf("listen port = %d\n", httpdObject->port);

		while (httpdObject->toStop == 0) {
			
			SOCKET socketClient;
			struct fd_set rfdsSocket;
			struct timeval timeoutSocket = { httpdObject->timeout,0 };

			//select accept
			while (1) {
				FD_ZERO(&rfdsSocket);
				FD_SET(httpdObject->socketListen, &rfdsSocket);

				//select return < 0 => select error.
				//select return = 0 => select timeout.
				res = select(0, &rfdsSocket, NULL, NULL, &timeoutSocket);
				
				if (0 > res) {
					R7_Printf(r7Sn, "select accept failed.\n");
					closesocket(httpdObject->socketListen);
					WSACleanup();
					res = -1;
					R7_SetVariableInt(r7Sn, functionSn, 1, res);
					return res;
				}
				else if (0 == res) {
					continue;
				}
				else {
					if (FD_ISSET(httpdObject->socketListen, &rfdsSocket)) {
						//recv get data.
						socketClient = accept(httpdObject->socketListen, NULL, NULL);
						if (socketClient == INVALID_SOCKET) {
							R7_Printf(r7Sn, "accept failed with error: %d\n", WSAGetLastError());
							closesocket(httpdObject->socketListen);
							WSACleanup();
							res = -1;
							R7_SetVariableInt(r7Sn, functionSn, 1, res);
							return res;
						}
						break;
					}
				}
			}
			
			printf("accept\n");
			printf("timeout set = %d\n", httpdObject->timeout);

			if (httpdObject->toStop != 0) {
				break;
			}

			// TODO: Read -> print
			//ouyput binary

			char recvBuf[R7_STRING_SIZE];
			char *httpData = NULL;
			char httpDataSize = 0;
			int httpDataLength = 0;
			int httpDataLengthNew = 0;
			int httpDataTotalLength = 0;
			//use select.
			struct fd_set rfds;
			struct timeval timeout = { httpdObject->timeout,0 };

			while (1) {
				FD_ZERO(&rfds);
				FD_SET(socketClient, &rfds);

				//select return < 0 => select error.
				//select return = 0 => select timeout.
				res = select(0, &rfds, NULL, NULL, &timeout);
				printf("select res = %d\n", res);
				if (0 > res) {
					break;
				}
				else if (0 == res) {
					break;
				}
				else {
					if (FD_ISSET(socketClient, &rfds)) {
						//recv get data.
						if ((res = recv(socketClient, recvBuf, R7_STRING_SIZE, 0)) > 0) {
								//printf("res = %d\n", res);
								//printf("recvBuf =%s\n", recvBuf);
								
								httpDataLengthNew = httpDataLength + res;

								if (httpDataLengthNew > httpDataSize) {
									char* buf = httpData;
									httpData = (char *)malloc(httpDataLengthNew);
									if (httpData == NULL) {
										printf("httpData == NULL!\n");
										return -1;
									}

									if (httpDataLength > 0) {
										memcpy(httpData, buf, httpDataLength);
									}
									else if (httpDataLength == 0) {//first time to find total size.
										// TODO: 有的沒有傳這個資料
										// TODO: string to QString
										// TODO: 可能沒有 Content-Length:
										QString httpFindContentLength = QString::fromUtf8(recvBuf, R7_STRING_SIZE);
										int contentLengthStart = (int)httpFindContentLength.indexOf("Content-Length: ", 0);
										int contentLengthEnd = (int)httpFindContentLength.indexOf("\r\n", contentLengthStart);
										int contentLength = httpFindContentLength.mid(contentLengthStart + (int)strlen("Content-Length: "), contentLengthEnd - contentLengthStart - (int)strlen("Content-Length: ")).toInt();
										//printf("contentLength = %d\n", contentLength);
										
										int headerLength = (int)httpFindContentLength.indexOf("\r\n\r\n", 0) + (int)strlen("\r\n\r\n");
										httpDataTotalLength = contentLength + headerLength;
									}

									memcpy(httpData + httpDataLength, recvBuf, res);

									free(buf);
								}

								httpDataLength += res;
								
								if (httpDataLength >= httpDataTotalLength) { // TODO httpd 
									//printf("resRecv = %d\n", res);
									//free(recvBuf);
									printf("braek!\n");
									break;
								}
							}
							if (res < 0) {
								R7_Printf(r7Sn, "recv fail\n");
								R7_SetVariableInt(r7Sn, functionSn, 1, res);
								closesocket(socketClient);
								closesocket(httpdObject->socketListen);
								WSACleanup();
								return -2;
							}
							else if (res == 0) {
								R7_Printf(r7Sn, "recv close.\n");
								//free(httpData);
								//closesocket(socketClient);
								//printf("close socketClient!\n");
								//continue;
								
								//temporarily break
								res = -1;
								break;
							}
					}
				}
			}

			//selcet < 0.
			if (res < 0) {
				free(httpData);
				closesocket(socketClient);
				printf("close socketClient!\n");
				continue;
			}

			// TODO: Parse http header
			QString qstr;
			
			//print httpData.
			//for (int i = 0; i < httpDataTotalLength; i++) {
			//	printf("%c", httpData[i]);
			//}
			
			qstr = QString::fromUtf8(httpData, httpDataLength);
			printf("String Length = %d\n", httpDataLength);
			printf("QString Length =%d\n", qstr.length());
			printf("****************************\n");

			//test
			bool testPost = false;
			bool testGet = false;
			int wordLengh = 0;

			QString headerData = qstr.mid(0, (int)qstr.indexOf("\r\n\r\n", 0) + (int)strlen("\r\n"));
			QString strKey = "Protocol";
			QString strValue = "GET";

			//1. headerProtocol
			int first = headerData.indexOf("GET", 0);
			int end = headerData.indexOf("\r\n", 0);
			if (first < 0 || end < 0) {
				first = headerData.indexOf("POST", 0);
				end = headerData.indexOf("\r\n", 0);

				if (first < 0 || end < 0) {
					qstr.clear();
					free(httpData);
					closesocket(socketClient);
					printf("close socketClient!\n");
					continue;
				}
				else {
					testPost = true;
				}
			}
			else {
				testGet = true;
			}

			strValue = headerData.mid(first, end);
			(*httpdObject->mapHeader)[strKey] = strValue;

			//save path.
			if (testGet) {
				first = strValue.indexOf("GET /", 0);
				end = strValue.lastIndexOf(" ");
				(*httpdObject->mapHeader)["Path"] = strValue.mid(first + (int)strlen("GET "), end - first - (int)strlen("GET "));
			}

			int lineCount = 1;

			while (1) {
				QString headerLineData;
				headerLineData = qstr.section("\r\n", lineCount, lineCount);
				int findKey = headerLineData.indexOf(": ", 0);

				if (findKey < 0) {
					break;
				}

				strKey = headerLineData.mid(0, findKey);
				strValue = headerLineData.mid(findKey + (int)strlen(": "));

				(*httpdObject->mapHeader)[strKey] = strValue;
				
				lineCount++;
			}

			//print mapHeader
			for (auto it = (*httpdObject->mapHeader).cbegin(); it != (*httpdObject->mapHeader).cend(); ++it) {
				printf("strKey   =%s\n", it->first.toUtf8().data());
				printf("strValue =%s\n", it->second.toUtf8().data());
			}

			printf("****************************************************\n");
			// TODO: Write http header and context
			
			//Test Post
			if (testPost) {
				//find webKitFormBoundary
				first = qstr.indexOf("Content-Type: multipart/form-data; boundary=");
				end = qstr.indexOf("\n", first);
				wordLengh = (int)strlen("Content-Type: multipart/form-data; boundary=");
				if (first < 0) {
					printf("find webKitFormBoundary fail!\n");
					qstr.clear();
					free(httpData);
					closesocket(socketClient);
					printf("close socketClient!\n");
					continue;
				}

				QString webKitFormBoundaryValue = qstr.mid(first + wordLengh, end - first - wordLengh -1);
				//printf("webKitFormBoundaryValue =%s\nLen=%d\n", webKitFormBoundaryValue.toLatin1().data(), webKitFormBoundaryValue.length());
				
				webKitFormBoundaryValue.sprintf("--%s", webKitFormBoundaryValue.toLatin1().data());
				//printf("webKitFormBoundaryValue2 =%s\nLen=%d\n", webKitFormBoundaryValue.toLatin1().data(), webKitFormBoundaryValue.length());

				//use string
				std::string getFileChar;
				std::string checkWebKey(webKitFormBoundaryValue.toStdString().c_str());
				int findFirstBoundaryPos = qstr.indexOf(webKitFormBoundaryValue, 0);
				
				printf("webKitFormBoundaryCheck = %s\n", checkWebKey.c_str());

				//Parse
				std::string httpDataStr(httpData, httpDataLength);
				while (1) {
					int findNextBoundaryPos = (int)httpDataStr.find(checkWebKey, findFirstBoundaryPos + checkWebKey.length() + (int)strlen("\r\n"));
					
					if (findNextBoundaryPos < 0) {
						printf("break!\n");
						break;
					}

					getFileChar = httpDataStr.substr(findFirstBoundaryPos + checkWebKey.length() + (int)strlen("\r\n"), findNextBoundaryPos - findFirstBoundaryPos - checkWebKey.length() - (int)strlen("\r\n"));
					
					if (res = (int)getFileChar.find("\r\nContent-Type: ", 0) < 0) {
						int pos = (int)getFileChar.find("Content-Disposition: form-data; name=\"", 0);
						int pos2 = (int)getFileChar.find("\"\r\n\r\n", 0);

						std::string stringName = getFileChar.substr(pos + (int)strlen("Content-Disposition: form-data; name=\""), pos2 - pos - (int)strlen("Content-Disposition: form-data; name=\""));
						std::string stringValue = getFileChar.substr(pos2 + (int)strlen("\"\r\n\r\n"), getFileChar.length() - pos2 - (int)strlen("\r\n") - (int)strlen("\"\r\n\r\n"));
					
						printf("stringName =%s\n", stringName.c_str());
						printf("stringValue =%s\n", stringValue.c_str());
					}
					else {//File
						int pos = (int)getFileChar.find("Content-Disposition: form-data; name=\"", 0);
						int pos2 = (int)getFileChar.find("\"; filename=\"", 0);
						int pos3 = (int)getFileChar.find("\"\r\n", pos2);
						int pos4 = (int)getFileChar.find("Content-Type: ", pos3);
						int pos5 = (int)getFileChar.find("\r\n\r\n", pos4);

						std::string stringName = getFileChar.substr(pos + (int)strlen("Content-Disposition: form-data; name=\""), pos2 - pos - (int)strlen("Content-Disposition: form-data; name=\""));
						std::string stringfileName = getFileChar.substr(pos2 + (int)strlen("\"; filename=\""), pos3 - pos2 - (int)strlen("\"; filename=\""));
						std::string stringfileType = getFileChar.substr(pos4 + (int)strlen("Content-Type: "), pos5 - pos4 - (int)strlen("Content-Type: "));
						std::string stringfileData = getFileChar.substr(pos5 + (int)strlen("\r\n\r\n"), getFileChar.length() - pos5 - (int)strlen("\r\n") - (int)strlen("\r\n\r\n"));

						printf("stringName =%s\n", stringName.c_str());
						printf("stringfileName =%s\n", stringfileName.c_str());
						printf("stringfileType =%s\n", stringfileType.c_str());

						//test save file.
						HANDLE hOutputFile = INVALID_HANDLE_VALUE;
						DWORD dwBytesWrite, dwBytesToWrite;
						dwBytesToWrite = DWORD(1);
						dwBytesWrite = 0;
						if (stringfileData.length() > 0) {//debug
							if (true) {
								char name[R7_STRING_SIZE];

								if ((int)stringfileName.find('\\', 0) > 0) {//for old version of IE
									int line = (int)stringfileName.find_last_of('\\');								
									stringfileName = stringfileName.substr(line + (int)strlen("\\"), stringfileName.length() - line);
								}
								
								sprintf(name, "C:\\Users\\allen93\\Documents\\GitHub\\OpenR8\\workspace\\Httpd\\%s", stringfileName.c_str());
								//sprintf(name, "C:\\Users\\allen93\\Documents\\GitHub\\OpenR8\\workspace\\Httpd\\a.zip");
								//printf("file=%s\n", name);

								wchar_t nameW[R7_STRING_SIZE];
								memset(nameW, 0, sizeof(wchar_t) * R7_STRING_SIZE);
								MultiByteToWideChar(CP_UTF8, 0, name, -1, nameW, R7_STRING_SIZE * 2);

								hOutputFile = CreateFile(nameW, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hOutputFile == INVALID_HANDLE_VALUE) {
									R7_Printf(r7Sn, "hOutputFile INVALID_HANDLE_VALUE!\n");
									CloseHandle(hOutputFile);
									R7_SetVariableInt(r7Sn, functionSn, 1, -1);
									closesocket(socketClient);
									closesocket(httpdObject->socketListen);
									WSACleanup();
									return -1;
								}
							}

							char data[1];

							for (int i = 0; i < stringfileData.length(); i++) {
								data[0] = stringfileData.c_str()[i];
								WriteFile(hOutputFile, data, dwBytesToWrite, &dwBytesWrite, NULL);//debug
							}

							CloseHandle(hOutputFile);
						}
					}

					findFirstBoundaryPos = findNextBoundaryPos;
				}
			}

			//Test Get
			if (testGet) {
				auto findProtocol = (*httpdObject->mapHeader).find("Protocol");
				
				first = findProtocol->second.indexOf("GET /", 0);
				end = findProtocol->second.lastIndexOf(" ");
				QString qstrFileName = findProtocol->second.mid(first + (int)strlen("GET /"), end - first - (int)strlen("GET /"));

				//url decode.
				QUrl url_path(qstrFileName);
				url_path.fromPercentEncoding(qstrFileName.toUtf8());
				printf("qstrFileName =%s\n", url_path.toString().toUtf8().data());

				qstrWorkSpacePath.append(qstrDocumentRoot).append(url_path.toString().toUtf8().data());
				printf("WorkSpacePath =%s!\n", qstrWorkSpacePath.toUtf8().data());

				wchar_t workSpacePathW[R7_STRING_SIZE];
				memset(workSpacePathW, 0, sizeof(wchar_t) * R7_STRING_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, qstrWorkSpacePath.toUtf8().data(), -1, workSpacePathW, R7_STRING_SIZE * 2);

				//open file.
				HANDLE file = CreateFile(workSpacePathW, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if (file == INVALID_HANDLE_VALUE) {
					R7_Printf(r7Sn, "hOutputFile INVALID_HANDLE_VALUE!\n");
					CloseHandle(file);

					send(socketClient, "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-type: text/html; charset=utf-8\r\n\r\n", 87, 0);
					send(socketClient, "404 Not Found!<br>No file found!<br>", 38, 0);

					qstr.clear();
					qstrWorkSpacePath.clear();
					free(httpData);
					closesocket(socketClient);
					printf("close socketClient!\n");
					continue;
				}

				DWORD dwBytesRead, dwBytesToRead, fileSize;
				char *fileBuffer, *tmpBuf;

				fileSize = GetFileSize(file, NULL);//get file size

				fileBuffer = (char *)malloc(fileSize);
				
				dwBytesToRead = fileSize;
				dwBytesRead = 0;
				tmpBuf = fileBuffer;

				do {
					ReadFile(file, tmpBuf, dwBytesToRead, &dwBytesRead, NULL);
					if (dwBytesRead == 0) {
						break;
					}
					dwBytesToRead -= dwBytesRead;
					tmpBuf += dwBytesRead;
				} while (dwBytesToRead > 0);

				CloseHandle(file);
				
				//need include <io.h>
				//_write((int)socketClient, "Failed to open file", 19);
				//_write((int)socketClient, "HTTP / 1.1 200 OK\r\nContent-type: text/html; charset=utf-8\r\n\r\n", 63);
				char httpSendBuffer[R7_STRING_SIZE];
				char httpContentTypBuffer[R7_STRING_SIZE];
				
				//print on web.
				int firstAscii = int(fileBuffer[0]);
				int secondAscii = int(fileBuffer[1]);
				if (firstAscii < 0) {
					firstAscii = 256 + firstAscii;
				}
				if (secondAscii < 0) {
					secondAscii = 256 + secondAscii;
				}

				if (firstAscii == 255 && secondAscii == 216) {//JPG = 255216
					sprintf(httpContentTypBuffer, "Content-Type: image/jpeg");
				}
				else if (firstAscii == 137 && secondAscii == 80) {//PNG = 13780,
					sprintf(httpContentTypBuffer, "Content-Type: image/png");
				}
				else if ((firstAscii == 102 && secondAscii == 100) || (firstAscii == 60 && secondAscii == 33) || (firstAscii == 239 && secondAscii == 187)) {//TXT = 102100,  //HTML = 6033,  text: 239 187
					sprintf(httpContentTypBuffer, "Content-type: text/html");
				}
				else {//Content-Type:text/html; charset=utf-8
					sprintf(httpContentTypBuffer, "Content-type: text/html; charset=utf-8");
				}

				sprintf(httpSendBuffer, "HTTP/1.1 200 OK\r\nConnection: close\r\n%s\r\n\r\n", httpContentTypBuffer);
				
				send(socketClient, httpSendBuffer, (int)strlen(httpSendBuffer), 0);
				send(socketClient, fileBuffer, (int)fileSize, 0);

				free(fileBuffer);
			}

			qstr.clear();
			qstrWorkSpacePath.clear();
			free(httpData);
			closesocket(socketClient);
			printf("close socketClient!\n");
		}

		R7_SetVariableInt(r7Sn, functionSn, 1, res);
		return 1;
	}

	static int Httpd_Stop(int r7Sn, int functionSn) {	
		int result = 1;

		// Stop Connect Loop
		void *variableObject = NULL;
		result = R7_GetVariableObject(r7Sn, functionSn, 2, &variableObject);
		if (result <= 0) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			return -1;
		}

		HttpdObject_t *httpdObject = ((HttpdObject_t*)variableObject);

		httpdObject->toStop = 1;
		closesocket(httpdObject->socketListen);

		R7_SetVariableInt(r7Sn, functionSn, 1, result);
		R7_SetVariableObject(r7Sn, functionSn, 2, httpdObject);

		return 1;
	}

	R7_API int R7Library_Init(void) {
		SetConsoleOutputCP(65001);
		setlocale(LC_ALL, "en_US.UTF-8");

		//Register your functions in this API.
		R7_RegisterFunction("Httpd_Init", (R7Function_t)&Httpd_Init);
		R7_RegisterFunction("Httpd_Set", (R7Function_t)&Httpd_Set);
		R7_RegisterFunction("Httpd_Start", (R7Function_t)&Httpd_Start);
		R7_RegisterFunction("Httpd_Stop", (R7Function_t)&Httpd_Stop);
		
		return 1;
	}


	R7_API int R7Library_Close(void) {
		// If you have something to do before close R7(ex: free memory), you should handle them in this API.

		//WSACleanup();

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
		json_object_set_new(functionGroupObject, "name", json_string("Httpd"));
		json_array_append(functionGroupArray, functionGroup);

		functionArray = json_array();
		json_object_set_new(functionGroupObject, "functions", functionArray);

		//Httpd_Init
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("Httpd_Init"));
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
		json_object_set_new(variableObject, "name", json_string("httpd"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);


		//Httpd_Set
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("Httpd_Set"));
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
		json_object_set_new(variableObject, "name", json_string("httpd"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("port"));
		json_object_set_new(variableObject, "type", json_string("Int"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("documentRoot"));
		json_object_set_new(variableObject, "type", json_string("String"));
		json_object_set_new(variableObject, "direction", json_string("IN, FOLDER_PATH"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("threadNum"));
		json_object_set_new(variableObject, "type", json_string("Int"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("backlogNum"));
		json_object_set_new(variableObject, "type", json_string("Int"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		variableObject = json_object();
		variable = json_object();
		json_object_set_new(variable, "variable", variableObject);
		json_object_set_new(variableObject, "name", json_string("timeout"));
		json_object_set_new(variableObject, "type", json_string("Int"));
		json_object_set_new(variableObject, "direction", json_string("IN"));
		json_array_append(variableArray, variable);

		//Httpd_Start
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("Httpd_Start"));
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
		json_object_set_new(variableObject, "name", json_string("httpd"));
		json_object_set_new(variableObject, "type", json_string("Object"));
		json_object_set_new(variableObject, "direction", json_string("IN, OUT"));
		json_array_append(variableArray, variable);

		//Httpd_Stop
		function = json_object();
		functionObject = json_object();
		json_object_set_new(function, "function", functionObject);
		json_object_set_new(functionObject, "name", json_string("Httpd_Stop"));
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
		json_object_set_new(variableObject, "name", json_string("httpd"));
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
