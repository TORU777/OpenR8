/*
Copyright (c) 2004-2018 Open Robot Club. All rights reserved.
CGI library for R7.
*/


#include "R7.hpp"
#include <tchar.h>
#include <wx/string.h>
#include <fcntl.h>

using namespace std;


#ifdef __cplusplus
extern "C"
{
#endif

#define ENV_VARIABLES_SIZE 17
#define CGI_VARIABLES_SIZE 64

// CGI_ParseRequest
typedef struct {
	int isEnv;
	int isGet;
	int isPost;
	wxString name;
	wxString strValue;
	void *binaryValue;
	int binarySize;
} CGI_VARIABLE_t;

static CGI_VARIABLE_t cgiVariables[CGI_VARIABLES_SIZE] = {0};
static int cgiVariablesLength = 0;

static int CGI_GetPostFile(int r7Sn, int functionSn) {
	int result = 0;

	//output binary
	unsigned char *binary;

	//intput
	char name[R7_STRING_SIZE];
	R7_GetVariableString(r7Sn, functionSn, 3, name, R7_STRING_SIZE);

	wchar_t nameW[R7_STRING_SIZE];
	memset(nameW, 0, sizeof(wchar_t) * R7_STRING_SIZE);
	MultiByteToWideChar(CP_UTF8, 0, name, -1, nameW, R7_STRING_SIZE * 2);
	//WideCharToMultiByte(CP_UTF8, 0, nameW, -1, name, R7_STRING_SIZE * 2, NULL, NULL);
	wxString nameWxstring(nameW, wxConvUTF8);
	int cmpStringP = 0;

	for (int i = 0; i < CGI_VARIABLES_SIZE; i++) {

		if (cgiVariables[i].isPost == 1 && cgiVariables[i].binarySize >= 0) {
			if ((cgiVariables[i].name.Cmp(nameWxstring)) == 0) {
				/*wxPrintf("name = %s<\br>" ,cgiVariables[i].name);
				wxPrintf("nameWxstring = %s", nameWxstring);
				printf("<br>success");*/
				cmpStringP = i;
				break;
			}
		}
	}
	//if file exist
	if (cgiVariables[cmpStringP].binarySize > 0) {
		result = cgiVariables[cmpStringP].binarySize;
		binary = (unsigned char*)malloc(result + 1);

		binary = (unsigned char*)cgiVariables[cmpStringP].binaryValue;

		//printf("<br>result = %d", result);

		//printf("<br>binary2 = %s", binary);

	}
	else {
		result = cgiVariables[cmpStringP].strValue.length();

		char *tempBinary = (char*)malloc(result);
		strcpy(tempBinary, cgiVariables[cmpStringP].strValue.mb_str());
		binary = (unsigned char*)tempBinary;
		result = int(strlen(tempBinary));


		/*printf("<br>result = %d", result);
		printf("<br>binary = %s", binary);*/
	}


	R7_SetVariableInt(r7Sn, functionSn, 1, result);
	R7_SetVariableBinary(r7Sn, functionSn, 2, binary, result);
	free(binary);


	return 1;
}


static int CGI_GetEnvString(int r7Sn, int functionSn) {
	int result = 0;
	char name[R7_STRING_SIZE];
	R7_GetVariableString(r7Sn, functionSn, 3, name, R7_STRING_SIZE);

	wchar_t nameW[R7_STRING_SIZE];
	memset(nameW, 0, sizeof(wchar_t) * R7_STRING_SIZE);
	MultiByteToWideChar(CP_UTF8, 0, name, -1, nameW, R7_STRING_SIZE * 2);

	char nameGet[R7_STRING_SIZE];
	WideCharToMultiByte(CP_UTF8, 0, nameW, -1, nameGet, R7_STRING_SIZE * 2, NULL, NULL);

	int i;
	char cstring[R7_STRING_SIZE];
	char *string = (char*)malloc(R7_STRING_SIZE);
	for (i = 0; i < CGI_VARIABLES_SIZE; i++) {
		if (cgiVariables[i].isEnv == 0) {
			continue;
		}

		strncpy(cstring, (const char*)cgiVariables[i].name.mb_str(wxConvUTF8), R7_STRING_SIZE - 1);
		if (strcmp(cstring, nameGet) == 0) {
			strncpy(string, (const char*)cgiVariables[i].strValue.mb_str(wxConvUTF8), R7_STRING_SIZE - 1);
			result = strlen(cstring);
			//	printf("name = %s, value = %s", cstring, string);
			i = CGI_VARIABLES_SIZE;
		}

		if (i == CGI_VARIABLES_SIZE - 1) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			free(string);
		}
	}

	R7_SetVariableInt(r7Sn, functionSn, 1, result);
	R7_SetVariableString(r7Sn, functionSn, 2, string);
	free(string);
	return 1;
	return 1;
}


static int CGI_GetGetString(int r7Sn, int functionSn) {
	int result = 0;
	char name[R7_STRING_SIZE];
	R7_GetVariableString(r7Sn, functionSn, 3, name, R7_STRING_SIZE);

	wchar_t nameW[R7_STRING_SIZE];
	memset(nameW, 0, sizeof(wchar_t) * R7_STRING_SIZE);
	MultiByteToWideChar(CP_UTF8, 0, name, -1, nameW, R7_STRING_SIZE * 2);

	char nameGet[R7_STRING_SIZE];
	WideCharToMultiByte(CP_UTF8, 0, nameW, -1, nameGet, R7_STRING_SIZE * 2, NULL, NULL);

	int i;
	char cstring[R7_STRING_SIZE];
	char *string = (char*)malloc(R7_STRING_SIZE);
	for (i = 0; i < CGI_VARIABLES_SIZE; i++) {
		if (cgiVariables[i].isGet == 0) {
			continue;
		}

		strncpy(cstring, (const char*)cgiVariables[i].name.mb_str(wxConvUTF8), R7_STRING_SIZE - 1);
		if (strcmp(cstring, nameGet) == 0) {
			strncpy(string, (const char*)cgiVariables[i].strValue.mb_str(wxConvUTF8), R7_STRING_SIZE - 1);
			result = strlen(cstring);
			//	printf("name = %s, value = %s", cstring, string);
			i = CGI_VARIABLES_SIZE;
		}

		if (i == CGI_VARIABLES_SIZE - 1) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			free(string);
		}
	}

	R7_SetVariableInt(r7Sn, functionSn, 1, result);
	R7_SetVariableString(r7Sn, functionSn, 2, string);
	free(string);
	return 1;
}


