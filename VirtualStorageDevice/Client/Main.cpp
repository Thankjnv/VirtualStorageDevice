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
using std::wcin;
using std::wcout;
using std::endl;


#pragma pack(push, 0)
struct GetLogicalDrivesStringEntry {
	wchar_t driveLetter;
	wchar_t semicolon;
	wchar_t slash;
	wchar_t nullByte;
};

typedef struct GetLogicalDrivesStringResponse {
	GetLogicalDrivesStringEntry driversStringEntry[0];
} *PGetLogicalDrivesStringResponse;
#pragma pack(pop)

int wmain() {
	wstring deviceName = L"\\\\.\\VirtualStorageDeviceManager";
	HANDLE device = CreateFile(deviceName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ,
		nullptr, OPEN_EXISTING, 0, nullptr);
	if (device == INVALID_HANDLE_VALUE) {
		auto gle = GetLastError();
		cerr << "CreateFile failed, GLE=" << gle << endl;
		return 1;
	}

	wstring name;
	wcin >> name;
	
	wstring path = L"\\Device\\HarddiskVolume1\\Temp\\" + name + L".txt";
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

	vector<wchar_t> data(256, 0);
	auto resultLength = GetLogicalDriveStringsW(data.size(), data.data());
	auto numOfDrives = (resultLength * sizeof(data[0])) / sizeof(GetLogicalDrivesStringEntry);
	auto logicalDriveStrings = reinterpret_cast<PGetLogicalDrivesStringResponse>(data.data());
	for (auto i = 0; i < numOfDrives; ++i) {
		auto driveType = GetDriveTypeW(reinterpret_cast<LPCWSTR>(&(logicalDriveStrings->driversStringEntry[i])));
		const wchar_t* driveTypeName = nullptr;
		switch (driveType) {
		case DRIVE_NO_ROOT_DIR:
			driveTypeName = L"NO_ROOT_DIR";
			break;
		case DRIVE_REMOVABLE:
			driveTypeName = L"REMOVABLE";
			break;
		case DRIVE_FIXED:
			driveTypeName = L"FIXED";
			break;
		case DRIVE_REMOTE:
			driveTypeName = L"REMOTE";
			break;
		case DRIVE_CDROM:
			driveTypeName = L"CDROM";
			break;
		case DRIVE_RAMDISK:
			driveTypeName = L"RAMDISK";
			break;
		case DRIVE_UNKNOWN:
			// Intentional fall through
		default:
			driveTypeName = L"UNKNOWN";
			break;
		}
		wcout << L"Logical drive: " << logicalDriveStrings->driversStringEntry[i].driveLetter << L" of type " << driveTypeName << endl;
	}
	return 0;
}
