#include "CMDUI.h"
#include <map>
#include <iterator>
#include <process.h>
#include <NTSecAPI.h>
#include <string>
#include <iostream>
std::string GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message = std::string(messageBuffer, size);
	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}
BOOL isNumberic(LPTSTR str) {
	for (int i = 0; i < wcslen(str); i++) {
		if (!iswdigit(str[i])) {
			return FALSE;
		}
	}
	return TRUE;
}

LPTSTR getUserName(PID pid);
static __int64 file_time_2_utc(const FILETIME* ftime)
{
	LARGE_INTEGER li;

	li.LowPart = ftime->dwLowDateTime;
	li.HighPart = ftime->dwHighDateTime;
	return li.QuadPart;
}

//Get CPU cores
static int get_processor_number()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return (int)info.dwNumberOfProcessors;
}

void error_message(const char* message,int error_code) {
	printf_s(message);
	printf_s("\n");
	printf_s("Error code:%d\n", error_code);
	printf_s("\n");
	exit(-1);
}

//This subtract algorithm of two structs FILETIME only for first greater than second
//Means that this subtract only get delta of two FILETIME and that delta is not greater than DWORD range
DWORD inline subtractFILETIME(FILETIME first, FILETIME second) {
	return MAXDWORD & (first.dwLowDateTime - second.dwLowDateTime);
}
CMDUI::CMDUI(int argc,TCHAR** args) {
	this->args = (LPTSTR*)args;
	this->argc = argc;
}

void handler(std::map<PID, Process>::iterator it) {

}
std::vector<LPTSTR> argumentToVector(LPTSTR args) {
	size_t length = wcslen(args);
	std::vector<LPTSTR> res;
	if (args[0] != L'(' || args[length - 1] != L')') {
		return res;
	}

	if (length < 2) {
		return res;
	}
	TCHAR buffer[100] = { 0 };
	LPTSTR _ = &args[1];
	wcsncpy(buffer, _, length - 2);
	size_t bufferLength = wcslen(buffer);
	size_t i;
	size_t j;
	for (i = 0, j = 0; j < bufferLength; j++) {
		if (buffer[j] == L',') {
			res.push_back((LPTSTR)GlobalAlloc(GPTR, sizeof(TCHAR) * (j - i + 1)));
			wcsncpy(res[res.size() - 1], (buffer + i), j - i);
			i = j + 1;
		}
	}

	res.push_back((LPTSTR)GlobalAlloc(GPTR, sizeof(TCHAR) * (j - i + 1)));
	wcsncpy(res[res.size() - 1], (buffer + i), j - i);
	return res;
}
void CMDUI::start() {
	if (argc < 2) {
		perror("The args' count least have 2");
	}
	if (wcscmp(args[1], L"process") == 0) {
		this->operateProcess();
	}
	else if (wcscmp(args[1], L"service") == 0) {
		this->operateService();
	}
	else if (wcscmp(args[1], L"thread") == 0) {
		this->operateThread();
	}
	else if (wcscmp(args[1], L"kill") == 0){
		this->operateKill();
	}
	else {
		this->help();
	}
}

