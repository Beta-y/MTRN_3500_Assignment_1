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
using namespace System::Text;

int main()
{
	/* Open existed SharedMemory Object, no need to create a new one.*/
	SMObject ProcessManagementObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	ProcessManagementObj.SMAccess();// Open the existed handle of SMO
	ProcessManagement* ProcessManagementPtr = (ProcessManagement*)ProcessManagementObj.pData;

	SMObject SM_VehicleControlObj(_TEXT("SM_VehicleControl"), sizeof(SM_VehicleControl));
	SM_VehicleControlObj.SMAccess();
	SM_VehicleControl* SM_VehicleControlPtr = (SM_VehicleControl*)SM_VehicleControlObj.pData;

	/* TCP Client*/
	TcpClient^ Vehicle_Client = gcnew TcpClient(VEHICLE_IP, VEHICLE_PORT);
	Vehicle_Client->NoDelay = true;
	Vehicle_Client->ReceiveTimeout = RECV_TIMEOUT;
	Vehicle_Client->SendTimeout = SEND_TIMEOUT;//ms
	Vehicle_Client->ReceiveBufferSize = RECVBUF_MAXSIZE;
	Vehicle_Client->SendBufferSize = SENDBUF_MAXSIZE;

	/* Send&Recv Buffer*/
	array<unsigned char>^ VC_SendData;
	array<unsigned char>^ VC_RecvData;
	VC_SendData = gcnew array<unsigned char>(SENDBUF_MAXSIZE);
	VC_RecvData = gcnew array<unsigned char>(RECVBUF_MAXSIZE);

	/* Send ZID to get authentication*/
	String^ ZID_Number = gcnew String("123456/n");
	VC_SendData = Encoding::ASCII->GetBytes(ZID_Number);
	NetworkStream^ NETStream = Vehicle_Client->GetStream();
	NETStream->Write(VC_SendData, 0, VC_SendData->Length);
	Console::WriteLine(Encoding::ASCII->GetString(VC_RecvData));

	/* Wait for response.*/
	Sleep(100);

	/* Print the response info*/
	NETStream->Read(VC_RecvData, 0, VC_RecvData->Length);
	Console::WriteLine(Encoding::ASCII->GetString(VC_RecvData));

	String^ Command;// CTRL Message, # <steer><speed><flag> #
	bool AutoCtrl = false;//Flag of Atuo&Manual Ctrl
	int cnt_ProcessManagement = 0;
	int MAX_WAITBEAT = 100;
	while (!ProcessManagementPtr->Shutdown.Flags.VehicleControl)
	{
		if (ProcessManagementPtr->Heartbeat.Flags.VehicleControl == 0b0)
		{
			/* Heart beats */
			ProcessManagementPtr->Heartbeat.Flags.VehicleControl == 0b1;
			cnt_ProcessManagement = 0;

			// Toggle the Auto&Manual flag
			AutoCtrl = !AutoCtrl;
			if (AutoCtrl)
			{
				Command = gcnew String("# " + Convert::ToString(-1 * SM_VehicleControlPtr->Steering) + " " + Convert::ToString(SM_VehicleControlPtr->Speed) + " 1 #");
				VC_SendData = Encoding::ASCII->GetBytes(Command);
				NETStream->Write(VC_SendData, 0, VC_SendData->Length);
				// Print
				Console::WriteLine(Encoding::ASCII->GetString(VC_RecvData));
			}
			else
			{
				Command = gcnew String("# " + Convert::ToString(-1 * SM_VehicleControlPtr->Steering) + " " + Convert::ToString(SM_VehicleControlPtr->Speed) + " 0 #");
				VC_SendData = Encoding::ASCII->GetBytes(Command);
				NETStream->Write(VC_SendData, 0, VC_SendData->Length);
				// Print
				Console::WriteLine(Encoding::ASCII->GetString(VC_RecvData));
			}
		}
		else
		{
			if (cnt_ProcessManagement++ > MAX_WAITBEAT)
			{
				ProcessManagementPtr->Shutdown.Flags.VehicleControl = 0b1;
			}
		}
		Sleep(100);
	}
	NETStream->Close();
	Vehicle_Client->Close();

	return 0;
}