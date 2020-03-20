#pragma once
#include "Utils.h"

NTSTATUS openFile(PUNICODE_STRING fileName, ULONG desiredAccess, ULONG shareAccess, _Out_ PHANDLE fileHandle);

NTSTATUS getDeviceTypeName(PDEVICE_OBJECT deviceObject, PCHAR* deviceTypeName);