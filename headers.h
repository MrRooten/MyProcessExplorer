#pragma once
#ifndef _HEADER_H
#define _HEADER_H

//#include <ntdef.h>
#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>
#include <stdlib.h>
#include <WinBase.h>
#include <Psapi.h>

#include <WinUser.h>
#include <winnt.h>
#include <fileapi.h>

#include <tchar.h>
#include <wchar.h>
#include <stdlib.h>


#define MAX_FILE_LENGTH 4096
#define MAX_USERNAME_LENGTH 255
#define MAX_PROCESS_NAME_LENGTH 255
#define MAX_CMDLINE_LENGTH 2048
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
typedef void (*processCallback)(PROCESSENTRY32 processEntry);
typedef unsigned int PID;
typedef unsigned int TID;
typedef long long int Affinity;
typedef DWORD Priority;
typedef ULONGLONG MemoryUsage;

typedef struct _SecurityState {
	TOKEN_GROUPS Groups;
	DWORD Session;

	SECURITY_IMPERSONATION_LEVEL impersonationLevel;
	//TOKEN_GROUPS_AND_PRIVILEGES groupsWithPrivileges;
	//TOKEN_PRIVILEGES privileges;
	//TOKEN_MANDATORY_LABEL integrity;

}SecurityState;

typedef struct _CPUState {
	FILETIME createTime;
	FILETIME exitTime;
	FILETIME kernelTime;
	FILETIME userTime;

	int priority;
	ULONG64 cycles;

}CPUState;

typedef struct _MemoryState {
	MemoryUsage privateBytes;
	MemoryUsage peakPrivateBytes;
	MemoryUsage virtualSize;
	long long int pageFaults;
	long long int pageFaultsDelta;
	MemoryUsage workingSet;
	MemoryUsage pageFile;

	int memoryPriority;
	MemoryUsage WSPrivate;
	MemoryUsage WSShareable;
	MemoryUsage WSShared;
	MemoryUsage peakWorkingSet;
}MemoryState;

typedef struct _IOState {
	int priority;
	ULONGLONG reads;
	ULONGLONG readDelta;
	MemoryUsage readBytesDelta;
	ULONGLONG writes;
	ULONGLONG writeDelta;
	MemoryUsage writeBytesDelta;
	ULONGLONG other;
	ULONGLONG otherDelta;
	MemoryUsage otherBytesDelta;
}IOState;

typedef struct _HandleState {
	int Handles;
	int PeakHandles;
	int GDIHandles;
	int USERHandles;
}HandleState;

typedef struct _ImageState {
	TCHAR imageFileName[MAX_PATH];
	//TCHAR path[MAX_PATH];
	TCHAR cmdline[MAX_CMDLINE_LENGTH];
	//TCHAR currentDirectory[MAX_PATH];
}ImageState;
#endif