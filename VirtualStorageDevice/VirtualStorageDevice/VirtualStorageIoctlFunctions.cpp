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
	PDISK_GEOMETRY outputInfo = nullptr;
	ULONG_PTR dataWritten = 0;
	LARGE_INTEGER deviceTracks;
	LARGE_INTEGER deviceCylinders;	
	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	TRACE("ioctlDispatchGetDriveGeometry called");
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(DISK_GEOMETRY)), "Buffer verification failed");
	outputInfo = reinterpret_cast<PDISK_GEOMETRY>(outputBuffer);
	outputInfo->MediaType = RemovableMedia;
	FILL_DISK_PHYSICAL_INFO(*outputInfo, virtualStorageExtension->file.size);
	dataWritten = sizeof(DISK_GEOMETRY);
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = dataWritten;
	return STATUS_SUCCESS;
}

NTSTATUS ioctlDispatchGetDriveGeometryEx(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	PDISK_GEOMETRY_EX outputInfo = nullptr;
	ULONG_PTR dataWritten = 0;
	LARGE_INTEGER deviceTracks;
	LARGE_INTEGER deviceCylinders;
	PDISK_PARTITION_INFO diskPartitionInfo = nullptr;
	PDISK_DETECTION_INFO diskDetectionInfo = nullptr;
	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	TRACE("ioctlDispatchGetDriveGeometryEx called");
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(DISK_GEOMETRY) + sizeof(LARGE_INTEGER)), "Buffer verification failed");
	outputInfo = reinterpret_cast<PDISK_GEOMETRY_EX>(outputBuffer);
	FILL_DISK_PHYSICAL_INFO(outputInfo->Geometry, virtualStorageExtension->file.size);
	outputInfo->DiskSize = virtualStorageExtension->file.size;
	dataWritten = sizeof(DISK_GEOMETRY) + sizeof(LARGE_INTEGER);
	CHECK_AND_SET_STATUS(length >= (sizeof(DISK_GEOMETRY) + sizeof(LARGE_INTEGER) + sizeof(DISK_PARTITION_INFO)), STATUS_BUFFER_TOO_SMALL, "Not adding disk partition info");
	diskPartitionInfo = reinterpret_cast<PDISK_PARTITION_INFO>(outputInfo->Data);
	diskPartitionInfo->SizeOfPartitionInfo = sizeof(DISK_PARTITION_INFO);
	diskPartitionInfo->PartitionStyle = PARTITION_STYLE_RAW;
	dataWritten += sizeof(DISK_PARTITION_INFO);
	CHECK_AND_SET_STATUS(length >= (sizeof(DISK_GEOMETRY) + sizeof(LARGE_INTEGER) + sizeof(DISK_PARTITION_INFO) + sizeof(DISK_DETECTION_INFO)), STATUS_BUFFER_TOO_SMALL, "Not adding disk detection info");
	diskDetectionInfo = reinterpret_cast<PDISK_DETECTION_INFO>(reinterpret_cast<char*>(outputInfo->Data) + sizeof(DISK_PARTITION_INFO));
	diskDetectionInfo->SizeOfDetectInfo = 0;
	diskDetectionInfo->DetectionType = DetectNone;
	dataWritten += sizeof(DISK_DETECTION_INFO);
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
	output->PartitionNumber = 0;
	output->RewritePartition = false;
#if (NTDDI_VERSION >= NTDDI_WIN10_RS3)  /* ABRACADABRA_WIN10_RS3 */
	output->IsServicePartition = false;
#endif
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesWritten;
	return status;
}

NTSTATUS ioctlDispatchGetDriveLayout(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchGetDriveLayout called");
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	PDRIVE_LAYOUT_INFORMATION output = nullptr;
	ULONG_PTR bytesWritten = 0;
	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(DRIVE_LAYOUT_INFORMATION)), "Buffer verification failed");
	output = reinterpret_cast<PDRIVE_LAYOUT_INFORMATION>(outputBuffer);
	bytesWritten = sizeof(DRIVE_LAYOUT_INFORMATION);
	output->PartitionCount = 1;
	output->Signature = 0;
	output->PartitionEntry[0].StartingOffset.QuadPart = 0;
	output->PartitionEntry[0].PartitionLength = virtualStorageExtension->file.size;
	output->PartitionEntry[0].HiddenSectors = 0;
	output->PartitionEntry[0].PartitionNumber = 0;
	output->PartitionEntry[0].BootIndicator = false;
	output->PartitionEntry[0].RecognizedPartition = true;
	output->PartitionEntry[0].RewritePartition = false;
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesWritten;
	return status;
}

