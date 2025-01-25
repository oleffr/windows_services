#define _CRT_SECURE_NO_WARNINGS
#include <zlib.h>
#include <zip.h>
#include <zconf.h>
#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <fstream>
#include <windows.h>
#include <TCHAR.H>
#include <sys/stat.h>
#include <time.h>

#define serviceName _T("Archive_Service")
#define servicePath _T("C:/Users/liliy/source/repos/BSIT2/Debug/BSIT2.exe")

using namespace std;

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE hStatus;

void zip();

int addLogMessage(const char* str) {
	errno_t err;
	FILE* log;
	if ((err = fopen_s(&log, "C:\\Users\\liliy\\source\\repos\\BSIT2\\log.log", "a+")) != 0) {
		return -1;
	}
	fprintf(log, "%s\n", str);
	fclose(log);
	return 0;
}

void ControlHandler(DWORD request) {
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
		serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hStatus, &serviceStatus);
		return;
	case SERVICE_CONTROL_SHUTDOWN:
		addLogMessage("Shutdown.");
		serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hStatus, &serviceStatus);
		return;
	default:
		break;
	}
	SetServiceStatus(hStatus, &serviceStatus);
	return;
}

void ServiceMain(int argc, char** argv) {

	int i = 0;
	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	serviceStatus.dwWin32ExitCode = 0;
	serviceStatus.dwServiceSpecificExitCode = 0;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;
	hStatus = RegisterServiceCtrlHandler(serviceName, (LPHANDLER_FUNCTION)ControlHandler);

	if (hStatus == (SERVICE_STATUS_HANDLE)0) {
		return;
	}
	serviceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hStatus, &serviceStatus);
	while (serviceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		zip();
	}
	return;
}

bool check(string p, string m)
{
	int i = 0, j = 0, lm = -1, lp = 0;
	while (p[i])
	{
		if (m[j] == '*')
			lp = i, lm = ++j;
		else if (p[i] == m[j] || m[j] == '?')
			i++, j++;
		else if (p[i] != m[j])
		{
			if (lm == -1) return false;
			i = ++lp;
			j = lm;
		}
	}
	if (!m[j]) return !p[i];
	return false;
}

string get_filename(const experimental::filesystem::path& p) { return p.filename().string(); }

