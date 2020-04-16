// dict.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <fstream>
#include "json.hpp"

#include <io.h>
#include <fcntl.h>
#include "convert.h"

#pragma comment(lib, "winhttp.lib")


using json = nlohmann::json;


std::string dictWork(const std::wstring word) {
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(L"WinHTTP Example/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);


	std::wstring site = L"www.iciba.com";



	if (hSession)
		hConnect = WinHttpConnect(hSession, site.c_str(),
			INTERNET_DEFAULT_HTTPS_PORT, 0);


	std::wstring fileQuery = L"index.php?callback=data&a=getWordMean&c=search&list=&word=" + word;
	// std::wcout << site << std::endl << fileQuery << std::endl;

	// Create an HTTP request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", fileQuery.c_str(),
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE);

	// Send a request.
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0, WINHTTP_NO_REQUEST_DATA, 0,
			0, 0);


	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);


	std::string contentBuffer = "";


	if (bResults)
	{
		do
		{
			// Check for available data.
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				printf("Error %u in WinHttpQueryDataAvailable.\n",
					GetLastError());
				break;
			}

			// No more available data.
			if (!dwSize)
				break;

			// Allocate space for the buffer.
			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer)
			{
				printf("Out of memory\n");
				break;
			}

			// Read the Data.
			ZeroMemory(pszOutBuffer, dwSize + 1);

			if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
				dwSize, &dwDownloaded))
			{
				printf("Error %u in WinHttpReadData.\n", GetLastError());
			}
			else
			{
				//printf("%s", pszOutBuffer);

				contentBuffer.append(pszOutBuffer);
			}


			// Free the memory allocated to the buffer.
			delete[] pszOutBuffer;

			if (!dwDownloaded)
				break;

		} while (dwSize > 0);
	}
	else
	{
		// Report any errors.
		printf("Error %d has occurred.\n", GetLastError());
	}

	int sEnd = contentBuffer.length() - 6;
	std::string jsonBuffer = contentBuffer.substr(5, sEnd);

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	std::ofstream outfile("out.txt", std::ofstream::out);
	outfile << jsonBuffer;

	return jsonBuffer;
}


int main(int argc, char** argv)
{
	_setmode(_fileno(stdout), _O_U16TEXT);


	wchar_t* mWord = (wchar_t*)malloc(100* sizeof(wchar_t));
	mbtowc(mWord, argv[1], MB_CUR_MAX);

	std::wstring word = mWord;

	std::wcout << word;


	std::ifstream infile("out.txt", std::ifstream::in);
	std::string bi = "";

	char ddd;

	while (!infile.eof()) {
		infile.get(ddd);
		bi.push_back(ddd);
	}
	bi.pop_back();
	auto myWord = dictWork(word.c_str());

	try {

		json j3 = json::parse(myWord);


		auto parts = j3["baesInfo"]["symbols"][0]["parts"];
		auto result = j3["baesInfo"]["translate_result"];

		for (json::iterator it = parts.begin(); it != parts.end(); ++it) {
			for (auto x : it.value()["means"]) {
				std::wcout << Utf16FromUtf8(x) << " ";
			}
			std::wcout << Utf16FromUtf8(it.value()["part"]) << "\n";
		}


		/*	for (json::iterator it = j3.begin(); it != j3.end(); ++it) {
				std::cout << it.key() << " : " << it.value() << "\n";
			}*/

	}
	catch (const std::exception& e) {
		std::wcout << L"parse error:" << e.what();
	}



}
