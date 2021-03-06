// dllmain.cpp : Defines the entry point for the DLL application.
#pragma once
#include <Windows.h>
#include <sys\stat.h>
#include <filesystem>
#include <string> 
#include <sstream>

namespace fs = std::experimental::filesystem;

uintptr_t BaseAddress;
const char* PluginFolder = "Plugins\\";
char workingDir[MAX_PATH];
std::vector<std::string> dllList;

std::stringstream sstream;

void LogToFile(const std::string &text)
{
	FILE* pFile = fopen("ddraw.log", "a");
	fprintf(pFile, "%s\n", text.c_str());
	fclose(pFile);
}

bool GetWorkingDirectory() 
{
	HMODULE hModule = GetModuleHandle(NULL);
	if (hModule) {
		GetModuleFileNameA(hModule, workingDir, MAX_PATH);
		std::string folder;
		size_t index;
		index = std::string(workingDir).find_last_of("\\") + 1;
		if (index != std::string::npos) {
			folder = std::string(workingDir).replace(index, std::string(workingDir).length() - index, "");
			strcpy(workingDir, folder.c_str());
		}
		return true;
	}
	return false;
}

struct stat info;
bool CheckAndReadPluginFolder() 
{
	if (stat(std::string(workingDir).append(PluginFolder).c_str(), &info) == 0) 
	{
		if (info.st_mode & S_IFDIR) {
			for (auto &p : fs::directory_iterator(std::string(workingDir).append(PluginFolder))) 
			{
				if((p.path()).string().substr((p.path()).string().find_last_of(".") + 1) == "dll")
					dllList.push_back((p.path()).string());
			}
			for (auto &dll : dllList) {
				HANDLE library = NULL;
				library = LoadLibraryA(dll.c_str());

				// If Library failed to load, log it to file
				if (library == NULL) {
					 sstream << "Failed to load DLL: " << dll.c_str() << " with errorcode: " << GetLastError() << '\n';
					 LogToFile(sstream.str());
					 sstream.str() = std::string();
				}
			}				
			return true;
		}
	}
	return false;
}

bool LoadPlugins() 
{
	if (GetWorkingDirectory() && CheckAndReadPluginFolder()) {
		return true;
	}
	return false;
}

DWORD WINAPI MainThread(LPVOID param)
{
	return LoadPlugins();
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)MainThread, hModule, NULL, nullptr);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