void zip()
{
	ifstream in;
	string cat, arch, mask;
	zip_t* archive;
	zip_source_t* s;
	int err;
	in.open("C:\\Users\\liliy\\source\\repos\\BSIT2\\config.txt");
	getline(in, cat);
	getline(in, arch);

	int error_zip;
	WIN32_FIND_DATAA wfd;
	HANDLE const hFind = FindFirstFileA(arch.c_str(), &wfd);
	if (INVALID_HANDLE_VALUE == hFind)
		archive = zip_open(arch.c_str(), ZIP_CREATE, &error_zip);
	else
		archive = zip_open(arch.c_str(), 0, &error_zip);
	while (getline(in, mask))
{
		for (const auto& dirEntry : experimental::filesystem::recursive_directory_iterator(cat)) {
			if (check(get_filename(dirEntry.path()), mask)) {
				string path = dirEntry.path().string();
				path.erase(0, cat.size() + 1);
				if (zip_name_locate(archive, path.c_str(), NULL) == -1) {
					if ((s = zip_source_file(archive, dirEntry.path().string().c_str(), 0, -1)) == NULL || zip_file_add(archive, path.c_str(), s, ZIP_FL_ENC_UTF_8) < 0) {

						zip_source_free(s);
						addLogMessage(">>Error adding file\n");
					}
				}
				else {
					struct stat f_stat;
					zip_stat_t a_stat;
					stat(dirEntry.path().string().c_str(), &f_stat);
					zip_stat(archive, path.c_str(), 0, &a_stat);

					if (&f_stat.st_mtime != &a_stat.mtime) {
						if ((s = zip_source_file(archive, dirEntry.path().string().c_str(), 0, -1)) == NULL || zip_file_add(archive, path.c_str(), s, ZIP_FL_ENC_UTF_8 | ZIP_FL_OVERWRITE) < 0) {

							zip_source_free(s);
							addLogMessage(">>File update error\n");
						}
					}
				}
			}
		}
	}
	zip_close(archive);
	Sleep(5000);
}
int InstallService() {
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager) {
		addLogMessage(">>Error: Can't open Service Control Manager");
		return -1;
	}

	SC_HANDLE hService = CreateService(
		hSCManager,
		serviceName,
		serviceName,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		servicePath,
		NULL, NULL, NULL, NULL, NULL
	);
	if (!hService) {
		int err = GetLastError();
		switch (err) {
		case ERROR_ACCESS_DENIED:
			addLogMessage(">>Error: ERROR_ACCESS_DENIED");
			break;
		case ERROR_CIRCULAR_DEPENDENCY:
			addLogMessage(">>Error: ERROR_CIRCULAR_DEPENDENCY");
			break;
		case ERROR_DUPLICATE_SERVICE_NAME:
			addLogMessage(">>Error: ERROR_DUPLICATE_SERVICE_NAME");
			break;
		case ERROR_INVALID_HANDLE:
			addLogMessage(">>Error: ERROR_INVALID_HANDLE");
			break;
		case ERROR_INVALID_NAME:
			addLogMessage(">>Error: ERROR_INVALID_NAME");
			break;
		case ERROR_INVALID_PARAMETER:
			addLogMessage(">>Error: ERROR_INVALID_PARAMETER");
			break;
		case ERROR_INVALID_SERVICE_ACCOUNT:
			addLogMessage(">>Error: ERROR_INVALID_SERVICE_ACCOUNT");
			break;
		case ERROR_SERVICE_EXISTS:
			addLogMessage(">>Error: ERROR_SERVICE_EXISTS");
			break;
		default:
			addLogMessage(">>Error: Undefined");
		}
		CloseServiceHandle(hSCManager);
		return -1;
	}
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	addLogMessage(">>Success install service!");
	return 0;
}

int RemoveService() {

	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager) {
		addLogMessage(">>Error: Can't open Service Control Manager");
		return -1;
	}

	SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_STOP | DELETE);
	if (!hService) {
		addLogMessage(">>Error: Can't remove service");
		CloseServiceHandle(hSCManager);
		return -1;
	}

	DeleteService(hService);
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	addLogMessage(">>Success remove service!");
	return 0;
}
int StartService() {

	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_START);

	if (!StartService(hService, 0, NULL)) {
		CloseServiceHandle(hSCManager);
		addLogMessage(">>Error: Can't start service");
		return -1;
	}
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	return 0;
}

int Stop_Service() {

	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_STOP);
	SERVICE_STATUS ss;

	if (!ControlService(hService, SERVICE_CONTROL_STOP, &ss))
	{
		CloseServiceHandle(hSCManager);
		addLogMessage(">>Error: impossible stop service");
		return -1;
	}
	addLogMessage(">>Success stopped service");
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[]) {

	SERVICE_TABLE_ENTRY ServiceTable[2];
	ServiceTable[0].lpServiceName = (LPWSTR)serviceName;
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;

	if (argc - 1 == 0) {

		if (!StartServiceCtrlDispatcher(ServiceTable)) {
			addLogMessage("Error: StartServiceCtrlDispatcher");
		}
	}
	else if (wcscmp(argv[argc - 1], _T("install")) == 0) {
		InstallService();
	}
	else if (wcscmp(argv[argc - 1], _T("remove")) == 0) {
		RemoveService();
	}
	else if (wcscmp(argv[argc - 1], _T("start")) == 0) {
		StartService();
	}
	else if (wcscmp(argv[argc - 1], _T("stop")) == 0) {
		Stop_Service();
	}
}



