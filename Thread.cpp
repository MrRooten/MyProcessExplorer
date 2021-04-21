#include "ProcessesManager.h"
#include "headers.h"

Thread::Thread(TID tid) {
	this->thisThreadHandle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, tid);
	if (this->thisThreadHandle == INVALID_HANDLE_VALUE) {

	}

	if (!OpenThreadToken(this->thisThreadHandle, TOKEN_QUERY, FALSE, &this->threadToken)) {
		//if can't open thread token
	}

	this->securityState = new SecurityState();

}

Thread::~Thread() {

}

void Thread::suspendThread() {
	if (SuspendThread(this->thisThreadHandle) == -1) {
		//error when can't suspend thread 
		return;
	}
}

void Thread::resumeThread() {
	if (ResumeThread(this->thisThreadHandle) == -1) {
		//error when can't resume thread
		return;
	}
}

void Thread::terminateThread() {
	if (!TerminateThread(this->thisThreadHandle, 0)) {
		//error when terminate thread
		return;
	}
}