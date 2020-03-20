#include "UtilFunctions.h"

NTSTATUS openFile(PUNICODE_STRING fileName, ULONG desiredAccess, ULONG shareAccess, _Out_ PHANDLE fileHandle) {
	NTSTATUS status = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES objectAttributes;
	IO_STATUS_BLOCK statusBlock;
	TRACE("Opening file %wZ", fileName);
	InitializeObjectAttributes(&objectAttributes, fileName, OBJ_KERNEL_HANDLE, nullptr, nullptr);
	CHECK_STATUS(ZwOpenFile(fileHandle, desiredAccess, &objectAttributes, &statusBlock, shareAccess, FILE_NON_DIRECTORY_FILE), "ZwOpenFile failed");
cleanup:
	return status;
}

NTSTATUS getDeviceTypeName(PDEVICE_OBJECT deviceObject, PCHAR* deviceTypeName) {
	NTSTATUS status = STATUS_SUCCESS;
	const auto deviceExtensionHeader = reinterpret_cast<DeviceExtensionHeader*>(deviceObject->DeviceExtension);
	CHECK_AND_SET_STATUS(deviceExtensionHeader, STATUS_BAD_DATA, "Device has no device extension");
	CHECK_AND_SET_STATUS(deviceExtensionHeader->type < DeviceType::MAX_DEVICE_TYPE, STATUS_BAD_DATA, "Device type out of range");
	CHECK_AND_SET_STATUS(deviceTypeName, STATUS_INVALID_PARAMETER_2, "deviceTypeName can't be null");
	switch (deviceExtensionHeader->type) {
	case DeviceType::MANAGER:
		*deviceTypeName = reinterpret_cast<char*>("Manager");
		break;
	case DeviceType::VIRTUAL_STORAGE:
		*deviceTypeName = reinterpret_cast<char*>("VirtualStorage");
		break;
	default:
		TRACE("ERROR: Unreachable code reached");
		break;
	}
cleanup:
	return status;
}
