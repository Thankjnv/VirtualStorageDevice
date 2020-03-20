#include "Utils.h"
#include "../VirtualStorageDevice/Common.h"
#include <string>
#include <iostream>
#include <vector>

using std::string;
using std::wstring;
using std::vector;
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

	wstring path = L"\\Device\\HarddiskVolume1\\Temp\\a.txt";
	auto size = (path.size() * sizeof(path[0]));
	vector<byte> buffer(sizeof(IoctlCreateVirtualStorageParameter) + size, 0);
	auto parameter = reinterpret_cast<IoctlCreateVirtualStorageParameter*>(buffer.data());
	parameter->size = size;
	memcpy_s(parameter->path, size, path.c_str(), size);
	DWORD bytesReturned = 0;
	if (!DeviceIoControl(device, IOCTL_CREATE_VIRTUAL_STORAGE_CODE, buffer.data(), buffer.size(), nullptr, 0,
						 &bytesReturned, nullptr)) {
		auto gle = GetLastError();
		cerr << "DeviceIoControl failed, GLE=" << gle << endl;
	}
	cout << "Closing device" << endl;
	CloseHandle(device);
	return 0;
}
