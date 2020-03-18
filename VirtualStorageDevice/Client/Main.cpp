#include "Common.h"
#include "../VirtualStorageDevice/IoctlCodes.h"
#include <string>
#include <iostream>

using std::string;
using std::wstring;
using std::cerr;
using std::cout;
using std::endl;

int wmain() {
	wstring deviceName = L"\\\\.\\VirtualStorageDevice";
	HANDLE device = CreateFile(deviceName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ,
		nullptr, OPEN_EXISTING, 0, nullptr);
	if (device == INVALID_HANDLE_VALUE) {
		auto gle = GetLastError();
		cerr << "CreateFile failed, GLE=" << gle << endl;
		return 1;
	}

	string buffer = "Some data";
	DWORD bytesReturned = 0;
	if (!DeviceIoControl(device, IOCTL_TEST_CODE, const_cast<char*>(buffer.c_str()), buffer.size(), nullptr, 0,
						 &bytesReturned, nullptr)) {
		auto gle = GetLastError();
		cerr << "DeviceIoControl failed, GLE=" << gle << endl;
	}
	cout << "Closing device" << endl;
	CloseHandle(device);
	
	return 0;
}
