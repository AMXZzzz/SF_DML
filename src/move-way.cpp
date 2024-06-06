#include "move-way.h"
#include "sf-trt.h"
#include "msdk.h"
#include <iostream>

#pragma region �޼�Ghub
BOOL MoveWay::GetNTStatus(PCWSTR device_name) {
	UNICODE_STRING name;
	OBJECT_ATTRIBUTES attr{};
	RtlInitUnicodeString(&name, device_name);
	InitializeObjectAttributes(&attr, &name, 0, NULL, NULL);

	//��Net
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
	g_logger->info("�ͷ��޼�����    Done...");
}

BOOL MoveWay::InitLogitech() {

	if (Lg == NULL) {
		//��ʼ��1
		wchar_t buffer0[] = L"\\??\\ROOT#SYSTEM#0002#{1abc05c0-c378-41b9-9cef-df1aba82b015}";
		if (GetNTStatus(buffer0)) {
			g_logger->info("�޼�buffer0 Done");
			goto Ghub_Done;
		}
		//��ʼ��2
		wchar_t buffer1[] = L"\\??\\ROOT#SYSTEM#0001#{1abc05c0-c378-41b9-9cef-df1aba82b015}";
		if (GetNTStatus(buffer1)) {
			g_logger->info("�޼�buffer1 Done");
			goto Ghub_Done;
		}

		//��ʼ��3
		wchar_t buffer2[] = L"\\??\\ROOT#SYSTEM#0003#{1abc05c0-c378-41b9-9cef-df1aba82b015}";
		if (GetNTStatus(buffer2)) {
			g_logger->info("�޼�buffer2 Done");
			goto Ghub_Done;
		}
	}
Ghub_Done:
	MoveR = LGMove;
	MoveClose = LGMoveClose;
	return TRUE;
}
#pragma endregion

#pragma region �׼���
VOID YjsMove(int x, int y) {
	M_MoveR(MoveWay::Get().yjs, x, y);
}

VOID YjsMoveClose() {
	M_Close(MoveWay::Get().yjs);
	g_logger->info("�ͷ��׼���    Done...");
}

BOOL MoveWay::InitYiJianShu() {
	yjs = M_Open(1);
	if (INVALID_HANDLE_VALUE == yjs) {
		MessageBoxA(NULL, "�׼����ʼ��ʧ��", MESSAGEBOX_TITLE, MB_OK);
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
	g_logger->info("SendInput�����ͷ�  q(�R���Qq)  ");
}

VOID MoveWay::InitSendInput() {
	send_input.type = INPUT_MOUSE;
	send_input.mi.dx = 0;
	send_input.mi.dy = 0;
	send_input.mi.mouseData = 0;
	send_input.mi.dwFlags = MOUSEEVENTF_MOVE;   //MOUSEEVENTF_ABSOLUTE �������λ��  MOUSEEVENTF_MOVE�����ƶ��¼�
	send_input.mi.time = 0;
	send_input.mi.dwExtraInfo = 0;

	MoveR = SendMove;
	MoveClose = SendMoveClose;
}
#pragma endregion

#pragma region �Զ����ƶ���ʽ
typedef BOOL(*CustomizeInit)(VOID*);
typedef VOID(*CustomizeMove)(int, int);
typedef VOID(*CustomizeFree)();

BOOL MoveWay::Customize() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();
	HMODULE modul = LoadLibraryA(variables->move_dll);
	if (modul == NULL) {
		g_logger->warn("�Զ����ƶ���ʽδ�ҵ�{}���뽫�����ͬһ·����", variables->move_dll);
		MessageBoxA(NULL, "δ�ҵ��ƶ�dll", MESSAGEBOX_TITLE, MB_OK);
		return FALSE;
	}

	CustomizeInit Init = (CustomizeInit)GetProcAddress(modul, "CustomizeInit");
	CustomizeMove Move = (CustomizeMove)GetProcAddress(modul, "CustomizeMove");
	CustomizeFree Free = (CustomizeFree)GetProcAddress(modul, "CustomizeFree");
	value->conitor = (CustomizeConitor)GetProcAddress(modul, "CustomizeConitor");

	if (value->conitor == NULL) {
		std::cout << "������갴������" << std::endl;
		g_logger->warn("������갴������");
		return FALSE;
	}

	if (!Init((VOID*)SF::Value::Get().customize_data)) {
		g_logger->warn("�����Զ����ƶ��ĳ�ʼ��ʱ����FALSE��FALSEԼ��Ϊʧ��");	
		return FALSE;
	};

	MoveR = Move;
	MoveClose = Free;
	g_logger->info("�����Զ����ƶ� Done");
	return TRUE;
}
#pragma endregion

BOOL MoveWay::InitMove(int way) {
	switch (way) {
	case 0:		// ghub
		if (!InitLogitech()) {
			g_logger->warn("�޼�Ghub��ʼ��ʧ��");
			return FALSE;
		}
		g_logger->info("����ghub�ƶ� Done");
		break;
	case 1:		// �׼���
		if (!InitYiJianShu()) {
			g_logger->warn("�׼����ʼ��ʧ��");
			return FALSE;
		}
		g_logger->info("�����׼����ƶ� Done");
		break;
	case 2:		// SendInput
		InitSendInput();
		g_logger->info("����SendInput�ƶ� Done");
		break;
	case 3:		// �Զ���
		if (!Customize()) {
			g_logger->warn("�Զ����ʼ��ʧ��");
			return FALSE;
		}
		break;
	}
	return TRUE;
}


