#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <typeinfo>

#include <SMObject.h>
#include <SMStructs.h>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;
using namespace std;
#define NUM_UNITS 5

void SetupSharedMemory(ProcessManagement* ProcessManagementPtr);
bool StartProcess(TCHAR* processName, STARTUPINFO* startupInfo, PROCESS_INFORMATION* processInfo);
bool KillProcess(TCHAR* processName, HANDLE processHandle);
bool IsProcessRunning(const char* processName);

//Definition of the start up sequences
TCHAR Units[5][30] =
{
	TEXT("Camera_Module.exe"),        // 0
	TEXT("GPS_Module.exe"),           // 1
	TEXT("LASER_Module.exe"),         // 2
	TEXT("Display_Module.exe"),       // 3
	TEXT("VehicleControl_Module.exe") // 4
};

int main()
{
	/*
		Shared Memory, Only Process_Mangement_SM need to do extra process in this .cpp
	*/
	SMObject ProcessManagementObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	ProcessManagementObj.SMCreate();// Build up the handle of SMO
	ProcessManagementObj.SMAccess();// Open the existed handle of SMO
	ProcessManagement* ProcessManagementPtr = (ProcessManagement*)ProcessManagementObj.pData;

	// Do not shut down any processes at the startup of the program
	ProcessManagementPtr->Shutdown.Flags.Camera = 0b0;
	ProcessManagementPtr->Shutdown.Flags.GPS = 0b0;
	ProcessManagementPtr->Shutdown.Flags.Laser = 0b0;
	ProcessManagementPtr->Shutdown.Flags.OpenGL = 0b0;
	ProcessManagementPtr->Shutdown.Flags.ProcessManagement = 0b0;
	ProcessManagementPtr->Shutdown.Flags.VehicleControl = 0b0;

	SMObject SM_GPSObj(_TEXT("SM_GPS"), sizeof(SM_GPS));
	SM_GPSObj.SMCreate();
	SM_GPSObj.SMAccess();

	SMObject SM_LaserObj(_TEXT("SM_Laser"), sizeof(SM_Laser));
	SM_LaserObj.SMCreate();
	SM_LaserObj.SMAccess();

	SMObject SM_VehicleControlObj(_TEXT("SM_VehicleControl"), sizeof(SM_VehicleControl));
	SM_VehicleControlObj.SMCreate();
	SM_VehicleControlObj.SMAccess();


	//start all 5 modules
	STARTUPINFO startupinfos[NUM_UNITS];//Specifies appearance of the main window etc. for a process at creation time.
	PROCESS_INFORMATION processinfos[NUM_UNITS];//Receives identification information about the new process.
	for (int i = 0; i < NUM_UNITS; i++)
	{
		StartProcess(Units[i], &startupinfos[i], &processinfos[i]);
	}

	//if (ProcessManagementPtr->Shutdown.Flags.Laser == 0b1)
	//{
	//	KillProcess(Units[2], processinfos[2].hProcess);
	//	StartProcess(Units[2], &startupinfos[2], &processinfos[2]);
	//}

	Sleep(1000);// Wait for 1s to allow processes to fully start up

	int cnt_Laser = 0;
	int cnt_GPS = 0;
	int cnt_OpenGL = 0;
	int cnt_VehicleControl = 0;
	int cnt_Camera = 0;
	int MAX_WAITBEAT = 100;
	// main loop
	while (1)
	{
		// Press Esc || PM Module break down  to break 
		if ((_kbhit() && _getch() == 27) || ProcessManagementPtr->Shutdown.Flags.ProcessManagement == 0b1) break; //You can set it to 0b1 in other module's exceptions to shutdown the whole PM process, which means it's a cross-level operation.

		/* Heartbear check, need simplification*/
		if (ProcessManagementPtr->Shutdown.Flags.Laser == 0b1)
		{
			KillProcess(Units[2], processinfos[2].hProcess);
			StartProcess(Units[2], &startupinfos[2], &processinfos[2]);
		}
		else if (ProcessManagementPtr->Heartbeat.Flags.Laser == 0b1) // Heartbeat still
		{
			ProcessManagementPtr->Heartbeat.Flags.Laser = 0b0; // Clear for next heartbeat checking
			cnt_Laser = 0;
		}
		else if (ProcessManagementPtr->Heartbeat.Flags.Laser == 0b0) // Heartbeat stop
		{
			cnt_Laser++;
			if (cnt_Laser > MAX_WAITBEAT)
			{
				ProcessManagementPtr->Shutdown.Status = 0xFF; // Critical laser module is broken, all modules need to be shutdown.
				std::cout << "Critical laser module is broken." << std::endl;
				continue;
			}
		}

		if (ProcessManagementPtr->Shutdown.Flags.Camera == 0b1)
		{
			KillProcess(Units[0], processinfos[0].hProcess);
			StartProcess(Units[0], &startupinfos[0], &processinfos[0]);
		}
		else if (ProcessManagementPtr->Heartbeat.Flags.Camera == 0b1)
		{
			ProcessManagementPtr->Heartbeat.Flags.Camera = 0b0;
			cnt_Camera = 0;
		}
		else if (ProcessManagementPtr->Heartbeat.Flags.Camera == 0b0)
		{
			cnt_Camera++;
			if (cnt_Camera > MAX_WAITBEAT)
			{
				ProcessManagementPtr->Shutdown.Status = 0xFF;
				std::cout << "Critical camera module is broken." << std::endl;
				continue;
			}
		}

		if (ProcessManagementPtr->Shutdown.Flags.GPS == 0b1)
		{
			KillProcess(Units[1], processinfos[1].hProcess);
			StartProcess(Units[1], &startupinfos[1], &processinfos[1]);
		}
		else if (ProcessManagementPtr->Heartbeat.Flags.GPS == 0b1)
		{
			ProcessManagementPtr->Heartbeat.Flags.GPS = 0b0;
			cnt_GPS = 0;
		}
		else if (ProcessManagementPtr->Heartbeat.Flags.GPS == 0b0)
		{
			cnt_GPS++;
			if (cnt_GPS > MAX_WAITBEAT)
			{
				ProcessManagementPtr->Shutdown.Status = 0xFF;
				std::cout << "Critical GPS module is broken." << std::endl;
			}
		}

		if (ProcessManagementPtr->Shutdown.Flags.OpenGL == 0b1)
		{
			KillProcess(Units[3], processinfos[3].hProcess);
			StartProcess(Units[3], &startupinfos[3], &processinfos[3]);
		}
		else if (ProcessManagementPtr->Heartbeat.Flags.OpenGL == 0b1)
		{
			ProcessManagementPtr->Heartbeat.Flags.OpenGL = 0b0;
			cnt_OpenGL = 0;
		}
		else if (ProcessManagementPtr->Heartbeat.Flags.OpenGL == 0b0)
		{
			cnt_OpenGL++;
			if (cnt_OpenGL > MAX_WAITBEAT)
			{
				ProcessManagementPtr->Shutdown.Status = 0xFF;
				std::cout << "Critical OpenGL module is broken." << std::endl;
				continue;
			}
		}

		if (ProcessManagementPtr->Shutdown.Flags.VehicleControl == 0b1)
		{
			KillProcess(Units[4], processinfos[4].hProcess);
			StartProcess(Units[4], &startupinfos[4], &processinfos[4]);
		}
		else if (ProcessManagementPtr->Heartbeat.Flags.VehicleControl == 0b1)
		{
			ProcessManagementPtr->Heartbeat.Flags.VehicleControl = 0b0;
			cnt_VehicleControl = 0;
		}
		else if (ProcessManagementPtr->Heartbeat.Flags.VehicleControl == 0b0)
		{
			cnt_VehicleControl++;
			if (cnt_VehicleControl > MAX_WAITBEAT)
			{
				ProcessManagementPtr->Shutdown.Status = 0xFF;
				std::cout << "Critical VehicleControl module is broken." << std::endl;
				continue;
			}
		}
	}

	//kill all 5 modules
	for (int i = 0; i < NUM_UNITS; i++)
	{
		KillProcess(Units[i], processinfos[i].hProcess);
	}

	// kill PM module
	return 0;
}