void CMDUI::operateProcess() {
	if (argc < 3) {
		//show usage of help
		perror("this command least have 3 arguments");
	}
	if (wcscmp(args[2], L"help")==0) {
		//show help of process command
		printf_s("show (arg1,arg2,arg3,....):show process's property(default just show processes'names)\n");
		printf_s("	pid:process's pid");
		printf_s("	name:process's name");
		printf_s("	cpu:show cpu's usage\n");
		printf_s("	memory:show memory's usage\n");
		printf_s("	user:show who own this processs\n");
		printf_s("  priority:show the priority of process");
		printf_s("kill pid:kill process by pid\n");
		printf_s("suspend pid:suspend process by pid\n");
		printf_s("resume pid:resume process that suspended\n");

	}
	else if (wcscmp(args[2], L"show")==0) {
		//show processes of system
		if (argc == 3 || argc >= 4) {
			//mpe.exe process show
			HANDLE hProcessSnap = INVALID_HANDLE_VALUE;
			PROCESSENTRY32 pe32;

			hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

			if (hProcessSnap == INVALID_HANDLE_VALUE) {
				error_message("Error happen when CreateToolhelp32Snapshot",GetLastError());
			}

			pe32.dwSize = sizeof(PROCESSENTRY32);

			if (!Process32First(hProcessSnap, &pe32)) {
				CloseHandle(hProcessSnap);
				error_message("Error happen in invocation of Process32First", GetLastError());
			}

			std::map<PID,Process*> processesMap;
			if (argc == 4) {
				do {
					PID pid = pe32.th32ProcessID;
					PID parentPid = pe32.th32ParentProcessID;
					if (pid == 0) {
						continue;
					}
					processesMap[pid] = new Process(pid);
					processesMap[pid]->processName = pe32.szExeFile;
					processesMap[pid]->parentPID = parentPid;
				} while (Process32Next(hProcessSnap, &pe32));
			}
			else if (argc == 5) {
				if (isNumberic(args[4])) {
					PID pid = _tstoi(args[4]);
					processesMap[pid] = new Process(pid);
					if (processesMap[pid]->thisProcess) {
						TCHAR Buffer[256];
						if (GetModuleBaseName(processesMap[pid]->thisProcess, 0, Buffer, 256)) {
							processesMap[pid]->processName = Buffer;
						}
						else {
							processesMap[pid]->processName = NULL;
						}
					}
				}
			}


			if (argc == 3) {
				for (auto it = processesMap.begin(); it != processesMap.end(); it++) {
					wprintf(L"%d %s\n",it->first,it->second->processName.toTString());
				}
			}
			else {
				//handle when argc == 4
				BOOL haveCPUState = FALSE;
				this->properties = argumentToVector(this->args[3]);
				
				for (LPTSTR arg : properties) {
					if (wcscmp(arg, L"cpu") == 0) {
						haveCPUState = TRUE;
						break;
					}
				}
				FILETIME idleTime;
				FILETIME systemKernelTime;
				FILETIME systemUserTime;
				__int64 skt;
				__int64 sut;
				if (haveCPUState) {
					
					GetSystemTimes(&idleTime, &systemKernelTime, &systemUserTime);
					skt = file_time_2_utc(&systemKernelTime);
					sut = file_time_2_utc(&systemUserTime);
					for (std::map<PID, Process*>::iterator it = processesMap.begin();
						it != processesMap.end(); it++) {
						it->second->setProcessCPUState();
					}
				}

				Sleep(1000);
				for (auto it = processesMap.begin(); it != processesMap.end(); it++) {
					Process* process = it->second;
					DOUBLE cpuUsage;
					SIZE_T workingSet;
					LPTSTR imageName;
					PID pid;
					LPTSTR userName;
					wprintf(L"%d %s:\n", it->first, it->second->processName.toTString());
					for (LPTSTR arg : properties) {
						if (wcscmp(L"cpu", arg) == 0) {
							FILETIME kernelTime;
							FILETIME userTime;
							FILETIME lastKernelTime;
							FILETIME lastUserTime;
							kernelTime = it->second->cpuState->kernelTime;
							__int64 kt = file_time_2_utc(&kernelTime);
							userTime = it->second->cpuState->userTime;
							__int64 ut = file_time_2_utc(&userTime);
							it->second->setProcessCPUState();
							lastKernelTime = it->second->cpuState->kernelTime;
							__int64 lkt = file_time_2_utc(&lastKernelTime);
							lastUserTime = it->second->cpuState->userTime;
							__int64 lut = file_time_2_utc(&lastUserTime);

							FILETIME lastIdleTime;
							FILETIME lastSystemKernelTime;
							FILETIME lastSystemUserTime;
							GetSystemTimes(&lastIdleTime, &lastSystemKernelTime, &lastSystemUserTime);
							__int64 lskt = file_time_2_utc(&lastSystemKernelTime);
							__int64 lsut = file_time_2_utc(&lastSystemUserTime);
							cpuUsage = 400 * (DOUBLE)((lut - ut) + (lkt - kt)) / ((lskt - skt) + (lsut - sut));
							printf("    CPU Usage:%.3f\n", cpuUsage);
						}
						else if (wcscmp(L"memory", arg) == 0) {
							process->setProcessMemoryState();
							workingSet = process->memoryState->workingSet;
							printf("    Working Set:%ld\n", workingSet);
						}
						else if (wcscmp(L"name", arg) == 0) {
							process->setProcessImageState();
							imageName = process->processName.toTString();
							wprintf(L"    Image Name:%s\n", imageName);
						}
						else if (wcscmp(L"pid", arg) == 0) {
							pid = it->first;
							printf("	PID:%d\n", pid);
						}
						else if (wcscmp(L"user", arg) == 0) {

								userName = getUserName(process->processId);
								if (userName == NULL) {
									TCHAR e[] = L"access denied";
									userName = e;
								}
								wprintf(L"    UserName:%s\n", userName);
							/*
							if (process->setProcessUserName() == 0) {
								userName = process->userName.toTString();
								wprintf(L"%s ", userName);
							}
							else {
								userName = NULL;
							}*/
						}
						else if (wcscmp(L"priority",arg)==0){
							DWORD dwPriority = GetPriorityClass(process->thisProcess);
							TCHAR priority[40];
							if (dwPriority == ABOVE_NORMAL_PRIORITY_CLASS) {
								wcscpy(priority, L"HIGHER");
							}
							else if (dwPriority == BELOW_NORMAL_PRIORITY_CLASS) {
								wcscpy(priority, L"LOWER");
							}
							else if (dwPriority == HIGH_PRIORITY_CLASS) {
								wcscpy(priority, L"HIGHEST");
							}
							else if (dwPriority == IDLE_PRIORITY_CLASS) {
								wcscpy(priority, L"IDLE PRIORITY");
							}
							else if (dwPriority == NORMAL_PRIORITY_CLASS) {
								wcscpy(priority, L"NORMAL");
							}
							else if (dwPriority == REALTIME_PRIORITY_CLASS) {
								wcscpy(priority, L"REALTIME");
							}
							wprintf(L"    Priority:%s\n", priority);
						}
					}

					printf("\n");
				}
			}
		}
		else if (argc == 5) {
			//mpe.exe process show (arg1,arg2,...) pid
			//show process information
			if (!isNumberic(args[4])) {
				error_message("Give pid is not digit format", 0);
			}

			PID pid = _tstoi(args[4]);
			Process* process = new Process(pid);

		}
	}
    else if (wcscmp(args[2], L"kill") == 0) {
		if (argc == 4) {
			if (isNumberic(args[3])) {
				PID pid = _tstoi(args[3]);
				Process* process = new Process(pid);
				if (process->killProcess() != 0) {
					std::string errorMessage = GetLastErrorAsString();
					std::cout << errorMessage << std::endl;
				}
			}
		}
	}
	else if (wcscmp(args[2], L"suspend") == 0) {
		if (argc == 4) {
		if (isNumberic(args[3])) {
			PID pid = _tstoi(args[3]);
			Process* process = new Process(pid);
			if (process->suspendProcess() != 0) {
				std::string errorMessage = GetLastErrorAsString();
				std::cout << errorMessage << std::endl;
			}
		}
	}
	}
	else if (wcscmp(args[2], L"resume") == 0) {
	if (argc == 4) {
		if (isNumberic(args[3])) {
			PID pid = _tstoi(args[3]);
			Process* process = new Process(pid);
			if (process->resumeProcess() != 0) {
				std::string errorMessage = GetLastErrorAsString();
				std::cout << errorMessage << std::endl;
			}
		}
	}
	}
}

