#pragma once
// This file contains constants and data structures that are shared between the user and kernel side

// Before including this file you must first include "Windows.h" for user side or "ntddk.h" for kernel side

#define MY_VENDOR_CODE 0x8000
#define IOCTL_CREATE_VIRTUAL_STORAGE_CODE CTL_CODE(MY_VENDOR_CODE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma warning(push)
#pragma warning(disable: 4200)
struct IoctlCreateVirtualStorageParameter {
	USHORT size;
	WCHAR path[0];
};
#pragma warning(pop)