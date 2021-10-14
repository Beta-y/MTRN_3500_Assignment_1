#ifndef SMLASER_H
#define SMLASER_H

#include <Windows.h>
#include <tchar.h>
#include "SMfcn.h"

class SMObject
{
	HANDLE CreateHandle;
	HANDLE AccessHandle;
	TCHAR *szName;
	int Size;
public:
	void *pData;
	int SMCreateError;
	int SMAccessError;
public:
	SMObject();
	SMObject(TCHAR* szname, int size);
	~SMObject();
	int SMCreate();
	int SMAccess();
	void SetSzname(TCHAR *szname);
	void SetSize(int size);
};
#endif