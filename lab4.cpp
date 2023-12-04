#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <fstream>
#include <thread>
#include <algorithm>
#include <vector>
#include <stdio.h>
#include "ThreadPool.h"
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <string>

#define BUF_SIZE 4096
std::vector<std::string> data;
std::vector<std::vector<std::string>> sep_data;
LPCSTR pBuf;
LPCSTR pBuf1;

bool GetDataFromFile()
{
	std::string filename = "SampleData.txt";

	OFSTRUCT fileInfo;

	HFILE file = OpenFile(filename.c_str(), &fileInfo, OF_READWRITE);

	DWORD fileSize = GetFileSize((HANDLE)file, NULL);

	HANDLE hMapFile = CreateFileMappingA((HANDLE)file, NULL, PAGE_READWRITE, 0, fileSize, NULL);
	if (hMapFile == NULL) {
		std::cout << "Can't create file mapping" << std::endl;
		return false;
	}

	pBuf = (LPCSTR)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, fileSize);

	if (pBuf == NULL) {
		std::cout << "Can't get mapped file" << std::endl;
		return false;
	}
	char* fileContent = (char*)calloc(fileSize+1, sizeof(char));

	if (fileContent == NULL) {
		std::cout << "Can't allocate memory for file content" << std::endl;
		return false;
	}
	strncpy(fileContent, pBuf, fileSize);
	fileContent[fileSize] = '\0';

	CopyMemory((PVOID)pBuf, "\0", 1);

	char* token = strtok(fileContent, "\r\n");
	int readBytes = strlen(token) + 2;
	while (token != NULL && readBytes <= fileSize) {
		data.push_back(std::string(token));
		token = strtok(NULL, "\r\n");
		if (token != NULL) 
		{
			readBytes += strlen(token);
		}
	}

	CloseHandle((HANDLE)file);
	CloseHandle(hMapFile);

	return true;
}

short GetThreadCapacity()
{
	short threadsCount;
	std::cout << "enter threads count - ";
	std::cin >> threadsCount;
	std::cout << "\n";
	if (threadsCount > data.size() || threadsCount <= 0) {
		std::cout << "Incorrect amount of threads" << std::endl;
		return -1;
	}
	return threadsCount;
}

void SortPiece(void* arr_piece)
{
	std::vector<std::string>* obj = (std::vector<std::string>*)arr_piece;
	std::sort(obj->begin(), obj->end());
}

struct TArrEl
{
	std::string str;
	int arrIndex;
	int elIndex;
};

typedef struct TArrEl ArrEl;

ArrEl FindMinEl(std::vector<ArrEl> &arr) {
	ArrEl tmpMinEl = arr[0];
	int elInd = 0;

	for (int i = 1; i < arr.size(); i++) {
		if (tmpMinEl.str.compare(arr[i].str) > 0) {
			tmpMinEl = arr[i];
			elInd = i;
		}
	}
	std::vector<ArrEl>::iterator it = arr.begin();
	std::advance(it, elInd);
	arr.erase(it);

	return tmpMinEl;
}



void MergeSort() {

	std::vector<ArrEl> tmpVector;

	data.clear();
	int maxStringCount = sep_data[0].size();
	int curStringInd = 0;
	int blocksCount = sep_data.size();

	for (int i = 0; i < maxStringCount; i++) {

		for (int j = 0; j < blocksCount; j++) {
			if (curStringInd >= sep_data[j].size()) { continue; }

			ArrEl arrEl;
			arrEl.str = sep_data[j][curStringInd];
			arrEl.arrIndex = j;
			arrEl.elIndex = curStringInd;
			tmpVector.push_back(arrEl);
		}
		curStringInd++;

		int size = tmpVector.size();

		while (tmpVector.size() != 0) {
			ArrEl minEl = FindMinEl(tmpVector);
			data.push_back(minEl.str);

			if (sep_data[minEl.arrIndex].size() - 1 >= minEl.elIndex + 1) {
				ArrEl arrEl;
				arrEl.str = sep_data[minEl.arrIndex][minEl.elIndex + 1];
				arrEl.arrIndex = minEl.arrIndex;
				arrEl.elIndex = minEl.elIndex + 1;
				tmpVector.push_back(arrEl);
			}
		}

		break;
	}

}

std::string CorrectSpaces(std::string str) {
	std::string delimiter = " ";
	size_t pos = 0;
	std::string token;

	std::string finalStr = "";
	while ((pos = str.find(delimiter)) != std::string::npos) {
		token = str.substr(0, pos);
		if (token != "") {
			finalStr += (token + " ");
		}
		str.erase(0, pos + delimiter.length());
	}
	finalStr += str;
	return finalStr;
}

void CreateTasks(std::vector<std::string> indata, short quantity, TaskQueue* taskQueue)
{
	bool isEven = indata.size() % quantity == 0;
	int standardStringCapacity = indata.size() / quantity;
	if (!isEven)
		standardStringCapacity = indata.size() / quantity + 1;
	int globalCounter = 0;
	for (int i = 0; i < quantity; i++)
	{
		sep_data[i].clear();
		int tmpCounter = 0;
		while (standardStringCapacity != tmpCounter)
		{
			if (globalCounter == indata.size())
				break;
			std::string str = CorrectSpaces(indata[globalCounter]);
			if (str != "") {
				sep_data[i].push_back(str);
			}
			globalCounter++;
			tmpCounter++;
		}
		Task* newtask = new Task(&SortPiece, (void*)& sep_data[i]);
		taskQueue->Enqueue(newtask);
	}
}

int main()
{
	if (!GetDataFromFile())
	{
		system("pause");
		return -1;
	}

	short threadsCount = GetThreadCapacity();
	if (threadsCount == -1)
	{
		std::cout << "Invalid thread count value." << std::endl;
		system("pause");
		return -1;
	}
	sep_data.resize(threadsCount);
	TaskQueue* taskqueue = new TaskQueue();
	CreateTasks(data, threadsCount, taskqueue);
	data.clear();
	ThreadPool* threads = new ThreadPool(threadsCount, taskqueue);
	threads->WaitAll();
	delete(threads);
	delete(taskqueue);

	MergeSort();

	std::string joinedStr = "";
	int dataCount = data.size();
	for (int i = 0; i < dataCount; i++) {
		if (i == dataCount - 1) {
			joinedStr += data[i];
			break;
		}
		joinedStr += data[i] + "\n";
	}

	CopyMemory((PVOID)pBuf, joinedStr.c_str(), strlen(joinedStr.c_str()) * sizeof(char));
	data.clear();
	UnmapViewOfFile(pBuf);
	system("pause");
	return 0;
}
