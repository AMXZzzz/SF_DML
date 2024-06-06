#include "move-way.h"
#include "sf-trt.h"
#include "msdk.h"
#include <iostream>

#pragma region 罗技Ghub
BOOL MoveWay::GetNTStatus(PCWSTR device_name) {
	UNICODE_STRING name;
	OBJECT_ATTRIBUTES attr{};
	RtlInitUnicodeString(&name, device_name);
	InitializeObjectAttributes(&attr, &name, 0, NULL, NULL);

	//打开Net
	LG_status = NtCreateFile(&Lg, GENERIC_WRITE | SYNCHRONIZE, &attr, &LG_io, 0,
		FILE_ATTRIBUTE_NORMAL, 0, 3, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, 0, 0);
	if (!(NT_SUCCESS(LG_status))) {
		return FALSE;
	}
	return TRUE;
}

VOID MoveWay::SendDrive(int x, int y) {
	mouse_io.x = x;
	mouse_io.y = y;
	IO_STATUS_BLOCK block;
	NtDeviceIoControlFile(Lg, 0, 0, 0, &block, 0x2a2010, &mouse_io, sizeof(MOUSE_IO), 0, 0);	// == 0L is free
}

VOID LGMove(int x,int y) {
	if (abs(x) > 127 || abs(y) > 127) {
		int x_left = x; int y_left = y;

		if (abs(x) > 127) {
			MoveWay::Get().SendDrive(int(x / abs(x)) * 127, 0);
			x_left = x - int(x / abs(x)) * 127;
		}
		else {
			MoveWay::Get().SendDrive(int(x), 0);
			x_left = 0;
		}

		if (abs(y) > 127) {
			MoveWay::Get().SendDrive(0, int(y / abs(y)) * 127);
			y_left = y - int(y / abs(y)) * 127;
		}
		else {
			MoveWay::Get().SendDrive(0, int(y));
			y_left = 0;
		}

		return LGMove(x_left, y_left);
	}
	else {
		MoveWay::Get().SendDrive(x, y);
	}
}

VOID LGMoveClose() {
	if (MoveWay::Get().Lg != NULL) {
		NtClose(MoveWay::Get().Lg);
		MoveWay::Get().Lg = nullptr;
	}
	g_logger->info("释放罗技驱动    Done...");
}

BOOL MoveWay::InitLogitech() {

	if (Lg == NULL) {
		//初始化1
		wchar_t buffer0[] = L"\\??\\ROOT#SYSTEM#0002#{1abc05c0-c378-41b9-9cef-df1aba82b015}";
		if (GetNTStatus(buffer0)) {
			g_logger->info("罗技buffer0 Done");
			goto Ghub_Done;
		}
		//初始化2
		wchar_t buffer1[] = L"\\??\\ROOT#SYSTEM#0001#{1abc05c0-c378-41b9-9cef-df1aba82b015}";
		if (GetNTStatus(buffer1)) {
			g_logger->info("罗技buffer1 Done");
			goto Ghub_Done;
		}

		//初始化3
		wchar_t buffer2[] = L"\\??\\ROOT#SYSTEM#0003#{1abc05c0-c378-41b9-9cef-df1aba82b015}";
		if (GetNTStatus(buffer2)) {
			g_logger->info("罗技buffer2 Done");
			goto Ghub_Done;
		}
	}
Ghub_Done:
	MoveR = LGMove;
	MoveClose = LGMoveClose;
	return TRUE;
}
#pragma endregion

#pragma region 易键鼠
VOID YjsMove(int x, int y) {
	M_MoveR(MoveWay::Get().yjs, x, y);
}

VOID YjsMoveClose() {
	M_Close(MoveWay::Get().yjs);
	g_logger->info("释放易键鼠    Done...");
}

BOOL MoveWay::InitYiJianShu() {
	yjs = M_Open(1);
	if (INVALID_HANDLE_VALUE == yjs) {
		MessageBoxA(NULL, "易键鼠初始化失败", MESSAGEBOX_TITLE, MB_OK);
		return FALSE;
	}

	MoveR = YjsMove;
	MoveClose = YjsMoveClose;
	return TRUE;
}
#pragma endregion

#pragma region SendInput
VOID SendMove(int x,int y) {
	MoveWay::Get().send_input.mi.dx = x;
	MoveWay::Get().send_input.mi.dy = y;
	SendInput(1, &MoveWay::Get().send_input, sizeof(INPUT));
}

VOID SendMoveClose() {
	g_logger->info("SendInput无需释放  q(≧▽≦q)  ");
}

VOID MoveWay::InitSendInput() {
	send_input.type = INPUT_MOUSE;
	send_input.mi.dx = 0;
	send_input.mi.dy = 0;
	send_input.mi.mouseData = 0;
	send_input.mi.dwFlags = MOUSEEVENTF_MOVE;   //MOUSEEVENTF_ABSOLUTE 代表决对位置  MOUSEEVENTF_MOVE代表移动事件
	send_input.mi.time = 0;
	send_input.mi.dwExtraInfo = 0;

	MoveR = SendMove;
	MoveClose = SendMoveClose;
}
#pragma endregion

#pragma region 自定义移动方式
typedef BOOL(*CustomizeInit)(VOID*);
typedef VOID(*CustomizeMove)(int, int);
typedef VOID(*CustomizeFree)();

BOOL MoveWay::Customize() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();
	HMODULE modul = LoadLibraryA(variables->move_dll);
	if (modul == NULL) {
		g_logger->warn("自定义移动方式未找到{}，请将其放在同一路径下", variables->move_dll);
		MessageBoxA(NULL, "未找到移动dll", MESSAGEBOX_TITLE, MB_OK);
		return FALSE;
	}

	CustomizeInit Init = (CustomizeInit)GetProcAddress(modul, "CustomizeInit");
	CustomizeMove Move = (CustomizeMove)GetProcAddress(modul, "CustomizeMove");
	CustomizeFree Free = (CustomizeFree)GetProcAddress(modul, "CustomizeFree");
	value->conitor = (CustomizeConitor)GetProcAddress(modul, "CustomizeConitor");

	if (value->conitor == NULL) {
		std::cout << "监听鼠标按键错误" << std::endl;
		g_logger->warn("监听鼠标按键错误");
		return FALSE;
	}

	if (!Init((VOID*)SF::Value::Get().customize_data)) {
		g_logger->warn("调用自定义移动的初始化时返回FALSE，FALSE约定为失败");	
		return FALSE;
	};

	MoveR = Move;
	MoveClose = Free;
	g_logger->info("调用自定义移动 Done");
	return TRUE;
}
#pragma endregion

BOOL MoveWay::InitMove(int way) {
	switch (way) {
	case 0:		// ghub
		if (!InitLogitech()) {
			g_logger->warn("罗技Ghub初始化失败");
			return FALSE;
		}
		g_logger->info("调用ghub移动 Done");
		break;
	case 1:		// 易键鼠
		if (!InitYiJianShu()) {
			g_logger->warn("易键鼠初始化失败");
			return FALSE;
		}
		g_logger->info("调用易键鼠移动 Done");
		break;
	case 2:		// SendInput
		InitSendInput();
		g_logger->info("调用SendInput移动 Done");
		break;
	case 3:		// 自定义
		if (!Customize()) {
			g_logger->warn("自定义初始化失败");
			return FALSE;
		}
		break;
	}
	return TRUE;
}


