#include <iostream>
#include <conio.h>    // _kbhit()
#include <SMObject.h>
#include <SMStructs.h>
#include <Windows.h>

#define VEHICLE_IP "192.168.1.200"
#define VEHICLE_PORT 25000
#define RECVBUF_MAXSIZE 1024
#define SENDBUF_MAXSIZE 1024
#define RECV_TIMEOUT 500
#define SEND_TIMEOUT 500

using namespace System;
using namespace System::Net;
using namespace System::Net::Sockets;

int main()
{
	/* Open existed SharedMemory Object, no need to create a new one.*/
	SMObject ProcessManagementObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	ProcessManagementObj.SMAccess();// Open the existed handle of SMO
	ProcessManagement* ProcessManagementPtr = (ProcessManagement*)ProcessManagementObj.pData;

	SMObject SM_VehicleControlObj(_TEXT("SM_VehicleControl"), sizeof(SM_VehicleControl));
	SM_VehicleControlObj.SMAccess();
	SM_VehicleControl* SM_VehicleControlPtr = (SM_VehicleControl*)SM_VehicleControlObj.pData;

	TcpClient^ Vehicle_Client = gcnew TcpClient(VEHICLE_IP, VEHICLE_PORT);
	Vehicle_Client->NoDelay = true;
	Vehicle_Client->ReceiveTimeout = RECV_TIMEOUT;
	Vehicle_Client->SendTimeout = SEND_TIMEOUT;//ms
	Vehicle_Client->ReceiveBufferSize = RECVBUF_MAXSIZE;
	Vehicle_Client->SendBufferSize = SENDBUF_MAXSIZE;

	return 0;
}