NTSTATUS ioctlDispatchGetDriveLayoutEx(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchGetDriveLayoutEx called");
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	PDRIVE_LAYOUT_INFORMATION_EX output = nullptr;
	ULONG_PTR bytesWritten = 0;
	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(DRIVE_LAYOUT_INFORMATION_EX)), "Buffer verification failed");
	output = reinterpret_cast<PDRIVE_LAYOUT_INFORMATION_EX>(outputBuffer);
	bytesWritten = sizeof(DRIVE_LAYOUT_INFORMATION_EX);
	output->PartitionStyle = PARTITION_STYLE_RAW;
	output->PartitionCount = 1;
	output->PartitionEntry[0].PartitionStyle = PARTITION_STYLE_RAW;
	output->PartitionEntry[0].StartingOffset.QuadPart = 0;
	output->PartitionEntry[0].PartitionLength = virtualStorageExtension->file.size;
	output->PartitionEntry[0].PartitionNumber = 0;
	output->PartitionEntry[0].RewritePartition = false;
#if (NTDDI_VERSION >= NTDDI_WIN10_RS3)  /* ABRACADABRA_WIN10_RS3 */
	output->IsServicePartition = false;
#endif
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesWritten;
	return status;
}

NTSTATUS ioctlDispatchGetDeviceNumber(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	// __debugbreak();
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchGetDeviceNumber called");
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	PSTORAGE_DEVICE_NUMBER output = nullptr;
	ULONG_PTR bytesWritten = 0;
	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(STORAGE_DEVICE_NUMBER)), "Buffer verification failed");
	output = reinterpret_cast<PSTORAGE_DEVICE_NUMBER>(outputBuffer);
	bytesWritten = sizeof(STORAGE_DEVICE_NUMBER);
	output->DeviceType = deviceObject->DeviceType;
	output->DeviceNumber = DEVICE_EXTENSION_HEADER_MAGIC + virtualStorageExtension->id;
	output->PartitionNumber = 0;
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesWritten;
	return status;
}

NTSTATUS ioctlDispatchGetMediaTypes(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	// __debugbreak();
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchGetMediaTypes called");
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	PGET_MEDIA_TYPES output = nullptr;
	ULONG_PTR bytesWritten = 0;
	LARGE_INTEGER deviceTracks;
	LARGE_INTEGER deviceCylinders;
	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(GET_MEDIA_TYPES)), "Buffer verification failed");
	output = reinterpret_cast<PGET_MEDIA_TYPES>(outputBuffer);
	bytesWritten = sizeof(GET_MEDIA_TYPES);
	output->DeviceType = deviceObject->DeviceType;
	output->MediaInfoCount = 1;
	output->MediaInfo->DeviceSpecific.RemovableDiskInfo.MediaType = static_cast<STORAGE_MEDIA_TYPE>(FixedMedia); // Looks like I need to lie to vds.exe
	output->MediaInfo->DeviceSpecific.RemovableDiskInfo.NumberMediaSides = 1;
	output->MediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics = MEDIA_READ_WRITE | MEDIA_CURRENTLY_MOUNTED;
	FILL_DISK_PHYSICAL_INFO(output->MediaInfo->DeviceSpecific.RemovableDiskInfo, virtualStorageExtension->file.size);
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesWritten;
	return status;
}

NTSTATUS ioctlDispatchIsDiskClustered(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchIsDiskClustered called");
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

NTSTATUS ioctlDispatchGetDiskAttributes(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchGetDiskAttributes called");
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	PGET_DISK_ATTRIBUTES  output = nullptr;
	ULONG_PTR bytesWritten = 0;
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(GET_DISK_ATTRIBUTES)), "Buffer verification failed");
	output = reinterpret_cast<PGET_DISK_ATTRIBUTES>(outputBuffer);
	bytesWritten = sizeof(GET_DISK_ATTRIBUTES);
	output->Version = sizeof(GET_DISK_ATTRIBUTES);
	output->Reserved1 = 0;
	output->Attributes = 0;
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesWritten;
	return status;
}

// Obtained from IDA
#define GET_DISK_FLAGS_SIZE 0x14

NTSTATUS ioctlDispatchGetDiskFlags(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("ioctlDispatchGetDiskFlags called");
	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG_PTR bytesWritten = 0;
	CHECK_STATUS(verifyBufferSize(outputBuffer, length, GET_DISK_FLAGS_SIZE), "Buffer verification failed");
	bytesWritten = GET_DISK_FLAGS_SIZE;
	// Looks like the only usage is to check if some flags are true, none of them relevant to us
	RtlZeroMemory(outputBuffer, GET_DISK_FLAGS_SIZE);
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesWritten;
	return status;
}

}
