#include "VirtualStorageIoctlFunctions.h"
#include <ntdddisk.h>

namespace virtualStorage {

NTSTATUS ioctlDispatchGetLength(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	GET_LENGTH_INFORMATION* outputInfo = nullptr;
	ULONG_PTR dataWritten = 0;
	TRACE("ioctlDispatchGetLength called");
	CHECK_AND_SET_STATUS((outputBuffer && length), STATUS_INVALID_PARAMETER, "No output buffer provided");
	CHECK_AND_SET_STATUS((length >= sizeof(GET_LENGTH_INFORMATION)), STATUS_BUFFER_TOO_SMALL, "Output buffer too small");
	outputInfo = reinterpret_cast<GET_LENGTH_INFORMATION*>(outputBuffer);
	// TODO: get length from file
	outputInfo->Length.QuadPart = 1024 * 1024; // 1MB
	dataWritten = sizeof(GET_LENGTH_INFORMATION);
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = dataWritten;
	return STATUS_SUCCESS;
}

NTSTATUS ioctlDispatchGetDriveGeometry(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	DISK_GEOMETRY* outputInfo = nullptr;
	ULONG_PTR dataWritten = 0;
	TRACE("ioctlDispatchGetDriveGeometry called");
	CHECK_AND_SET_STATUS((outputBuffer && length), STATUS_INVALID_PARAMETER, "No output buffer provided");
	CHECK_AND_SET_STATUS((length >= sizeof(DISK_GEOMETRY)), STATUS_BUFFER_TOO_SMALL, "Output buffer too small");
	outputInfo = reinterpret_cast<DISK_GEOMETRY*>(outputBuffer);
	outputInfo->Cylinders.QuadPart = 1;
	outputInfo->MediaType = RemovableMedia;
	outputInfo->BytesPerSector = BYTES_PER_SECTOR;
	outputInfo->SectorsPerTrack = 1;
	// TODO: get length from file
	outputInfo->TracksPerCylinder = (1024 * 1024) / BYTES_PER_SECTOR;
	dataWritten = sizeof(DISK_GEOMETRY);
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = dataWritten;
	return STATUS_SUCCESS;
}

NTSTATUS ioctlDispatchMediaRemoval(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(stackLocation);
	TRACE("ioctlDispatchMediaRemoval called");
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	return STATUS_SUCCESS;
}

NTSTATUS ioctlDispatchDiskIsWritable(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(stackLocation);
	TRACE("ioctlDispatchDiskIsWritable called");
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	return STATUS_SUCCESS;
}

NTSTATUS ioctlDispatchQueryProperty(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(stackLocation);
	TRACE("ioctlDispatchQueryProperty called");
	irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	irp->IoStatus.Information = 0;
	return STATUS_SUCCESS;
}

NTSTATUS ioctlDispatchGetHotplugInfo(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	STORAGE_HOTPLUG_INFO* outputInfo = nullptr;
	ULONG_PTR dataWritten = 0;
	TRACE("ioctlDispatchGetHotplugInfo called");
	CHECK_AND_SET_STATUS((outputBuffer && length), STATUS_INVALID_PARAMETER, "No output buffer provided");
	CHECK_AND_SET_STATUS((length >= sizeof(STORAGE_HOTPLUG_INFO)), STATUS_BUFFER_TOO_SMALL, "Output buffer too small");
	outputInfo = reinterpret_cast<STORAGE_HOTPLUG_INFO*>(outputBuffer);
	outputInfo->Size = sizeof(STORAGE_HOTPLUG_INFO);
	outputInfo->MediaRemovable = true;
	outputInfo->MediaHotplug = true;
	outputInfo->DeviceHotplug = true;
	outputInfo->WriteCacheEnableOverride = NULL;
	dataWritten = sizeof(STORAGE_HOTPLUG_INFO);
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = dataWritten;
	return STATUS_SUCCESS;
}

// Obtained from ida
#define NESTING_LEVEL_OUTPUT_INFO_SIZE 0x14

NTSTATUS ioctlDispatchGetNestingLevel(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(stackLocation);
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchGetNestingLevel called");
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	int* output = nullptr;
	ULONG_PTR bytesWritten = 0;
	CHECK_AND_SET_STATUS((outputBuffer && length), STATUS_INVALID_PARAMETER, "No output buffer provided");
	CHECK_AND_SET_STATUS((length >= NESTING_LEVEL_OUTPUT_INFO_SIZE), STATUS_BUFFER_TOO_SMALL, "Output buffer too small");
	output = reinterpret_cast<int*>(outputBuffer);
	// These values are copied from code I found online that handles this request
	output[0] = 1;
	output[0] = 0;
	output[0] = 0x402;
	output[0] = 0;
	output[0] = 1;
	bytesWritten = length;
cleanup:
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = bytesWritten;
	return STATUS_SUCCESS;
}

}
