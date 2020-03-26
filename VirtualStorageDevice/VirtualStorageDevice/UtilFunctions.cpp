#include "UtilFunctions.h"
#include <ntstrsafe.h>

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

NTSTATUS verifyDeviceExtensionHeader(PDEVICE_OBJECT deviceObject) {
	NTSTATUS status = STATUS_SUCCESS;
	const auto deviceExtensionHeader = reinterpret_cast<PDeviceExtensionHeader>(deviceObject->DeviceExtension);
	CHECK_AND_SET_STATUS(deviceExtensionHeader, STATUS_BAD_DATA, "Device has no device extension");
	CHECK_AND_SET_STATUS(deviceExtensionHeader->magic == DEVICE_EXTENSION_HEADER_MAGIC, STATUS_BAD_DATA, "Device doesn't start with magic");
	CHECK_AND_SET_STATUS(deviceExtensionHeader->type < DeviceType::MAX_DEVICE_TYPE, STATUS_BAD_DATA, "Device type out of range");
cleanup:
	return status;
}

NTSTATUS getDeviceTypeName(PDEVICE_OBJECT deviceObject, PCHAR* deviceTypeName) {
	NTSTATUS status = STATUS_SUCCESS;
	const auto deviceExtensionHeader = reinterpret_cast<PDeviceExtensionHeader>(deviceObject->DeviceExtension);
	CHECK_STATUS(verifyDeviceExtensionHeader(deviceObject), "verifyDeviceExtensionHeader failed");
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

#define LENGTH_OF_08LX_FORMAT 4

NTSTATUS idToDeviceName(ULONG id, _Out_ PUNICODE_STRING deviceName) {
	NTSTATUS status = STATUS_SUCCESS;
	WCHAR deviceNameFormat[] = L"\\Device\\VirtualStorageDevice#%08lx";
	WCHAR* deviceNameWstring = nullptr;
	// The maximum length of the string is the length of name format (%lx subtracted as it doesn't appear in result) +
	// the maximum digits in a hex number + null terminator.
	const size_t nameLengthInWChars = COUNT_OF(deviceNameFormat) - LENGTH_OF_08LX_FORMAT + MAX_LONG_HEX_DIGITS + 1;
	const size_t nameLengthInBytes = nameLengthInWChars * sizeof(WCHAR);
	CHECK_AND_SET_STATUS(deviceName, STATUS_INVALID_PARAMETER_2, "deviceName can't be NULL");
	deviceNameWstring = reinterpret_cast<WCHAR*>(ExAllocatePoolWithTag(PagedPool, nameLengthInBytes, 'NveD'));
	CHECK_AND_SET_STATUS(deviceNameWstring, STATUS_INSUFFICIENT_RESOURCES, "ExAllocatePoolWithTag failed");
	RtlZeroMemory(deviceNameWstring, nameLengthInBytes);
	CHECK_STATUS(RtlStringCchPrintfW(deviceNameWstring, nameLengthInWChars, deviceNameFormat, id), "RtlStringCchPrintfW failed");
	RtlInitUnicodeString(deviceName, deviceNameWstring);
cleanup:
	if (!NT_SUCCESS(status)) {
		FREE_IF_NOT_NULL(deviceNameWstring, 'NveD');
	}
	return status;
}

NTSTATUS sendIoctlByDevicePointer(PDEVICE_OBJECT deviceObject, ULONG ioctlCode, PVOID inputBuffer, ULONG inputBufferLength, 
								  PVOID outputBuffer, ULONG outputBufferLength, _Out_ ULONG* outputBufferFinalLength) {

	NTSTATUS status = STATUS_SUCCESS;
	PIRP irp = nullptr;
	KEVENT finishedEvent;
	IO_STATUS_BLOCK statusBlock = {STATUS_SUCCESS, 0};
	LARGE_INTEGER maxTimeout;
	CHECK_AND_SET_STATUS(outputBufferFinalLength, STATUS_INVALID_PARAMETER_7, "Must provide output buffer's output length");
	KeInitializeEvent(&finishedEvent, NotificationEvent, false);
	irp = IoBuildDeviceIoControlRequest(ioctlCode, deviceObject, inputBuffer, inputBufferLength, outputBuffer,
						  outputBufferLength, false, &finishedEvent, &statusBlock);
	CHECK_AND_SET_STATUS(irp, STATUS_UNSUCCESSFUL, "IoBuildDeviceIoControlRequest failed");
	CHECK_STATUS(IoCallDriver(deviceObject, irp), "IoCallDriver failed");
	if (status == STATUS_PENDING) {
		maxTimeout.QuadPart = MAX_TIMEOUT_IOCTL;
		status = KeWaitForSingleObject(&finishedEvent, Executive, KernelMode, false, &maxTimeout);
		CHECK_AND_SET_STATUS(status == STATUS_SUCCESS, STATUS_UNSUCCESSFUL, "KeWaitForSingleObject failed");
	}
cleanup:
	if (NT_SUCCESS(status)) {
		status = statusBlock.Status;
		*outputBufferFinalLength = statusBlock.Information;
	}
	return status;
}

NTSTATUS sendIoctlByDeviceName(PCWSTR deviceNamePtr, ULONG ioctlCode, PVOID inputBuffer, ULONG inputBufferLength,
							   PVOID outputBuffer, ULONG outputBufferLength, _Out_ ULONG* outputBufferFinalLength) {

	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING deviceName = { 0, 0, nullptr };
	PFILE_OBJECT fileObject = nullptr;
	PDEVICE_OBJECT deviceObject = nullptr;
	CHECK_AND_SET_STATUS(deviceNamePtr, STATUS_INVALID_PARAMETER_1, "Must provide device name");
	RtlInitUnicodeString(&deviceName, deviceNamePtr);
	CHECK_STATUS(IoGetDeviceObjectPointer(&deviceName, FILE_READ_ACCESS | FILE_WRITE_ACCESS, &fileObject, &deviceObject), "IoGetDeviceObjectPointer failed");
	CHECK_STATUS(sendIoctlByDevicePointer(deviceObject, ioctlCode, inputBuffer, inputBufferLength, outputBuffer, outputBufferLength, outputBufferFinalLength), "sendIoctlByDevicePointer failed");
cleanup:
	DELETE_IF_NOT_NULL_MACRO(fileObject, ObDereferenceObject);
	return status;
}

NTSTATUS verifyBufferSize(PVOID buffer, ULONG bufferSize, ULONG requiredSize) {
	NTSTATUS status = STATUS_SUCCESS;
	CHECK_AND_SET_STATUS((buffer && bufferSize), STATUS_INVALID_PARAMETER, "No buffer provided");
	CHECK_AND_SET_STATUS((bufferSize >= requiredSize), STATUS_BUFFER_TOO_SMALL, "Buffer too small");
cleanup:
	return status;
}

NTSTATUS verifyFileSize(LARGE_INTEGER fileSize, ULONG bytesPerSector, ULONG sectorsPerTrack, ULONG maxTracksPerCylinder) {
	NTSTATUS status = STATUS_SUCCESS;
	LARGE_INTEGER deviceSectors;
	LARGE_INTEGER deviceTracks;
	LARGE_INTEGER deviceCylinders;
	deviceSectors.QuadPart = fileSize.QuadPart / bytesPerSector;
	CHECK_AND_SET_STATUS(deviceSectors.QuadPart && (deviceSectors.LowPart & (sectorsPerTrack - 1)) == 0, STATUS_BAD_DATA, "File size is not a multiple of (bytesPerSector * sectorsPerTrack)");
	deviceTracks.QuadPart = deviceSectors.QuadPart / sectorsPerTrack;
	if (deviceTracks.QuadPart > maxTracksPerCylinder) {
		deviceCylinders.QuadPart = deviceTracks.QuadPart / maxTracksPerCylinder;
		CHECK_AND_SET_STATUS((deviceCylinders.QuadPart * maxTracksPerCylinder) == deviceTracks.QuadPart, STATUS_BAD_DATA,
							 "File size is not a multiple of (bytesPerSector * sectorsPerTrack * maxTracksPerCylinder)");
	}
cleanup:
	return status;
}
