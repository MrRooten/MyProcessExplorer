#pragma once
#ifndef _PROCESS_H
#define _PROCESS_H
#include "headers.h"
#include "ProcessesManager.h"
#include "Thread.h"
#include <Psapi.h>
#include <signal.h>
#include <minidumpapiset.h>

class ProcessesManager;
class Process {
public:

	typedef struct _CPUState {
		FILETIME createTime;
		FILETIME lpExitTime;
		FILETIME lpKernelTime;
		FILETIME lpUserTime;

		int priority;
		long long int cycles;

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
		TCHAR path[MAX_PATH];
		TCHAR cmdline[MAX_CMDLINE_LENGTH];
		TCHAR currentDirectory[MAX_PATH];
	}ImageState;


	std::vector<Thread*>* threads;
	PID processId;
	LPTSTR processName;
	LPTSTR userName;
	Process* parent = nullptr;
	PID parentPID;
	std::vector<Process*>* childProcesses = nullptr;
	MemoryState* memoryState = new MemoryState();
	CPUState* cpuState = new CPUState();
	IOState* ioState = new IOState();
	HandleState* handleState = new HandleState();
	ImageState* imageState = new ImageState();
	SecurityState* securityState = new SecurityState();
	Process(PID processId);
	DWORD updateInfo();
	DWORD killProcess();
	void setPriority(Priority priority);
	void setAffinity(Affinity affinity);
	Affinity getAffinity();
	Priority getPriority();
	BOOL createDump(LPTSTR filename,MINIDUMP_TYPE dumpType);

	~Process();
private:
	HANDLE thisProcess;
	HANDLE processToken;
	TOKEN_USER* tokenUser;
	PROCESS_MEMORY_COUNTERS_EX pMemoryCounters;
	IO_COUNTERS ioCounters;
	//A Pointer to ProcessesMangager
	ProcessesManager *pManager;
	Affinity affinity;
	void staticInitProcessState();
	//set Process User Name after setting Process Security State
	void setProcessUserName();
	//set Security State in Process
	void setProcessSecurityState();
	//set Image State in Process
	void setProcessImageState();
	//set Hanldes State in Process
	void setProcessHandleState();
	//set IO State in Process
	void setProcessIOState();
	//set Memory State in Process
	void setProcessMemoryState();
	//set CPU State in Process
	void setProcessCPUState();
	BOOL getAllThreads();
};

#endif