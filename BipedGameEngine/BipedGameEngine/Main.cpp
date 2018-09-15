#include "engine/System.h"

int APIENTRY wWinMain(
	_In_		HINSTANCE hInstance,
	_In_opt_	HINSTANCE hPrevInstance,
	_In_		LPWSTR lpCmdLine,
	_In_		int nCmdShow
){
	bpd::System* system = new bpd::System;
	if (!system->Initialize(hInstance)){
		ErrorLogger::Log("Biped Engine failed");
		return 1;
	}
	SAFE_DELETE(system);
	return 0;
}