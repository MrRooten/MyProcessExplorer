#include "ConsoleUI.h"

ConsoleUI::ConsoleUI() {
	this->processManager = new ProcessesManager();

	if (this->processManager == NULL) {
		//WHEN CAN'T ALLOC ProcessesManager
	}

	this->hConsole = GetActiveWindow();
}

void ConsoleUI::start() {

}

void ConsoleUI::setHotKey() {
	//Register help hot key
	RegisterHotKey(this->hConsole, 1, MOD_CONTROL | MOD_NOREPEAT, 'H');

	//Register processes hot key
	RegisterHotKey(this->hConsole, 1, MOD_CONTROL | MOD_NOREPEAT, 'P');

	//Register threads of process hot key
	RegisterHotKey(this->hConsole, 1, MOD_CONTROL | MOD_NOREPEAT, 'T');


}