static int CGI_GetPostString(int r7Sn, int functionSn) {
	int result = 0;
	char name[R7_STRING_SIZE];
	R7_GetVariableString(r7Sn, functionSn, 3, name, R7_STRING_SIZE);

	wchar_t nameW[R7_STRING_SIZE];
	memset(nameW, 0, sizeof(wchar_t) * R7_STRING_SIZE);
	MultiByteToWideChar(CP_UTF8, 0, name, -1, nameW, R7_STRING_SIZE * 2);

	char nameGet[R7_STRING_SIZE];
	WideCharToMultiByte(CP_UTF8, 0, nameW, -1, nameGet, R7_STRING_SIZE * 2, NULL, NULL);

	int i;
	char cstring[R7_STRING_SIZE];
	char *string = (char*)malloc(R7_STRING_SIZE);
	for (i = 0; i < CGI_VARIABLES_SIZE; i++) {
		if (cgiVariables[i].isPost == 0) {
			continue;
		}
		strncpy(cstring, (const char*)cgiVariables[i].name.mb_str(wxConvUTF8), R7_STRING_SIZE - 1);
		if (strcmp(cstring, nameGet) == 0) {
			strncpy(string, (const char*)cgiVariables[i].strValue.mb_str(wxConvUTF8), R7_STRING_SIZE - 1);
			result = strlen(cstring);
			//printf("name = %s, value = %s", cstring, string);
			i = CGI_VARIABLES_SIZE;
		}

		if (i == CGI_VARIABLES_SIZE - 1) {
			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
			free(string);
		}
	}

	R7_SetVariableInt(r7Sn, functionSn, 1, result);
	R7_SetVariableString(r7Sn, functionSn, 2, string);
	free(string);
	return 1;
}


char htod(char c1, char c2) {
	char c = 0;

	if (c1 >= '0' && c1 <= '9') {
		c = c1 - '0';
	}
	else if (c1 >= 'a' && c1 <= 'z') {
		c = c1 - 'a' + 10;
	}
	else if (c1 >= 'A' && c1 <= 'Z') {
		c = c1 - 'A' + 10;
	}

	if (c2 >= '0' && c2 <= '9') {
		c = c * 16 + c2 - '0';
	}
	else if (c2 >= 'a' && c2 <= 'z') {
		c = c * 16 + c2 - 'a' + 10;
	}
	else if (c2 >= 'A' && c2 <= 'Z') {
		c = c * 16 + c2 - 'A' + 10;
	}

	return c;
}


int urlDecoder(wxString &src, wxString &dst) {
	if (src.empty()) {
		return -1;
	}

	int i, j;
	int strLen = src.length();
	char srcChar[R7_STRING_SIZE];
	strncpy(srcChar, (const char*)src.mb_str(), strLen);
	srcChar[strLen] = '\0';

	char dstChar[R7_STRING_SIZE];

	for (i = 0, j = 0; i < strLen;) {
		if (srcChar[i] == '%') {
			dstChar[j++] = htod(src[i + 1], src[i + 2]);
			i += 3;
		}
		else if (srcChar[i] == '+') {
			dstChar[j++] = '&';
			dstChar[j++] = 'n';
			dstChar[j++] = 'b';
			dstChar[j++] = 's';
			dstChar[j++] = 'p';
			dstChar[j++] = ';';
			i++;
		}
		else {
			dstChar[j++] = srcChar[i++];
		}
	}

	dstChar[j++] = '\0';
	dst = wxString::FromUTF8(dstChar);

	return 1;
}


int parseGetNameValues(const char *src, int &index) {
	if (src == NULL) {
		return -1;
	}

	int i = 0, j = 0;
	int nameF = 0, nameL = 0;
	int valueF = 0, valueL = 0;
	int count = 0;
	int temp = 0;
	int srcLength = (int)strlen(src);
	wxString srcString(src, wxConvUTF8);
	wxString nameTmp, valueTmp;

	for (i = 0; i < srcLength; i++) {
		if (src[i] == '=') {
			//printf("i = %d<br>", i);
			nameL = i - 1;
			j = valueF = i + 1;
			while (src[j] != '&' && j < srcLength) {
				//printf("j = %d<br>", j);
				j++;
			}
			valueL = j - 1;

			if (count > 0) {
				nameF = temp;
			}

			nameTmp = srcString.substr(nameF, nameL - nameF + 1);
			urlDecoder(nameTmp, nameTmp);
			//names.push_back(nameTmp);

			valueTmp = srcString.substr(valueF, valueL - valueF + 1);
			urlDecoder(valueTmp, valueTmp);
			//values.push_back(valueTmp);
			//printf("index = %d, name = %s, value = %s<br>", index, nameTmp.mb_str(wxConvUTF8).data(), valueTmp.mb_str(wxConvUTF8).data());

			cgiVariables[index].name = nameTmp;
			cgiVariables[index].isEnv = 0;
			cgiVariables[index].isGet = 1;
			cgiVariables[index].isPost = 0;
			cgiVariables[index].strValue = valueTmp;
			cgiVariables[index].binarySize = 0;

			count++;
			index++;

			temp = valueL + 2;
		}
	}

	return 1;
}

