#pragma once
#include <windows.h>


struct LockMode {
	static LockMode& Get() {
		static LockMode m_pInstance;
		return m_pInstance;
	}

	HANDLE lock_event = NULL;
	BOOL InitLock(int,VOID (**f)());

private:

	BOOL CreateLockEvent();
	HANDLE lock_thread = NULL;
};