#include "VirtualStorageIoctlFunctions.h"
#include <ntdddisk.h>
#include <mountdev.h>
#include <ntddvol.h>
#include "UtilFunctions.h"

namespace virtualStorage {

NTSTATUS ioctlDispatchGetLength(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	GET_LENGTH_INFORMATION* outputInfo = nullptr;
	ULONG_PTR dataWritten = 0;
	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	TRACE("ioctlDispatchGetLength called");
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(GET_LENGTH_INFORMATION)), "Buffer verification failed");
	outputInfo = reinterpret_cast<GET_LENGTH_INFORMATION*>(outputBuffer);
	outputInfo->Length = virtualStorageExtension->file.size;
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
	LARGE_INTEGER deviceTracks;
	LARGE_INTEGER deviceCylinders;	
	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	TRACE("ioctlDispatchGetDriveGeometry called");
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(DISK_GEOMETRY)), "Buffer verification failed");
	outputInfo = reinterpret_cast<DISK_GEOMETRY*>(outputBuffer);
	deviceTracks.QuadPart = (virtualStorageExtension->file.size.QuadPart / BYTES_PER_SECTOR) / SECTORS_PER_TRACK;
	deviceCylinders.QuadPart = deviceTracks.QuadPart / MAX_TRACK_PER_CYLINDER;
	if (!deviceCylinders.QuadPart) {
		deviceCylinders.QuadPart = 1;
	}
	if (!deviceTracks.QuadPart || deviceTracks.QuadPart > MAX_TRACK_PER_CYLINDER) {
		deviceTracks.QuadPart = MAX_TRACK_PER_CYLINDER;
	}
	outputInfo->Cylinders.QuadPart = deviceCylinders.QuadPart;
	outputInfo->MediaType = RemovableMedia;
	outputInfo->TracksPerCylinder = deviceTracks.LowPart;
	outputInfo->SectorsPerTrack = SECTORS_PER_TRACK;
	outputInfo->BytesPerSector = BYTES_PER_SECTOR;
	dataWritten = sizeof(DISK_GEOMETRY);
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = dataWritten;
	return STATUS_SUCCESS;
}

NTSTATUS ioctlDispatchNoHandleReturnSuccess(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(stackLocation);
	TRACE("ioctlDispatchNoHandleReturnSuccess called");
	irp->IoStatus.Status = STATUS_SUCCESS;
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
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(STORAGE_HOTPLUG_INFO)), "Buffer verification failed");
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
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchGetNestingLevel called");
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	int* output = nullptr;
	ULONG_PTR bytesWritten = 0;
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, NESTING_LEVEL_OUTPUT_INFO_SIZE), "Buffer verification failed");
	output = reinterpret_cast<int*>(outputBuffer);
	// These values are copied from code I found online that handles this request: https://pastebin.com/cxsN0X9u
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

NTSTATUS ioctlDispatchCheckVerify(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchCheckVerify called");
	ULONG bytesWritten = 0;
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(ULONG)), "Buffer verification failed");
	*reinterpret_cast<ULONG*>(outputBuffer) = 0;
	bytesWritten = sizeof(ULONG);
cleanup:
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = bytesWritten;
	return status;
}

NTSTATUS ioctlDispatchQueryDeviceName(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchQueryDeviceName called");
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	PMOUNTDEV_NAME output = nullptr;
	ULONG_PTR bytesWritten = 0;
	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(MOUNTDEV_NAME)), "Buffer verification failed");
	output = reinterpret_cast<PMOUNTDEV_NAME>(outputBuffer);
	CHECK_STATUS(verifyDeviceExtensionHeader(deviceObject), "verifyDeviceExtensionHeader");
	bytesWritten = sizeof(MOUNTDEV_NAME);
	output->NameLength = virtualStorageExtension->deviceName.Length;
	CHECK_AND_SET_STATUS(length >= static_cast<ULONG>(FIELD_OFFSET(MOUNTDEV_NAME, Name)) + virtualStorageExtension->deviceName.Length, STATUS_BUFFER_OVERFLOW, "Output buffer too small");
	RtlCopyMemory(output->Name, virtualStorageExtension->deviceName.Buffer, virtualStorageExtension->deviceName.Length);
	bytesWritten = FIELD_OFFSET(MOUNTDEV_NAME, Name) + output->NameLength;
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesWritten;
	return status;
}

