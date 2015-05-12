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


int _wrapperFgrep(LPTSTR szDir, int tab, std::wfstream &outStream, LPTSTR search);
int _Fgrep(TCHAR * fileHandle, TCHAR * search);

int _tmain(int argc, _TCHAR* argv[])
{
	int result, tab = 0;
	std::wfstream results;

	if (argc != 4){
		_tprintf(_T("error on parameters.\n"));
		exit(-1);
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


	//StringCchCopy(szDir, MAX_PATH, argv[1]);

	result = _wrapperFgrep(argv[1], tab, results, argv[2]);

	results.close();


	system("pause");


	return 0;
}

int _wrapperFgrep(LPTSTR szDir, int tab, std::wfstream &outStream, LPTSTR search){

	HANDLE hFind = INVALID_HANDLE_VALUE;
	LARGE_INTEGER filesize;
	WIN32_FIND_DATA ffd;
	WCHAR directoryName[MAX_PATH + 2], handleDirectory[MAX_PATH + 2], handleFile[MAX_PATH + 2];
	int i, result;

	StringCchCopy(directoryName, MAX_PATH, szDir);
	StringCchCat(directoryName, MAX_PATH, TEXT("\\*"));
	hFind = FindFirstFile(directoryName, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		outStream << TEXT("Invalid Handle value\n");
		return -1;
	}

	// List all the files in the directory with some info about them.

	do
	{
		if (_tcscmp(ffd.cFileName, _T(".")) != 0 && _tcscmp(ffd.cFileName, _T("..")) != 0){

			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				for (i = 0; i<tab; i++){
					outStream << TEXT("\t");
				}

				outStream << ffd.cFileName << TEXT(" <DIR>\n");
				//ricorsivo sulle sottocartelle
				StringCchCopy(handleDirectory, MAX_PATH, szDir);
				StringCchCat(handleDirectory, MAX_PATH, _T("\\"));
				StringCchCat(handleDirectory, MAX_PATH, ffd.cFileName);
				_wrapperFgrep(handleDirectory, tab + 1, outStream, search);
			}
			else
			{
				for (i = 0; i<tab; i++){
					outStream << TEXT("\t");
				}

				filesize.LowPart = ffd.nFileSizeLow;
				filesize.HighPart = ffd.nFileSizeHigh;
				//controllo all'interno del file
				StringCchCopy(handleFile, MAX_PATH, szDir);
				StringCchCat(handleFile, MAX_PATH, _T("\\"));
				StringCchCat(handleFile, MAX_PATH, ffd.cFileName);

				result = _Fgrep(handleFile, search);

				if (result){
					outStream << ffd.cFileName << _T(" ") << filesize.QuadPart << TEXT(" bytes ") << TEXT("Stringa ricercata presente.\n");
				}
				else{
					outStream << ffd.cFileName << _T(" ") << filesize.QuadPart << TEXT(" bytes\n");
				}
			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	//_ftprintf(fp,TEXT("Cartella finita\n"));

	FindClose(hFind);

	return 0;
};

//grep con FILENAME TCHAR

int _Fgrep(TCHAR * fileName, TCHAR * search){

	std::wfstream inStream;
	int find = 0;
	std::wstring buff;


	try{
		inStream.open(fileName, std::wfstream::in);
	}
	catch (std::exception& e){
		std::cout << e.what() << std::endl;
		exit(-3);
	}

	while (std::getline(inStream, buff)){
		if ((buff.find(search)) != std::string::npos){
			inStream.close();
			return 1;
		}
	}


	inStream.close();

	return 0;
}
