#include "ProcessesManager.h"
#include "utils.h"
#include "ConsoleUI.h"
#include "WindowUI.h"
#include "CMDUI.h"
#include <NTSecAPI.h>
LSA_HANDLE GetPolicyHandle1() {
	LSA_OBJECT_ATTRIBUTES ObjectAttributes;
	TCHAR* SystemName = NULL;
	USHORT SystemNameLength;
	LSA_UNICODE_STRING lusSystemName;
	NTSTATUS ntsResult;
	LSA_HANDLE lsahPolicyHandle;

	ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

	ntsResult = LsaOpenPolicy(NULL, &ObjectAttributes, POLICY_LOOKUP_NAMES, &lsahPolicyHandle);

	if (ntsResult != CMC_STATUS_SUCCESS) {
		_tprintf(_T("OpenPolicy returned %lu"), LsaNtStatusToWinError(ntsResult));
		return NULL;
	}

	return lsahPolicyHandle;
}
LPTSTR getUserName(PID pid) {
	DWORD res = 0;
	LSA_HANDLE policyHandle;
	policyHandle = GetPolicyHandle1();
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	HANDLE processToken;
	OpenProcessToken(hProcess, TOKEN_QUERY, &processToken);
	TOKEN_USER tokenUser;
	DWORD dwSize;
	GetTokenInformation(processToken, TokenUser, &tokenUser, 0, &dwSize);
	GetTokenInformation(processToken, TokenUser, &tokenUser, dwSize, &dwSize);
	if (policyHandle == NULL) {
		//FAILS TO GET POLICY HANDLE
	}

	if (processToken == NULL) {
		return NULL;
	}

	PSID sids[1];
	sids[0] = tokenUser.User.Sid;
	PLSA_TRANSLATED_NAME names = NULL;
	PLSA_REFERENCED_DOMAIN_LIST referencedNames = NULL;
	LPTSTR Name;
	PWSTR userName = NULL;
	__try {
		if (LsaLookupSids(policyHandle, 1, &tokenUser.User.Sid, &referencedNames, &names) >= 0) {
			if (names[0].Use != SidTypeInvalid && names[0].Use != SidTypeUnknown) {
				if (userName == NULL) {
					userName = names[0].Name.Buffer;
				}
			}
		}
		else {
			res = NULL;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return NULL;
	}


	if (names) {
		LsaFreeMemory(names);
	}

	if (referencedNames) {
		LsaFreeMemory(referencedNames);
	}

	return userName;
}
DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	CMDUI* pCMDUI = (CMDUI*)lpParameter;
	pCMDUI->start();
	return 0L;
}

int wmain(int argc, TCHAR** argv) {
    if (lstrcmpW(argv[1], L"gui")==0) {
        WindowUI windowUI;
        windowUI.start();
    }
    else if (lstrcmpW(argv[1], L"cui")==0) {
        ConsoleUI consoleUI;
        consoleUI.start();
    }
    else {
        CMDUI cmdUI(argc,argv);
        //cmdUI.start();
		HANDLE thread = CreateThread(NULL, 0, ThreadProc, &cmdUI, 0, NULL);
		WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);
    }
}