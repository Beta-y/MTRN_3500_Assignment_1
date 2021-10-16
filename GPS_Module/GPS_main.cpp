//Compile in a C++ CLR empty project
#using <System.dll>

#include <iostream>
#include <SMObject.h>
#include <SMStructs.h>
#include "GPS.h" 

#define GPS_IP "192.168.1.200"
#define GPS_PORT 24000
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

	SMObject SM_GPSObj(_TEXT("SM_GPS"), sizeof(SM_GPS));
	SM_GPSObj.SMAccess();
	SM_GPS* SM_GPSPtr = (SM_GPS*)SM_GPSObj.pData;

	TcpClient^ GPS_Client = gcnew TcpClient(GPS_IP, GPS_PORT);
	GPS_Client->NoDelay = true;
	GPS_Client->ReceiveTimeout = RECV_TIMEOUT;
	GPS_Client->SendTimeout = SEND_TIMEOUT;//ms
	GPS_Client->ReceiveBufferSize = RECVBUF_MAXSIZE;
	GPS_Client->SendBufferSize = SENDBUF_MAXSIZE;

	/* Recv buffer*/
	array<unsigned char>^ GPS_RecvData;
	GPS_RecvData = gcnew array<unsigned char>(RECVBUF_MAXSIZE);

	NetworkStream^ NETStream = GPS_Client->GetStream();
	int cnt_ProcessManagement = 0;
	int MAX_WAITBEAT = 100;
	/* Loop */
	while (!ProcessManagementPtr->Shutdown.Flags.GPS)
	{
		if (ProcessManagementPtr->Heartbeat.Flags.GPS == 0b0)
		{
			/* Heart beats */
			ProcessManagementPtr->Heartbeat.Flags.GPS == 0b1;
			cnt_ProcessManagement = 0;
			if (NETStream->DataAvailable)
			{
				NETStream->Read(GPS_RecvData, 0, GPS_RecvData->Length);

				/* Codes from lecture slite - begin*/
				unsigned int Header = 0;
				int i = 0;
				int Start;
				unsigned char Data;
				// Find the header
				do
				{
					Data = GPS_RecvData[i++]; // 1bytes = 8bits
					Header = ((Header << 8) | Data);// Left-move 1bytes everyloop, OR-operation means 1|0 = 1; 0|0 = 0; 1|1 = 1;
				} while (Header != 0xaa44121c);

				// Where header begins, -4 bytes
				Start = i - 4;

				SM_GPS GPS_SMOBject;
				unsigned char* GPS_BytePtr = (unsigned char*)&GPS_SMOBject;

				/* Process data Begin:
					if data from GPS include:northing->easting->height->other info->CRC checksum */
				int j = 0;
				for (j = Start; j < Start + sizeof(SM_GPS) - sizeof(unsigned int); j++)// GPS_SMOBject.northing, GPS_SMOBject.easting, GPS_SMOBject.height
				{
					*(GPS_BytePtr++) = GPS_RecvData[j];
				}
				//Where "CRC checksum" begins, 40 bytes
				j = j + 40;// 40 bytes
				// GPS_SMOBject.checksum
				for (int k = j; k < j + sizeof(unsigned int); k++)
				{
					*(GPS_BytePtr++) = GPS_RecvData[k];
				}
				unsigned long CRC_checksum = CalculateBlockCRC32(28 + 80, (unsigned char*)(GPS_RecvData[Start]));
				if (GPS_SMOBject.checksum == CRC_checksum)
				{
					SM_GPSPtr->northing = GPS_SMOBject.northing;
					SM_GPSPtr->easting = GPS_SMOBject.easting;
					SM_GPSPtr->height = GPS_SMOBject.height;
					SM_GPSPtr->checksum = GPS_SMOBject.checksum;
				}
				else
				{
					std::cout << "Failed to get GPS Data! " << std::endl;
				}
				/* Process data End*/

				std::cout << "Northing: " << GPS_SMOBject.northing << std::endl;
				std::cout << "Easting: " << GPS_SMOBject.easting << std::endl;
				std::cout << "Height: " << GPS_SMOBject.height << std::endl;
				std::cout << std::hex << "Recv_CRC: " << GPS_SMOBject.checksum << std::endl;
				std::cout << std::hex << "Cac_CRC: " << CRC_checksum << std::endl;
			}
		}
		else
		{
			if (cnt_ProcessManagement++ > MAX_WAITBEAT)
			{
				ProcessManagementPtr->Shutdown.Flags.GPS = 0b1;
			}
		}
	}
	NETStream->Close();
	GPS_Client->Close();
	return 0;
}

/* CRC test - Binary, User Manual P25*/
//#include <iostream>
//#include <string.h>
//#include "GPS.h" 
//int main()
//{
//	unsigned char buffer[] = { 0xAA, 0x44, 0x12, 0x1C, 0x2A,0x00, 0x02, 0x42, 0x48, 0x00, 0x00, 0x00, 0x96, 0xB4, 0x05, 0x05,
//	0x90, 0x32, 0x8E, 0x09, 0x20, 0x00, 0x00, 0x00, 0x41, 0x59, 0x8C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
//	0x03, 0x9A, 0x8A, 0x8A, 0xE6, 0x8E, 0x49, 0x40, 0xEB, 0xD8, 0xE7, 0xB2, 0x73, 0x82, 0x5C, 0xC0, 0x00, 0xB0, 0xDD,
//	0xA2, 0x37,0x9B, 0x90, 0x40, 0x80, 0x2B, 0x82, 0xC1, 0x3D, 0x00, 0x00, 0x00, 0x9D, 0xDA, 0x3F, 0xF7, 0x58, 0xA1,
//	0x3F, 0x3F, 0xF4, 0x32, 0x89, 0x40, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x0A,
//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//	unsigned long crc = CalculateBlockCRC32(60, buffer);
//	std::cout << std::hex << crc << std::endl; //1168de0c
//	system("pause");
//  return 0;
//}

/* CRC test - ASCII User Manual P25*/
//#include <iostream>
//#include <string.h>
//#include "GPS.h" 
//int main()
//{
//	char * i = "#BESTPOSA,COM2,0,77.5,FINESTEERING,1285,160578.000,00000020,5941,1164;SOL_COMPUTED,SINGLE,51.11640941570,-114.03830951024,1062.6963,-16.2712,WGS84,1.6890,1.2564,2.7826,"",0.000,0.000,10,10,0,0,0,0,0,0";
//	unsigned long iLen = strlen(i);
//	unsigned long CRC = CalculateBlockCRC32(iLen, (unsigned char*)i);
//	std::cout << std::hex << CRC << std::endl;//23a3ee9a
//	system("pause");
//  return 0;
//}