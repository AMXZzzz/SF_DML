#pragma once
#include <windows.h>

enum CONFIG_IO {
	CONFIG_READ,
	CONFIG_WRITE,
};

struct ConfigModule {
	ConfigModule() {};
	~ConfigModule() {};

	static ConfigModule& Get() {
		static ConfigModule m_pInstance;
		return m_pInstance;
	}

	VOID GetParame(char* path);
	VOID SaveParame();

	char Config_name[MAX_PATH] = "Config.ini";		// configÎÄ¼þÃû
};
