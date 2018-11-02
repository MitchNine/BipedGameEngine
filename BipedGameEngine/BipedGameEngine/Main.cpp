#include "System.h"

// Application entry point
int APIENTRY wWinMain(
	_In_		HINSTANCE hInstance,
	_In_opt_	HINSTANCE hPrevInstance,
	_In_		LPWSTR lpCmdLine,
	_In_		int nCmdShow
){

	// Core system initialize
	bpd::System system = bpd::System();
	if (!system.Initialize(hInstance)){

		// If System failed to initialize show an error message
		ErrorLogger::Log("Biped Engine failed");
		return 1;
	}

	return 0;
}