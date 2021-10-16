#include <iostream>
#include <conio.h>    // _kbhit()
#include <winsock2.h> // socket
#pragma comment(lib, "ws2_32.lib")
#include <SMObject.h>
#include <SMStructs.h>
#include <Windows.h>
#include <sstream>     // isstirngstream

#define LASER_IP "192.168.1.200"
#define LASER_PORT 23000
#define RECVBUF_MAXSIZE 4096
#define PI 3.1416

using namespace std;
using namespace System;

int main()
{
	/* Open existed SharedMemory Object, no need to create a new one.*/
	SMObject ProcessManagementObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	ProcessManagementObj.SMAccess();// Open the existed handle of SMO
	ProcessManagement* ProcessManagementPtr = (ProcessManagement*)ProcessManagementObj.pData;

	SMObject SM_LaserObj(_TEXT("SM_Laser"), sizeof(SM_Laser));
	SM_LaserObj.SMAccess();
	SM_Laser* SM_LaserPtr = (SM_Laser*)SM_LaserObj.pData;

	/* Socket Communication*/
	/*
		Some blogs about socket communication
			https://blog.csdn.net/xiaoquantouer/article/details/58001960
			https://zhuanlan.zhihu.com/p/75672631
			https://www.cnblogs.com/helloworldcode/p/10610581.html
	*/
	// Initialize a socket
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Socket failed to initialise: " << WSAGetLastError() << endl;
		ProcessManagementPtr->Shutdown.Flags.ProcessManagement == 0b1;
		return 0;
	}
	// Creat a socket Object
	SOCKET Laser_Client = socket(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP);
	if (Laser_Client == INVALID_SOCKET)
	{
		cout << "Socket failed to create: " << WSAGetLastError() << endl;
		WSACleanup();
		ProcessManagementPtr->Shutdown.Flags.ProcessManagement == 0b1;
		return 0;
	}
	// Connect the Laser
	SOCKADDR_IN AddrSer;
	AddrSer.sin_family = AF_INET;
	AddrSer.sin_port = htons(LASER_PORT);
	AddrSer.sin_addr.S_un.S_addr = inet_addr(LASER_IP);
	if (connect(Laser_Client, (SOCKADDR*)&AddrSer, sizeof(AddrSer)) == SOCKET_ERROR)
	{
		cout << "Socket failed to connect: " << WSAGetLastError() << endl;
		closesocket(Laser_Client);
		WSACleanup();
		ProcessManagementPtr->Shutdown.Flags.ProcessManagement == 0b1;
		return 0;
	}

	/* Send "zid" to authenticate connection. */
	char* sendBuf = NULL;
	sendBuf = new char[30];
	sprintf(sendBuf, "%s", "1234567\n");// Donot use "sendBuf = "12345678\n" directly"
	if (send(Laser_Client, sendBuf, (int)strlen(sendBuf), 0) == SOCKET_ERROR)
	{
		cout << "Socket failed to send: " << WSAGetLastError() << endl;
		closesocket(Laser_Client);
		WSACleanup();
		delete[]sendBuf;
		ProcessManagementPtr->Shutdown.Flags.Laser == 0b1;
		return 0;
	}
	/*
		Maybe need some presend-message, details on <SICKLMS151.pdf>Page 30
		1.Start measurement (not present)
		2.1 Query measuring status (not present)
		2.2 Data processing (below)
	*/
	char* recvBuf = NULL;
	recvBuf = new char[RECVBUF_MAXSIZE];
	int cnt_ProcessManagement = 0;
	int MAX_WAITBEAT = 100;
	/* Loop */
	while (!ProcessManagementPtr->Shutdown.Flags.Laser)
	{
		if (ProcessManagementPtr->Heartbeat.Flags.Laser == 0b0)
		{
			/* Heart beats */
			ProcessManagementPtr->Heartbeat.Flags.Laser == 0b1;
			cnt_ProcessManagement = 0;
			/* Send "sRN LMDscandatacfg" to ask for original laser data*/
			sprintf(sendBuf, "%c%s%c", 0x02, "sRN LMDscandatacfg", 0x03);
			cout << strlen(sendBuf) << ":" << sendBuf << endl;
			if (send(Laser_Client, sendBuf, (int)strlen(sendBuf), 0) == SOCKET_ERROR)
			{
				cout << "Socket failed to send: " << WSAGetLastError() << endl;
				closesocket(Laser_Client);
				WSACleanup();
				ProcessManagementPtr->Shutdown.Flags.ProcessManagement == 0b1;
				return 0;
			}
			/* Receive the data */
			recv(Laser_Client, recvBuf, RECVBUF_MAXSIZE, 0);

			/* Decode the original laser data*/
			// Message structur:<SICKLMS151.pdf> Page 89-93
			if (!strlen(recvBuf))
			{
				double startAngle = 0;         // Starting of angle
				double stepWidth = 0;          // Resolution of angle
				uint16_t numberData = 0;       // Sum of data

				istringstream isin(recvBuf);   // Object of istringstream
				double dataTmp;

				for (int i = 0; i < 22; i++)
				{
					isin >> dataTmp;           // Skip the headInfos
				}
				isin >> std::hex >> startAngle;// HEX -> double
				isin >> std::hex >> stepWidth;
				isin >> std::hex >> numberData;
				startAngle = startAngle / 10000;
				stepWidth = stepWidth / 10000;

				for (int step = 0; step < numberData; step++)
				{
					isin >> std::hex >> dataTmp;
					SM_LaserPtr->x[step] = dataTmp * cos((startAngle + stepWidth * step) * (PI / 180));
					SM_LaserPtr->y[step] = dataTmp * sin((startAngle + stepWidth * step) * (PI / 180));
					printf("(x[%d], y[%d]) = (%.3f, %.3f)\n", step, step, SM_LaserPtr->x[step], SM_LaserPtr->y[step]);
				}
			}
		}
		else
		{
			if (cnt_ProcessManagement++ > MAX_WAITBEAT)
			{
				ProcessManagementPtr->Shutdown.Flags.Laser = 0b1;
				/*
				   do Shudown Process
				*/
			}
		}
		Sleep(100);
	}
	closesocket(Laser_Client);
	WSACleanup();
	delete[]sendBuf;
	delete[]recvBuf;
	ProcessManagementPtr->Shutdown.Flags.Laser == 0b1;
	return 0;
}