#include "ProcessesManager.h"
BOOL ProcessesManager::setAllThreads() {
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
	THREADENTRY32 te32;

	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return FALSE;

	te32.dwSize = sizeof(THREADENTRY32);

	if (!Thread32First(hThreadSnap, &te32)) {
		CloseHandle(hThreadSnap);
		return FALSE;
	}

	do {
		PID pid = te32.th32OwnerProcessID;
		if (this->threadesMap.count(pid)) {
			threadesMap[pid].push_back(new Thread(te32.th32ThreadID));
		}
		else {
			std::vector<Thread*> _threads;
			threadesMap[pid] = _threads;
		}
	} while (Thread32Next(hThreadSnap, &te32));

	return TRUE;
}


ProcessesManager::ProcessesManager() {

}

ProcessesManager::~ProcessesManager() {

}

BOOL ProcessesManager::setAllProcesses() {
	HANDLE hProcessSnap = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 pe32;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32)) {
		CloseHandle(hProcessSnap);
		return FALSE;
	}

	do {
		PID pid = pe32.th32ProcessID;
		PID parentPid = pe32.th32ParentProcessID;
		processesMap[pid] = new Process(pid,this);
		processesMap[pid]->parentPID = parentPid;
	} while (Process32Next(hProcessSnap, &pe32));

	return TRUE;
}

std::vector<Thread*>* ProcessesManager::getThreadsByPID(PID pid) {
	return &this->threadesMap[pid];
}
