// PDS_lab3_thread.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <windows.h>
#include <process.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include <regex>
#include <fstream>
#include <exception>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <array>

#define MAX_THREAD 20
#define MAX_ARGS 100




typedef struct {
	TCHAR  fileName[MAX_PATH];
	TCHAR  search[MAX_PATH];
	DWORD result;
}ARGS;

void ErrorExit(LPTSTR lpszFunction);
void _wrapperFgrep(LPTSTR szDir, LPTSTR search, std::vector<HANDLE> &threadHandles, std::vector<ARGS> &args);
void _Fgrep(void *args);

int _tmain(int argc, _TCHAR* argv[])
{

	std::vector<ARGS> args;
	std::vector<HANDLE> threadHandles;
	std::wfstream results;

	if (argc != 4){
		_tprintf(_T("error on parameters.\n"));
		exit(-1);
	}


	_wrapperFgrep(argv[1], argv[2], threadHandles, args);

	

	/*if (WaitForMultipleObjects(threadHandles.size(), threadHandles.data(), TRUE, INFINITE) == WAIT_FAILED){
		ErrorExit(_T("pippo"));
		exit(-1);
	}*/

	while (threadHandles.size() > 0){
		WaitForSingleObject(threadHandles.back(), INFINITE);
		threadHandles.pop_back();
	}

	try{
		results.open(argv[3], std::wfstream::out);
	}
	catch (std::exception& e){
		std::cout << e.what() << std::endl;
		exit(-2);
	}

	if (!results.is_open()){
		std::cout << "errore\n";
		exit(-1);
	}


	int i;

	for (DWORD i = 0; i < args.size(); i++){
		if (args[i].result == 1)
			results << "File name: " << args[i].fileName << " - stringa presente." << std::endl;
		else
			results << "File name: " << args[i].fileName << "."<< std::endl;

	}

	//print results

	results.close();


	system("pause");


	return 0;
}

void _wrapperFgrep(LPTSTR szDir, LPTSTR search, std::vector<HANDLE> &threadHandles, std::vector<ARGS> &args){

	HANDLE hFind = INVALID_HANDLE_VALUE;
	LARGE_INTEGER filesize;
	WIN32_FIND_DATA ffd;
	WCHAR directoryName[MAX_PATH + 2], handleDirectory[MAX_PATH + 2], handleFile[MAX_PATH + 2];
	int i;

	StringCchCopy(directoryName, MAX_PATH, szDir);
	StringCchCat(directoryName, MAX_PATH, TEXT("\\*"));
	hFind = FindFirstFile(directoryName, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		std::cout << TEXT("Invalid Handle value\n");
		exit(-1);
	}

	// List all the files in the directory with some info about them.

	do
	{
		if (_tcscmp(ffd.cFileName, _T(".")) != 0 && _tcscmp(ffd.cFileName, _T("..")) != 0){

			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				//ricorsivo sulle sottocartelle
				StringCchCopy(handleDirectory, MAX_PATH, szDir);
				StringCchCat(handleDirectory, MAX_PATH, _T("\\"));
				StringCchCat(handleDirectory, MAX_PATH, ffd.cFileName);
				_wrapperFgrep(handleDirectory, search, threadHandles, args);
			}
			else
			{
				//controllo all'interno del file
				StringCchCopy(handleFile, MAX_PATH, szDir);
				StringCchCat(handleFile, MAX_PATH, _T("\\"));
				StringCchCat(handleFile, MAX_PATH, ffd.cFileName);

				if (threadHandles.size()>= MAX_THREAD){
					DWORD threadIndex;
					WaitForSingleObject(threadHandles.back(), INFINITE);
					threadHandles.pop_back();
					/*if ((threadIndex = WaitForMultipleObjects(threadHandles.size(),threadHandles.data() , TRUE, INFINITE)) == WAIT_FAILED){
						ErrorExit(_T("pippo"));
						exit(-1);
					}*/
					// remove thread handle threadHandles.pop_back();
					//threadHandles.erase(threadHandles.begin() + (threadIndex - WAIT_OBJECT_0));
				}

				ARGS a;
				StringCchCopy(a.fileName, MAX_PATH, handleFile);
				StringCchCopy(a.search, MAX_PATH, search);
				a.result = 0;
				args.push_back(a);

				threadHandles.push_back((HANDLE)_beginthread(_Fgrep, 0, (void *)&args.back()));

			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	//_ftprintf(fp,TEXT("Cartella finita\n"));

	FindClose(hFind);

	return ;
};

//grep con FILENAME TCHAR

void _Fgrep(void *args){

	ARGS *FGArgs = (ARGS *) args;
	std::wfstream inStream;
	int find = 0;
	std::wstring buff;

	try{
		inStream.open(FGArgs->fileName, std::wfstream::in);
	}
	catch (std::exception& e){
		std::cout << e.what() << std::endl;
		exit(-3);
	}

	while (std::getline(inStream, buff)){
		if ((buff.find(FGArgs->search)) != std::string::npos){
			inStream.close();
			FGArgs->result = 1;
			return ;
		}
	}
	
	inStream.close();
	FGArgs->result = 0;
	return ;
}


void ErrorExit(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	//LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	/*lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));*/
	/*StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);*/
	std::cout << (LPTSTR)lpMsgBuf << std::endl;
	//MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	//LocalFree(lpDisplayBuf);
	return;
}
