#pragma once
#include <windows.h>
#include <winternl.h>

typedef struct {	// 声明类型
	char button = 0;
	char x = 0;
	char y = 0;
	char wheel = 0;
	char unk1 = 0;
} MOUSE_IO;

struct MoveWay {
	static MoveWay& Get() {
		static MoveWay m_pInstance;
		return m_pInstance;
	}
	HANDLE Lg = NULL;
	HANDLE yjs = NULL;
	INPUT send_input;

	VOID(*MoveR)(int, int);
	VOID(*MoveClose)();


	BOOL InitMove(int way);
	VOID SendDrive(int, int);
private:
	// ghub
	BOOL GetNTStatus(PCWSTR device_name);
	BOOL InitLogitech();
	BOOL Customize();
	NTSTATUS LG_status{};
	MOUSE_IO mouse_io{};
	IO_STATUS_BLOCK LG_io{};

	// 易键鼠
	BOOL InitYiJianShu();

	// 
	VOID InitSendInput();


};