int parsePostNameValues(char *src, int &cgiVariablesIndex) {
	//printf("test = %s", src);
	//decoding
	//get boundary
	wchar_t *contentW = L"CONTENT_TYPE";
	wchar_t *boundaryW;
	boundaryW = _wgetenv(contentW);

	char boundary[R7_STRING_SIZE];
	WideCharToMultiByte(CP_UTF8, 0, boundaryW, -1, boundary, R7_STRING_SIZE * 2, NULL, NULL);

	//find boundary in CONTENT_TYPE
	char findBoundary[R7_STRING_SIZE];
	sprintf(findBoundary, "boundary=");

	char *test = strstr(boundary, findBoundary);
	int address = int(test - boundary);

	//get WebKitFormBoundary
	char webKitFormBoundary[R7_STRING_SIZE];
	strncpy(webKitFormBoundary, boundary + address + int(strlen(findBoundary)), int(strlen("----WebKitFormBoundary") + 16));

	//Separate the data
	wxString string2(src, wxConvUTF8);
	wxString string3(src, wxConvUTF8);
	wxString string4;
	//printf("<br>string2 = %s<br>", string2.mb_str(wxConvUTF8).data());
	//------WebKitFormBoundary
	char webKitFormBoundary2[R7_STRING_SIZE];
	sprintf(webKitFormBoundary2, "--%s", webKitFormBoundary);
	wxString web(webKitFormBoundary2, wxConvUTF8);

	int count = 0;
	int len = string2.length();
	int len2 = string2.length();
	int index = 0;
	int str = 0;

	int countNum[100];

	do {
		str = (int)string2.find(web, index);
		if (str < 0) {
			break;
		}

		int rr = len - str - (int)web.length();

		string2 = string2.substr(str + (int)web.length(), rr);
		//printf("<br>string2 = %s<br>", static_cast<const char*>(string2.mb_str(wxConvUTF8)));
		len = string2.length();
		countNum[count] = len2 - len;
		count++;

	} while (1);

	//Analyze the data
	for (int i = 0; i < count - 1; i++)
	{
		string4 = string3.substr(countNum[i]/* + (int)web.length()*/, countNum[i + 1] - countNum[i] - (int)web.length());

		wxString name(wxT("Content-Disposition: form-data; name=\""));
		//wxString name2(wxT("\"\n"));
		wxString name2(wxT("\""));
		wxString name3(wxT("\"; filename=\""));
		str = (int)string4.find(name, index);

		int str2 = (int)string4.find(name2, str + 1 + (int)name.length());
		int str3 = (int)string4.find(name3, str + 1 + (int)name.length());

		//printf("str2 :%d<br>", str2);
		//printf("str3 :%d<br>", str3);

		if (str2 != str3)
		{
			wxString nameResult = string4.substr(str + (int)name.length(), str2 - str - (int)name.length());
			wxString valueResult = string4.substr(str2 + 3, (int)string4.length() - str2 - 4);

			cgiVariables[cgiVariablesIndex].name = nameResult;
			cgiVariables[cgiVariablesIndex].strValue = valueResult;
			cgiVariables[cgiVariablesIndex].isEnv = 0;
			cgiVariables[cgiVariablesIndex].isGet = 0;
			cgiVariables[cgiVariablesIndex].isPost = 1;
			cgiVariablesIndex++;

			//printf("nameResult :%s<br>", static_cast<const char*>(nameResult.mb_str(wxConvUTF8)));
			//printf("valueResult :%s<br>", static_cast<const char*>(valueResult.mb_str(wxConvUTF8)));
		}
		else
		{
			continue;
		}
	}
	return 1;
}