void CMDUI::operateKill() {
	if (wcscmp(args[2], L"help")) {
		//show help of kill command
	}
	else if (isNumberic(args[2])) {
		//terminal process by using pid(process id)
		PID pid = _tstoi(args[2]);
		HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
		if (hProcess == NULL) {
			//Error
		}
		TerminateProcess(hProcess, 0);
	}
	else {
		//unknown command
	}
}

void CMDUI::operateService() {
	if (wcscmp(args[2], L"help") == 0) {
		//show help of service command
		printf_s("service show:display the service in this computer's state\n");
	}
	else if (wcscmp(args[2], L"show") == 0) {
		//show services of system
		//Open the Service Control Manager
		SC_HANDLE sc = ::OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
		//Check if OpenSCManager returns NULL. Otherwise proceed
		if (sc != NULL)
		{
			printf("Opened SCM using OpenSCManager n");
			ENUM_SERVICE_STATUS service_data, * lpservice;
			BOOL retVal;
			DWORD bytesNeeded, srvCount, resumeHandle = 0, srvType, srvState;
			srvType = SERVICE_WIN32;
			srvState = SERVICE_STATE_ALL;
			//Call EnumServicesStatus using the handle returned by OpenSCManager
			retVal = ::EnumServicesStatus(sc, srvType, srvState, &service_data, sizeof(service_data),
				&bytesNeeded, &srvCount, &resumeHandle);
			DWORD err = GetLastError();
			DWORD dwBytes = bytesNeeded + sizeof(ENUM_SERVICE_STATUS);
			lpservice = new ENUM_SERVICE_STATUS[dwBytes];
			//Check if EnumServicesStatus needs more memory space
			if ((retVal == FALSE) || err == ERROR_MORE_DATA)
			{


				EnumServicesStatus(sc, srvType, srvState, lpservice, dwBytes,
					&bytesNeeded, &srvCount, &resumeHandle);
			}
			printf("Count of NT Services using EnumServicesStatus : %dnn", srvCount);
			for (DWORD i = 0; i < srvCount; i++)
			{
				LPTSTR STATE = (LPTSTR)GlobalAlloc(GPTR,sizeof(TCHAR) * 40);
				DWORD dwState = lpservice[i].ServiceStatus.dwCurrentState;
				if (dwState == SERVICE_CONTINUE_PENDING) {
					wcscpy(STATE, L"SERVICE_CONTINUE_PENDING");
				}
				else if (dwState == SERVICE_PAUSE_PENDING) {
					wcscpy(STATE, L"SERVICE_PAUSE_PENDING");
				}
				else if (dwState == SERVICE_PAUSED) {
					wcscpy(STATE, L"SERVICE_PAUSED");
				}
				else if (dwState == SERVICE_RUNNING) {
					wcscpy(STATE, L"SERVICE_RUNNING");
				}
				else if (dwState == SERVICE_START_PENDING) {
					wcscpy(STATE, L"SERVICE_START_PENDING");
				}
				else if (dwState == SERVICE_STOP_PENDING) {
					wcscpy(STATE, L"SERVICE_STOP_PENDING");
				}
				else if (dwState == SERVICE_STOPPED) {
					wcscpy(STATE, L"SERVICE_STOPPED");
				}
				_tprintf(L"%s %s\n", lpservice[i].lpDisplayName,STATE);
				GlobalFree(STATE);
			}
			delete[] lpservice;
		}

		//Close the SC_HANLDE returned by OpenSCManager
		CloseServiceHandle(sc);
		
	}
	else {
		//unknow command
	}
}

void CMDUI::operateThread() {
	if (wcscmp(args[2], L"help") == 0) {
		//show help of thread command
	}
	else if (wcscmp(args[2], L"show") == 0) {
		//show threads of system
	}
	else {
		//unknown command
	}
}

void CMDUI::help() {

}