void SetupSharedMemory(ProcessManagement* ProcessManagementPtr)
{

}
// Starting funtion of a process
bool StartProcess(TCHAR* processName, STARTUPINFO* startupInfo, PROCESS_INFORMATION* processInfo)
//	STARTUPINFO: https://blog.csdn.net/akof1314/article/details/5471727
//  PROCESS_INFORMATION:  https://blog.csdn.net/akof1314/article/details/5471768
{
	if (!IsProcessRunning((const char*)processName))
	{
		ZeroMemory(startupInfo, sizeof(*startupInfo)); //ZeroMemory宏用0来填充一块内存区域
		(*startupInfo).cb = sizeof(*startupInfo);
		ZeroMemory(processInfo, sizeof(*processInfo));
		//CreateProcess:  https://blog.csdn.net/bzhxuexi/article/details/23950701
		//processthreadsapi.h: https://docs.microsoft.com/zh-cn/windows/win32/api/processthreadsapi/
		if (!CreateProcess(NULL, processName, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, startupInfo, processInfo))
		{
			printf("Start %s failed (%d).\n", processName, GetLastError());
			return false;
		}
		std::cout << "Started: " << processName << std::endl;
	}
	return true;
}

// Killing funtion of a process
bool KillProcess(TCHAR* processName, HANDLE processHandle)
//TerminateProcess:  https://blog.csdn.net/eunyeon/article/details/53708168
{
	bool killedFlag = false;
	if (!IsProcessRunning((const char*)processName))
	{
		std::cout << "Doesn't exist: " << processName << std::endl;
	}
	else if (!TerminateProcess(processHandle, 0))
	{
		printf("Kill %s failed (%d).\n", processName, GetLastError());
	}
	else
	{
		CloseHandle(processHandle);
		std::cout << "Killed: " << processName << std::endl;
		killedFlag = true;
	}
	return killedFlag;
}

//Is process running function
bool IsProcessRunning(const char* processName)
{
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
			if (!_stricmp((const char*)entry.szExeFile, processName))
				exists = true;

	CloseHandle(snapshot);
	return exists;
}