static int CGI_ParseRequest(int r7Sn, int functionSn) {
	int result = 0;
	wchar_t * envVariableW[ENV_VARIABLES_SIZE] = {
		L"CONTENT_LENGTH", L"QUERY_STRING", L"SERVER_SOFTWARE", L"SERVER_NAME" , L"SERVER_PROTOCOL",
		L"REQUEST_METHOD", L"PATH_INFO", L"PATH_TRANSLATED", L"SCRIPT_NAME", L"GATEWAY_INTERFACE",
		L"REMOTE_HOST", L"CONTENT_TYPE", L"SERVER_PORT", L"HTTP_COOKIE", L"HTTP_USER_AGENT",
		L"REMOTE_ADDR", L"SCRIPT_FILENAME"
	};

	wchar_t *getEnvVariableW;

	cgiVariablesLength = 0;

	for (int i = 0; i < ENV_VARIABLES_SIZE; i++) {
		if (envVariableW[i] == NULL) {
			continue;
		}

		getEnvVariableW = _wgetenv(envVariableW[i]);

		if (getEnvVariableW == NULL) {
			continue;
		}

		if (wcslen(getEnvVariableW) == 0) {
			continue;
		}

		//wchar_t convert to char
		int utf8Size = WideCharToMultiByte(CP_UTF8, 0, getEnvVariableW, -1, NULL, 0, NULL, false);
		char* getEnvVariable = new char[utf8Size + 1];
		WideCharToMultiByte(CP_UTF8, 0, getEnvVariableW, -1, getEnvVariable, utf8Size, NULL, false);

		int utf8envVariableSize = WideCharToMultiByte(CP_UTF8, 0, envVariableW[i], -1, NULL, 0, NULL, false);
		char* envVariable = new char[utf8envVariableSize + 1];
		WideCharToMultiByte(CP_UTF8, 0, envVariableW[i], -1, envVariable, utf8envVariableSize, NULL, false);

		//get
		if (envVariableW[i] == L"QUERY_STRING") {
			parseGetNameValues(getEnvVariable, cgiVariablesLength);
			continue;
		}
		//post
		else if (envVariableW[i] == L"CONTENT_LENGTH") {

			int contentLength = int(atoi(getEnvVariable));
			if (contentLength == 0) {
				return -1;
			}

			if (1) {
				//get boundary
				char *e;
				char * multipartBoundaryTemp = "";
				e = getenv("CONTENT_TYPE");

				char *contentType = (char*)malloc(strlen(e) + 1);
				strcpy(contentType, e);
				contentType[(strlen(e) + 1)] = '\0';

				char find[] = "; boundary=";
				char *sat1 = strstr(contentType, find);
				multipartBoundaryTemp = sat1 + strlen(find);
				multipartBoundaryTemp[(strlen(sat1) + strlen(find))] = '\0';

				_setmode(_fileno(stdin), _O_BINARY);
				_setmode(_fileno(stdout), _O_BINARY);

				//read post string
				//FILE *cgiIn;
				//cgiIn = stdin;

				HANDLE hInputFile = GetStdHandle(STD_INPUT_HANDLE);
				DWORD dwInputBytesWrite, dwInputBytesToWrite;
				dwInputBytesToWrite = DWORD(1);
				dwInputBytesWrite = 0;
				int readFileErr = 0;

				int boffset = 0;
				char buffer[1];
				char multipartBoundary[R7_PAGE_SIZE];
				int boundaryLength = 0;

				sprintf(multipartBoundary, "--%s", multipartBoundaryTemp);
				boundaryLength = strlen(multipartBoundary);

				while (1) {
					//int err = fread(buffer, 1, 1, cgiIn);
					readFileErr = ReadFile(hInputFile, buffer, dwInputBytesToWrite, &dwInputBytesWrite, NULL);
					if (readFileErr != 1) {
						break;
					}
					if (buffer[0] == multipartBoundary[boffset]) {
						boffset++;
						if (boffset == boundaryLength) {
							break;
						}
					}
				}

				char valueName[1024];
				char fileName[1024];
				char contentTypeName[1024];
				char *postBinaryBuffer = (char*)malloc(contentLength);//free

				char value[R7_PAGE_SIZE];
				char attr[R7_PAGE_SIZE];
				while (1) {

					readFileErr = ReadFile(hInputFile, buffer, dwInputBytesToWrite, &dwInputBytesWrite, NULL);
					readFileErr = ReadFile(hInputFile, buffer, dwInputBytesToWrite, &dwInputBytesWrite, NULL);
					//fread(buffer, 1, 1, cgiIn);// \r
					//fread(buffer, 1, 1, cgiIn);//\n

					valueName[0] = '\0';
					fileName[0] = '\0';
					contentTypeName[0] = '\0';

					int valueFound = 0;
					int attrLen = 0;
					int valueLen = 0;

					int attrSpace = sizeof(attr);
					int valueSpace = sizeof(value);
					int existContent = false;

					while (1) {
						valueFound = 0;
						attrLen = 0;
						valueLen = 0;
						while (1) {//Content-Disposition: find "name" and "filename" , Content - Type: ""
							readFileErr = ReadFile(hInputFile, buffer, dwInputBytesToWrite, &dwInputBytesWrite, NULL);
							//readFileErr = fread(buffer, 1, 1, cgiIn);
							if (!readFileErr) {
								break;
							}
							if (buffer[0] == '\r') {
								readFileErr = ReadFile(hInputFile, buffer, dwInputBytesToWrite, &dwInputBytesWrite, NULL);
								//readFileErr = fread(buffer, 1, 1, cgiIn);
								if (readFileErr == 1) {
									if (buffer[0] != '\n') {
										value[valueLen++] = *buffer;
									}
								}
								break;
							}
							else if (buffer[0] == '\n') {
								break;
							}
							else if ((buffer[0] == ':') && attrLen) {
								valueFound = 1;
								while (ReadFile(hInputFile, buffer, dwInputBytesToWrite, &dwInputBytesWrite, NULL)) {
									//while (fread(buffer, 1, 1, cgiIn) == 1) {
									if (!isspace(buffer[0])) {
										value[valueLen++] = *buffer;
										break;
									}
								}
							}
							else if (!valueFound) {
								if (!isspace(*buffer)) {
									if (attrLen < (attrSpace - 1)) {
										attr[attrLen++] = *buffer;
									}
								}
							}
							else if (valueFound) {
								if (valueLen < (valueSpace - 1)) {
									value[valueLen++] = *buffer;
								}
							}
						}

						if (attrSpace) {
							attr[attrLen] = '\0';
						}
						if (valueSpace) {
							value[valueLen] = '\0';
						}

						if (true) {
							if (!strcmp(attr, "Content-Disposition")) {//name="file1"; filename="1.png"
								for (int i = 0; i < 2; i++) {
									char find[1024];// "name=";// "filename=";
									if (i == 0) {
										sprintf(find, "%s", "name=");
									}
									else if (i == 1) {
										sprintf(find, "%s", "filename=");
									}
									if (strstr(value, find)) {
										char *sat1 = strstr(value, find);

										char name[R7_PAGE_SIZE];
										char *name2;

										sprintf(name, "%s", sat1);
										name2 = name + strlen(find);

										char *sat = strchr(name, '"');
										sprintf(name, "%s", sat);
										name[strlen(name)] = '\0';

										name2 = name + 1;

										char tempName[R7_PAGE_SIZE];// char *tempName[= (char*)malloc(strlen(name2));
										sprintf(tempName, "%s", name2);

										char *sat2 = strchr(name2, '"');
										sprintf(name, "%s", sat2);
										tempName[strlen(tempName) - strlen(name)] = '\0';

										if (i == 0) {
											sprintf(valueName, "%s", tempName);
										}
										else if (i == 1) {

											sprintf(fileName, "%s", tempName);
										}
										//printf("<br>tempName = %s", tempName);
									}
								}
							}
							else if (!strcmp(attr, "Content-Type")) {
								sprintf(contentTypeName, "%s", value);
							}
						}

						if (!attrLen && !valueLen) {
							break;
						}
						else {
							existContent = true;
						}
					}

					if (!attrLen && !valueLen && !existContent) {//end
						break;
					}

					bool hasBinaryfFle = false;
					if (strlen(fileName)) {
						hasBinaryfFle = true;
						memset(postBinaryBuffer, 0, sizeof(char) * contentLength); //initial		
					}

					HANDLE hOutputFile = INVALID_HANDLE_VALUE;
					DWORD dwBytesWrite, dwBytesToWrite;
					dwBytesToWrite = DWORD(1);
					dwBytesWrite = 0;
					if (false) {////debug
						if (hasBinaryfFle) {
							char workSpacePath[R7_STRING_SIZE];
							R7_GetWorkspacePath(r7Sn, workSpacePath, R7_STRING_SIZE);
							wchar_t workSpacePathW[R7_STRING_SIZE];
							memset(workSpacePathW, 0, sizeof(wchar_t) * R7_STRING_SIZE);
							MultiByteToWideChar(CP_UTF8, 0, workSpacePath, -1, workSpacePathW, R7_STRING_SIZE * 2);

							wchar_t fileNameW[R7_STRING_SIZE];
							memset(fileNameW, 0, sizeof(wchar_t) * R7_STRING_SIZE);
							MultiByteToWideChar(CP_UTF8, 0, fileName, -1, fileNameW, R7_STRING_SIZE * 2);
							wchar_t filePathW[R7_STRING_SIZE];
							memset(filePathW, 0, sizeof(wchar_t) * R7_STRING_SIZE);

							wsprintf(filePathW, L"%s%s\0", workSpacePathW, fileNameW);
							hOutputFile = CreateFile(filePathW, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
							if (hOutputFile == INVALID_HANDLE_VALUE) {
								CloseHandle(hOutputFile);
								R7_SetVariableInt(r7Sn, functionSn, 1, -1);
								return -1;
							}
						}
					}

					if (true) {//read value
						boffset = 0;
						sprintf(multipartBoundary, "\r\n--%s", multipartBoundaryTemp);
						boundaryLength = strlen(multipartBoundary);

						char value[R7_PAGE_SIZE];
						memset(value, 0, sizeof(char) * 1);
						valueLen = 0;

						char tempBuffer[1];
						char tempFileBuffer[R7_PAGE_SIZE];
						memset(tempBuffer, 0, sizeof(char) * 1);
						memset(tempFileBuffer, 0, sizeof(char) * R7_STRING_SIZE);
						int tempLen = 0;
						while (1) {
							//readFileErr = fread(buffer, 1, 1, cgiIn);
							readFileErr = ReadFile(hInputFile, buffer, dwInputBytesToWrite, &dwInputBytesWrite, NULL);
							if (!readFileErr) {
								break;
							}
							if (buffer[0] == multipartBoundary[boffset]) {
								tempFileBuffer[boffset] = buffer[0];
								boffset++;
								if (boffset == boundaryLength) {
									break;
								}
							}

							else if (boffset > 0) {
								if (!hasBinaryfFle) {
									value[valueLen++] = *buffer;
								}
								else {
									for (int i = 0; i < boffset; i++) {
										tempBuffer[0] = tempFileBuffer[i];
										postBinaryBuffer[tempLen] = tempBuffer[0];
										//WriteFile(hOutputFile, tempBuffer, dwBytesToWrite, &dwBytesWrite, NULL);//debug
										tempLen++;
									}
								}

								if (buffer[0] == multipartBoundary[0]) {
									boffset = 1;
									tempFileBuffer[0] = '\r';
								}
								else {

									if (hasBinaryfFle) {
										tempBuffer[0] = buffer[0];
										postBinaryBuffer[tempLen] = tempBuffer[0];
										//WriteFile(hOutputFile, tempBuffer, dwBytesToWrite, &dwBytesWrite, NULL);//debug
										boffset = 0;
										tempLen++;
									}
								}
							}
							else {
								if (!hasBinaryfFle) {
									value[valueLen++] = *buffer;
								}
								else {
									postBinaryBuffer[tempLen] = buffer[0];
									//WriteFile(hOutputFile, buffer, dwBytesToWrite, &dwBytesWrite, NULL);//debug
									tempLen++;
								}
							}
						}
						if (hasBinaryfFle) {
							//CloseHandle(hOutputFile);//debug

							//printf("<br> valueName %s<br>", valueName);
							//printf("<br> fileName %s<br>", fileName);

							char * out = (char*)malloc(tempLen);
							memcpy(out, postBinaryBuffer, tempLen);

							//for (int i = 0; i < tempLen; i++) {
							//	printf("<br> out = %c<br>", out[i]);
							//}

							wxString mystring = wxString::FromUTF8(fileName);
							wxString myValue = wxString::FromUTF8(valueName);
							cgiVariables[cgiVariablesLength].name = myValue;
							cgiVariables[cgiVariablesLength].isEnv = 0;
							cgiVariables[cgiVariablesLength].isGet = 0;
							cgiVariables[cgiVariablesLength].isPost = 1;
							cgiVariables[cgiVariablesLength].binaryValue = (char*)out;
							cgiVariables[cgiVariablesLength].binarySize = tempLen;
							cgiVariables[cgiVariablesLength].strValue = mystring;
							cgiVariablesLength++;
						}

						if (sizeof(value)) {
							value[valueLen] = '\0';
						}
						if (!hasBinaryfFle) {
							//printf("<br> result = value <br>%s", value);
							//printf("<br> fileName <br>%s", fileName);

							wxString mystring = wxString::FromUTF8(valueName);
							wxString myValue = wxString::FromUTF8(value);

							cgiVariables[cgiVariablesLength].name = mystring;
							cgiVariables[cgiVariablesLength].strValue = myValue;
							cgiVariables[cgiVariablesLength].isEnv = 0;
							cgiVariables[cgiVariablesLength].isGet = 0;
							cgiVariables[cgiVariablesLength].isPost = 1;
							cgiVariables[cgiVariablesLength].binarySize = 0;
							cgiVariablesLength++;
						}
					}

					//if (!attrLen && !value) {
					//	break;
					//}
				}
				free(contentType);
				free(postBinaryBuffer);
				CloseHandle(hInputFile);
			}
				else {
					HANDLE hDstFile = GetStdHandle(STD_INPUT_HANDLE);
					if (hDstFile == INVALID_HANDLE_VALUE) {
						return -2;
					}

					//get boundary
					wchar_t *contentW = L"CONTENT_TYPE";
					wchar_t *boundaryW;
					boundaryW = _wgetenv(contentW);

					char boundary[R7_STRING_SIZE];
					WideCharToMultiByte(CP_UTF8, 0, boundaryW, -1, boundary, R7_STRING_SIZE * 2, NULL, NULL);

					//find boundary in CONTENT_TYPE
					char findBoundary[R7_STRING_SIZE];
					sprintf(findBoundary, "boundary=");

					char *test = strstr(boundary, findBoundary);
					int address = int(test - boundary);

					//get WebKitFormBoundary
					char webKitFormBoundary[R7_STRING_SIZE];
					strncpy(webKitFormBoundary, boundary + address + int(strlen(findBoundary)), int(strlen("----WebKitFormBoundary") + 16));
					//printf("<br>webKitFormBoundary =%s<br>", webKitFormBoundary);
					char webKitFormBoundary2[R7_STRING_SIZE];
					sprintf(webKitFormBoundary2, "--%s", webKitFormBoundary);

					char *string1;
					DWORD dwBytesToRead, dwBytesRead;
					string1 = (char *)malloc(contentLength + 1);
					memset(string1, 0, sizeof(char) * contentLength + 1);
					dwBytesToRead = contentLength;
					dwBytesRead = 0;

					//to get fileValue
					string getFileChar;
					string checkWebKey(webKitFormBoundary2);
					string totalGet[3];
					bool checkFlag = false;
					bool checkEndFlag = false;
					int checkLine = 0;
					int getValue = 0;

					do {
						ReadFile(hDstFile, string1, 1, &dwBytesRead, NULL);
						if (dwBytesRead == 0) {
							break;
						}

						if (int(string1[0]) == 0) {
							string1[0] = '\0';
						}

						getFileChar.push_back(string1[0]);

						if (getFileChar == checkWebKey && checkFlag == false) {//find boundary
							checkFlag = true;
							getValue = 0;
							getFileChar.clear();
						}
						else if (getFileChar == checkWebKey && checkFlag == true) {//end
							checkEndFlag = true;
						}

						if (string1[0] == '\r' && (checkLine == 0 || checkLine == 2))
						{
							checkLine++;
						}
						else if (string1[0] == '\n' && (checkLine == 1 || checkLine == 3)) {
							checkLine++;

							if (checkLine == 4 || checkLine == 2) {
								//countForNum++;

								if (checkFlag == true && checkEndFlag == false) {
									//first \r\n skip
									if (getValue == 0) {
										getValue++;
									}
									else {
										totalGet[getValue - 1] += getFileChar;
										if (getValue < 3) {
											getValue++;
											if (getValue == 1) {
												totalGet[2].clear();
											}
										}
									}
									getFileChar.clear();

								}
								else if (checkFlag == true && checkEndFlag == true) {
									getFileChar.clear();
									//Analyze the data
									if (totalGet[1].find("Content-Type", 0) != 0) {//string value
										totalGet[0] = totalGet[0].substr((int)strlen("Content-Disposition: form-data; name=\""), totalGet[0].length() - (int)strlen("Content-Disposition: form-data; name=\"") - 3);
										totalGet[2] = totalGet[2].substr(0, totalGet[2].length() - 2);

										wxString stringName(totalGet[0].c_str(), wxConvUTF8);
										wxString stringValue(totalGet[2].c_str(), wxConvUTF8);

										//save data
										cgiVariables[cgiVariablesLength].name = stringName;
										cgiVariables[cgiVariablesLength].strValue = stringValue;
										cgiVariables[cgiVariablesLength].isEnv = 0;
										cgiVariables[cgiVariablesLength].isGet = 0;
										cgiVariables[cgiVariablesLength].isPost = 1;
									}
									else {//File
										totalGet[0] = totalGet[0].substr((int)strlen("Content-Disposition: form-data; name=\""), totalGet[0].length() - (int)strlen("Content-Disposition: form-data; name=\"") - 3);
										int findM = totalGet[0].find("\"; filename=\"", 0);

										totalGet[1] = totalGet[0].substr(findM + (int)strlen("\"; filename=\""), totalGet[0].length() - findM - (int)strlen("\"; filename=\""));
										totalGet[0] = totalGet[0].substr(0, findM);
										totalGet[2] = totalGet[2].substr(2, totalGet[2].length() - 4);

										string useCheck = totalGet[2];

										char* dataBinary = new char[useCheck.length()];
										copy(useCheck.begin(), useCheck.end(), dataBinary);

										wxString stringName(totalGet[0].c_str(), wxConvUTF8);
										wxString stringFileName(totalGet[2].c_str(), wxConvUTF8);

										//save data
										cgiVariables[cgiVariablesLength].name = stringName;
										cgiVariables[cgiVariablesLength].strValue = stringFileName;
										cgiVariables[cgiVariablesLength].isEnv = 0;
										cgiVariables[cgiVariablesLength].isGet = 0;
										cgiVariables[cgiVariablesLength].isPost = 1;
										cgiVariables[cgiVariablesLength].binaryValue = (char*)dataBinary;
										cgiVariables[cgiVariablesLength].binarySize = (int)useCheck.length();
									}

									//test
									for (int i = 0; i < 3; i++) {
										//printf("value%d =%s<br>", i, totalGet[i].c_str());
										totalGet[i].clear();
									}

									checkFlag = true;
									checkEndFlag = false;
									getValue = 1;
									cgiVariablesLength++;
								}
								checkLine = 0;
							}
						}
						else
						{
							checkLine = 0;
						}


					} while (dwBytesToRead > 0);

					free(string1);
					CloseHandle(hDstFile);
				}
			continue;
		}

		//env
		else {
			wxString mystring = wxString::FromUTF8(envVariable);
			wxString myValue = wxString::FromUTF8(getEnvVariable);
			//wprintf(L"result = %s", envVariable[i]);
			//printf("<br> mystring  =%s", getEnvVariable);
			cgiVariables[cgiVariablesLength].name = mystring;
			cgiVariables[cgiVariablesLength].strValue = myValue;
			cgiVariables[cgiVariablesLength].isEnv = 1;
			cgiVariables[cgiVariablesLength].isGet = 0;
			cgiVariables[cgiVariablesLength].isPost = 0;
			cgiVariables[cgiVariablesLength].binarySize = 0;
			cgiVariablesLength++;
		}
	}
	R7_SetVariableInt(r7Sn, functionSn, 1, result);
	return 1;
}

static int CGI_Print(int r7Sn, int functionSn) {
	int res;
	void *variableObject = NULL;

	res = R7_GetVariableObject(r7Sn, functionSn, 1, &variableObject);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableObject = %d", res);
		return -2;
	}

	return 1;
}


