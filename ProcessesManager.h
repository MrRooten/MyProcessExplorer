#pragma once
#ifndef _PROCESSESMANAGER_H
#define _PROCESSESMANAGER_H

#include <vector>
#include <map>
#include "headers.h"
#include <Psapi.h>
#include <signal.h>
#include <minidumpapiset.h>
#include "utils.h"

#pragma comment(lib,"dbghelp.lib")


class Thread;
class ProcessesManager;
class Process {
public:

	std::vector<Thread*>* threads;
	PID processId;
	MPEString processName;
	MPEString userName;
	Process* parent = nullptr;
	PID parentPID;
	std::vector<Process*>* childProcesses = nullptr;
	MemoryState* memoryState = new MemoryState();
	CPUState* cpuState = new CPUState();
	IOState* ioState = new IOState();
	HandleState* handleState = new HandleState();
	ImageState* imageState = new ImageState();
	SecurityState* securityState = new SecurityState();
	Process(PID processId, ProcessesManager* processesManager);
	Process(PID processId);
	DWORD updateInfo();
	DWORD killProcess();
	DWORD suspendProcess();
	DWORD resumeProcess();
	void setPriority(Priority priority);
	void setAffinity(Affinity affinity);
	Affinity getAffinity();
	Priority getPriority();
	BOOL createDump(LPTSTR filename, MINIDUMP_TYPE dumpType);
	
	PID getPID();
	MPEString getUserName();
	SecurityState* getSecurityState();
	ImageState* getImageState();
	HandleState* getHandleState();
	IOState* getIOState();
	MemoryState* getMemoryState();
	CPUState* getCPUState();
	int getCPUUsage();
	~Process();
	HANDLE thisProcess;
	HANDLE processToken;
	//TOKEN_USER* tokenUser;
	PROCESS_MEMORY_COUNTERS_EX pMemoryCounters;
	IO_COUNTERS ioCounters;
	//A Pointer to ProcessesMangager
	ProcessesManager* processesManager;
	Affinity affinity;
	void staticInitProcessState();
	//set Process User Name after setting Process Security State
	DWORD setProcessUserName();
	//set Security State in Process
	DWORD setProcessSecurityState();
	//set Image State in Process
	DWORD setProcessImageState();
	//set Hanldes State in Process
	DWORD setProcessHandleState();
	//set IO State in Process
	DWORD setProcessIOState();
	//set Memory State in Process
	DWORD setProcessMemoryState();
	//set CPU State in Process
	DWORD setProcessCPUState();
	BOOL getAllThreads();

};

class Thread {
public:
	Thread(TID tid);
	SecurityState* securityState;
	TID tid;
	Priority priority;
	ULONG memoryPriority;
	FILETIME createTime;
	FILETIME exitTime;
	FILETIME kernelTime;
	FILETIME userTime;
	ULONG64 cycleTime;
	LPTSTR description;
	void suspendThread();
	void resumeThread();
	void terminateThread();
	~Thread();
private:
	HANDLE thisThreadHandle;
	HANDLE threadToken;
	CONTEXT threadContext;
	void setMemoryPriority();
	void setThreadPriority();
	void setDescription();
	void setThreadContext();
};

class ProcessesManager {
public:
	
	ProcessesManager();
	~ProcessesManager();
	std::vector<Thread*>* getThreadsByPID(PID pid);
	FILETIME systemTime;
	FILETIME userTime;
	FILETIME idleTime;
	void updateInfo();
	std::vector<MPEString> getProcessesProperties(std::vector<LPTSTR> args);
private:
	std::map<PID, Process*> processesMap;
	std::map<PID, std::vector<Thread*>> threadesMap;
	BOOL setAllThreads();
	BOOL setAllProcesses();
};





#endif