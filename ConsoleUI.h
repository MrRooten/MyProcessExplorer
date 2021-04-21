#pragma once
#include "ProcessesManager.h"
class ConsoleUI
{
public:
	ConsoleUI();
	void start();
private:
	void setHotKey();
	ProcessesManager* processManager;
	HWND hConsole;
	MSG msg = { 0 };
};