static int CGI_PrintBinary(int r7Sn, int functionSn) {

	int binarySize = R7_GetVariableBinarySize(r7Sn, functionSn, 1);
	char *binary = (char *)malloc(binarySize);
	R7_GetVariableBinary(r7Sn, functionSn, 1, binary, binarySize);

	int firstAscii = int(binary[0]);
	int secondAscii = int(binary[1]);
	if (firstAscii < 0) {
		firstAscii = 256 + firstAscii;
	}
	if (secondAscii < 0) {
		secondAscii = 256 + secondAscii;
	}
	//printf("%s", " ");
	//printf("%d", firstAscii);
	//printf("%d", secondAscii);
	//return 1;

	if (firstAscii == 255 && secondAscii == 216) {//JPG = 255216
		printf("Content-Type: image/jpeg\r\n\r\n");
	}
	else if (firstAscii == 71 && secondAscii == 73) {//GIF = 7173,

	}
	else if (firstAscii == 66 && secondAscii == 77) {//BMP = 6677,
		printf("Content-Type: image/bmp\r\n\r\n");
	}
	else if (firstAscii == 137 && secondAscii == 80) {//PNG = 13780,
		printf("Content-Type: image/png\r\n\r\n");

	}
	else if (firstAscii == 67 && secondAscii == 87) {//SWF = 6787,

	}
	else if (firstAscii == 82 && secondAscii == 97) {//RAR = 8297,

	}
	else if (firstAscii == 80 && secondAscii == 75) {//ZIP = 8075,   excel  8075
		printf("Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet\r\n\r\n");
	}
	else if (firstAscii == 55 && secondAscii == 122) {// _7Z = 55122,

	}
	else if ((firstAscii == 102 && secondAscii == 100) || (firstAscii == 60 && secondAscii == 33) || (firstAscii == 239 && secondAscii == 187)) {//TXT = 102100,  //HTML = 6033,  text: 239 187
		printf("Content-type: text/html\r\n\r\n");
	}
	else if (firstAscii == 37 && secondAscii == 80) {//PDF = 3780,
		printf("Content-Type: application/pdf\r\n\r\n");
	}
	else if (firstAscii == 208 && secondAscii == 207) {//208207, ppt
		printf("Content-Type: application/vnd.ms-powerpoint\r\n\r\n");
	}
	else if (firstAscii == 80 && secondAscii == 75) {//doc = 8075,
		printf("Content-Type:application/vnd.openxmlformats-officedocument.wordprocessingml.document\r\n\r\n");
	}
	else if (firstAscii == 207 && secondAscii == 208) {//XLS = 207208,

	}
	else if (firstAscii == 73 && secondAscii == 84) {//CHM = 7384

	}
	else if (firstAscii == 60 && secondAscii == 63) {//XML = 6063,

	}
	else if (firstAscii == 239 && secondAscii == 187) {//ASPX = 239187,,
		printf("Content-type:application/octet-stream\r\n\r\n");//r6
	}
	else if (firstAscii == 117 && secondAscii == 115) {//CS = 117115,

	}
	else if (firstAscii == 119 && secondAscii == 105) {//JS = 119105,

	}
	else if (firstAscii == 255 && secondAscii == 254) {//JS = 255254,

	}
	else {//Content-Type:text/html; charset=utf-8
		printf("Content-type: text/html; charset=utf-8\r\n\r\n");
	}
	
	for (int i = 0; i < binarySize; i++) {
		printf("%c", binary[i]);
	}


	//HANDLE hOutputFile = INVALID_HANDLE_VALUE;
	//DWORD dwBytesWrite, dwBytesToWrite;
	//dwBytesToWrite = DWORD(1);
	//dwBytesWrite = 0;
	//if (true) {////debug
	//	if (true) {


	//		hOutputFile = CreateFile(L"C:\\Users\\kelly82\\Desktop\\新增資料夾\\hihihi.jpg", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//		if (hOutputFile == INVALID_HANDLE_VALUE) {
	//			CloseHandle(hOutputFile);
	//			R7_SetVariableInt(r7Sn, functionSn, 1, -1);
	//			return -1;
	//		}
	//	}

	//	char d[1];
	//		
	//	for (int i = 0; i < binarySize; i++) {
	//		d[0] = binary[i];
	//		WriteFile(hOutputFile, d, dwBytesToWrite, &dwBytesWrite, NULL);//debug
	//	}

	//	CloseHandle(hOutputFile);
	//}

	return 1;
}


