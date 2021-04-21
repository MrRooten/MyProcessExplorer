#pragma once
#ifndef _CMDUI_H
#define _CMDUI_H
#include "headers.h"
#include "ProcessesManager.h"
class CMDUI {
public:
	CMDUI(int argc,TCHAR** args);
	void start();
	void operateProcess();
	void operateThread();
	void operateService();
	void operateKill();
	void help();
	std::vector<LPTSTR> properties;
private:
	LPTSTR* args;
	int argc;
	ProcessesManager* processesManager;
};

#endif