NTSTATUS ioctlDispatchQueryDeviceUniqueId(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchQueryDeviceName called");
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	PMOUNTDEV_UNIQUE_ID output = nullptr;
	ULONG_PTR bytesWritten = 0;
	GUID deviceUniqueGuid;
	UNICODE_STRING uniqueId = { 0, 0, nullptr };
	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(MOUNTDEV_UNIQUE_ID)), "Buffer verification failed");
	output = reinterpret_cast<PMOUNTDEV_UNIQUE_ID>(outputBuffer);
	CHECK_STATUS(verifyDeviceExtensionHeader(deviceObject), "verifyDeviceExtensionHeader");
	deviceUniqueGuid = DEVICE_UNIQUE_ID(virtualStorageExtension->id);
	CHECK_STATUS(RtlStringFromGUID(deviceUniqueGuid, &uniqueId), "RtlStringFromGUID failed");
	bytesWritten = sizeof(PMOUNTDEV_UNIQUE_ID);
	output->UniqueIdLength = uniqueId.Length;
	CHECK_AND_SET_STATUS(length >= static_cast<ULONG>(FIELD_OFFSET(MOUNTDEV_UNIQUE_ID, UniqueId)) + uniqueId.Length, STATUS_BUFFER_OVERFLOW, "Output buffer too small");
	RtlCopyMemory(output->UniqueId, uniqueId.Buffer, uniqueId.Length);
	bytesWritten = FIELD_OFFSET(MOUNTDEV_UNIQUE_ID, UniqueId) + output->UniqueIdLength;
cleanup:
	if (uniqueId.Buffer) {
		RtlFreeUnicodeString(&uniqueId);
	}
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesWritten;
	return status;
}

NTSTATUS ioctlDispatchVolumeIsDynamic(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchVolumeIsDynamic called");
	ULONG bytesWritten = 0;
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(BOOLEAN)), "Buffer verification failed");
	*reinterpret_cast<BOOLEAN*>(outputBuffer) = FALSE;
	bytesWritten = sizeof(BOOLEAN);
cleanup:
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = bytesWritten;
	return status;
}

NTSTATUS ioctlDispatchDiskExtents(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchDiskExtents called");
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	PVOLUME_DISK_EXTENTS output = nullptr;
	ULONG_PTR bytesWritten = 0;
	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(VOLUME_DISK_EXTENTS)), "Buffer verification failed");
	output = reinterpret_cast<PVOLUME_DISK_EXTENTS>(outputBuffer);
	bytesWritten = sizeof(VOLUME_DISK_EXTENTS);
	output->NumberOfDiskExtents = 1;
	output->Extents[0].DiskNumber = 0;
	output->Extents[0].StartingOffset.QuadPart = 0;
	output->Extents[0].ExtentLength = virtualStorageExtension->file.size;
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesWritten;
	return status;
}

NTSTATUS ioctlDispatchGetPartitionInfo(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchGetPartitionInfo called");
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	PPARTITION_INFORMATION_EX output = nullptr;
	ULONG_PTR bytesWritten = 0;
	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(PARTITION_INFORMATION_EX)), "Buffer verification failed");
	output = reinterpret_cast<PPARTITION_INFORMATION_EX>(outputBuffer);
	bytesWritten = sizeof(PARTITION_INFORMATION_EX);
	output->PartitionStyle = PARTITION_STYLE_RAW;
	output->StartingOffset.QuadPart = 0;
	output->PartitionLength = virtualStorageExtension->file.size;
	output->RewritePartition = false;
#if (NTDDI_VERSION >= NTDDI_WIN10_RS3)  /* ABRACADABRA_WIN10_RS3 */
	output->IsServicePartition = false;
#endif
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesWritten;
	return status;
}

}