static int CGI_PrintInfo(int r7Sn, int functionSn) {
	int i = 1;	//extern wchar_t **_wenviron;
	wchar_t *s = *_wenviron;

	for (; s; i++) {
		char nameGet[R7_STRING_SIZE];
		WideCharToMultiByte(CP_UTF8, 0, s, -1, nameGet, R7_STRING_SIZE * 2, NULL, NULL);
		R7_Printf(r7Sn, "%s<br>", nameGet);
		s = *(_wenviron + i);
	}

	return 1;
}


static int CGI_Println(int r7Sn, int functionSn) {
	int res;
	void *variableObject = NULL;

	res = R7_GetVariableObject(r7Sn, functionSn, 1, &variableObject);
	if (res <= 0) {
		R7_Printf(r7Sn, "ERROR! R7_GetVariableObject = %d", res);
		return -2;
	}

	return 1;
}


R7_API int R7Library_Init(void) {
	// Register your functions in this API.

	R7_RegisterFunction("CGI_GetPostFile", (R7Function_t)&CGI_GetPostFile);
	R7_RegisterFunction("CGI_GetEnvString", (R7Function_t)&CGI_GetEnvString);
	R7_RegisterFunction("CGI_GetGetString", (R7Function_t)&CGI_GetGetString);
	R7_RegisterFunction("CGI_GetPostString", (R7Function_t)&CGI_GetPostString);
	R7_RegisterFunction("CGI_ParseRequest", (R7Function_t)&CGI_ParseRequest);
	R7_RegisterFunction("CGI_Print", (R7Function_t)&CGI_Print);
	R7_RegisterFunction("CGI_PrintBinary", (R7Function_t)&CGI_PrintBinary);
	R7_RegisterFunction("CGI_PrintInfo", (R7Function_t)&CGI_PrintInfo);
	R7_RegisterFunction("CGI_Println", (R7Function_t)&CGI_Println);

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
	json_object_set_new(functionGroupObject, "name", json_string("CGI"));
	json_array_append(functionGroupArray, functionGroup);

	functionArray = json_array();
	json_object_set_new(functionGroupObject, "functions", functionArray);

	// CGI_GetPostFile
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("CGI_GetPostFile"));
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
	json_object_set_new(variableObject, "name", json_string("binary"));
	json_object_set_new(variableObject, "type", json_string("Binary"));
	json_object_set_new(variableObject, "direction", json_string("OUT"));
	json_array_append(variableArray, variable);
	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string("name"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "direction", json_string("IN"));
	json_array_append(variableArray, variable);

	//CGI_GetEnvString
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("CGI_GetEnvString"));
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
	json_object_set_new(variableObject, "name", json_string("string"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "direction", json_string("OUT"));
	json_array_append(variableArray, variable);
	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string("name"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "direction", json_string("IN"));
	json_array_append(variableArray, variable);

	//CGI_GetGetString
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("CGI_GetGetString"));
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
	json_object_set_new(variableObject, "name", json_string("string"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "direction", json_string("OUT"));
	json_array_append(variableArray, variable);
	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string("name"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "direction", json_string("IN"));
	json_array_append(variableArray, variable);

	//CGI_GetPostString
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("CGI_GetPostString"));
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
	json_object_set_new(variableObject, "name", json_string("string"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "direction", json_string("OUT"));
	json_array_append(variableArray, variable);
	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string("name"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "direction", json_string("IN"));
	json_array_append(variableArray, variable);

	// CGI_ParseRequest
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("CGI_ParseRequest"));
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

	// CGI_Print
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("CGI_Print"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);
	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string("string"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "direction", json_string("IN"));
	json_array_append(variableArray, variable);

	// CGI_PrintBinary
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("CGI_PrintBinary"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);
	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string("binary"));
	json_object_set_new(variableObject, "type", json_string("Binary"));
	json_object_set_new(variableObject, "direction", json_string("IN"));
	json_array_append(variableArray, variable);

	// CGI_PrintInfo
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("CGI_PrintInfo"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);

	// CGI_Println
	function = json_object();
	functionObject = json_object();
	json_object_set_new(function, "function", functionObject);
	json_object_set_new(functionObject, "name", json_string("CGI_Println"));
	json_object_set_new(functionObject, "doc", json_string(""));
	json_array_append(functionArray, function);
	variableArray = json_array();
	json_object_set_new(functionObject, "variables", variableArray);
	variableObject = json_object();
	variable = json_object();
	json_object_set_new(variable, "variable", variableObject);
	json_object_set_new(variableObject, "name", json_string("string"));
	json_object_set_new(variableObject, "type", json_string("String"));
	json_object_set_new(variableObject, "direction", json_string("IN"));
	json_array_append(variableArray, variable);


	sprintf_s(str, strSize, "%s", json_dumps(root, 0));

	json_decref(root);

	return 1;
}

#ifdef __cplusplus
}
#endif
