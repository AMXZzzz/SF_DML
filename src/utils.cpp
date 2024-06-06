#include "sf-trt.h"
#include <atltime.h>
#include "config-module.h"


std::shared_ptr<spdlog::logger> g_logger;	// ȫ�ֶ���
BOOL InitSpdLog() {
	static char temp[MAX_PATH]{};

	CTime t = CTime::GetCurrentTime();
	_snprintf_s(temp, MAX_PATH, "logs/%d-%d-%d %d-%d-%d.txt", t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());

	g_logger = spdlog::basic_logger_mt("SF_TRT", temp);
	//g_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");	// ����ʾ����
	g_logger->flush_on(spdlog::level::trace);					// �¼���������ˢ��
	g_logger->info("SF_TRT 6.1��������Ϊ��ѳ��򣡣���");
	//g_logger->flush();							// �ֶ�����ˢ��	
	//spdlog::flush_every(std::chrono::seconds(5));	// ÿ5��ˢ�»���
	return TRUE;
}

/*
	����ת��
*/
static std::wstring UTF8ToUnicode(const char* strSrc) {
	std::wstring wstrRet;

	if (NULL != strSrc) {
		int len = MultiByteToWideChar(CP_UTF8, 0, strSrc, -1, NULL, 0) * sizeof(WCHAR);
		WCHAR* strDst = new(std::nothrow) WCHAR[len + 1];
		if (NULL != strDst) {
			MultiByteToWideChar(CP_UTF8, 0, strSrc, -1, strDst, len);
			wstrRet = strDst;;
			delete[]strDst;
		}
	}
	return wstrRet;
}
static std::string UnicodeToAnsi(const WCHAR* strSrc) {
	std::string strRet;

	if (NULL != strSrc) {
		int len = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
		char* strDst = new(std::nothrow) char[len + 1];
		if (NULL != strDst) {
			WideCharToMultiByte(CP_ACP, 0, strSrc, -1, strDst, len, NULL, NULL);
			strRet = strDst;
			delete[]strDst;
		}
	}
	return strRet;
}
std::string UTF8ToAnsi(const char* strSrc) {
	return UnicodeToAnsi(UTF8ToUnicode(strSrc).c_str());
}


/*
	��ANSII�����stringתΪutf-8 ��Чu8
*/
std::string StringToUTF8(const std::string& str) {
	size_t nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	wchar_t* pwBuf = new wchar_t[nwLen + 1];
	ZeroMemory(pwBuf, nwLen * 2 + 2);
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

	size_t nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
	char* pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
	std::string retStr(pBuf);

	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;
	return retStr;
}


std::string GetIniPath() {
	ConfigModule* config = &ConfigModule::Get();
	char exe_path[MAX_PATH];
	std::string comfig_name = config->Config_name;
	comfig_name = "\\" + comfig_name;
	GetModuleFileNameA(NULL, exe_path, MAX_PATH);
	std::string root_path = exe_path;
	root_path = (root_path.substr(0, root_path.find_last_of("\\"))) + comfig_name;
	return root_path;
}

const wchar_t* GetWC(const char* c) {
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);
	return wc;
}

std::wstring String2WString(const std::string& s)   //���ֽ�ת�ɿ��ַ�
{
	std::string strLocale = setlocale(LC_ALL, "");
	const char* chSrc = s.c_str();
	size_t nDestSize = mbstowcs(NULL, chSrc, 0) + 1;
	wchar_t* wchDest = new wchar_t[nDestSize];
	wmemset(wchDest, 0, nDestSize);
	mbstowcs(wchDest, chSrc, nDestSize);
	std::wstring wstrResult = wchDest;
	delete[]wchDest;
	setlocale(LC_ALL, strLocale.c_str());
	return wstrResult;
}

std::string exe_cmd(const char* cmd) {
	char buffer[128] = { 0 };
	std::string result;
	FILE* pipe = _popen(cmd, "r");
	if (!pipe) throw std::runtime_error("_popen() failed!");
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	_pclose(pipe);
	return result;
}

std::string GetSystemReleaseId() {
	HKEY hKey = NULL;
	DWORD keyType;
	DWORD keySize = sizeof(DWORD);
	char szLocation[MAX_PATH] = { '\0' };
	LONG ret;
	std::string releaseId = "-1";
	// ������
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", 0, KEY_READ, &hKey);

	// �򿪳ɹ�
	if (ret == ERROR_SUCCESS) {
		ret = RegQueryValueExA(hKey, "ReleaseId", 0, &keyType, (LPBYTE)&szLocation, &keySize);
		ret = RegQueryValueExA(hKey, "ReleaseId", 0, &keyType, (LPBYTE)&szLocation, &keySize);
		if (ERROR_SUCCESS == ret) {
			releaseId = szLocation;
		}
		// �ر�
		RegCloseKey(hKey);
	}

	return releaseId;
}


std::string& replace_all_distinct(std::string& str, const std::string& old_value, const   std::string& new_value) {
	for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length()) {
		if ((pos = str.find(old_value, pos)) != std::string::npos) {
			str.replace(pos, old_value.length(), new_value);
		}
		else { break; }
	}
	return str;
}

VOID get_user_name(char** name) {
	//char name[MAX_PATH];
	DWORD dwSize = 256;
	GetUserNameA((*name), &dwSize);
}

std::string get_exe_path() {
	char exe_path[MAX_PATH];
	GetModuleFileNameA(NULL, exe_path, MAX_PATH);

	std::string temp = exe_path;
	temp = replace_all_distinct(temp, "\\", "\\\\");
	return temp;
}

std::string reverse_read(CmdOrder Order) {
	std::string exe_path = get_exe_path();
	char* name = nullptr;
	get_user_name(&name);
	char cmd[MAX_PATH];
	switch (Order) {
	case Replace:
		_snprintf_s(cmd, MAX_PATH, "ICACLS %s %s %s:RX", exe_path.c_str(), "/deny", name);
		break;
	case Recover:
		_snprintf_s(cmd, MAX_PATH, "ICACLS %s %s %s:RX", exe_path.c_str(), "/grant", name);
		break;
	case Inquire:
		_snprintf_s(cmd, MAX_PATH, "ICACLS %s", exe_path.c_str());
		break;
	}
	return exe_cmd(cmd);
}
