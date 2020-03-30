[1mdiff --git a/VirtualStorageDevice/VirtualStorageDevice/Main.cpp b/VirtualStorageDevice/VirtualStorageDevice/Main.cpp[m
[1mindex 6abcea2..26f07be 100644[m
[1m--- a/VirtualStorageDevice/VirtualStorageDevice/Main.cpp[m
[1m+++ b/VirtualStorageDevice/VirtualStorageDevice/Main.cpp[m
[36m@@ -29,11 +29,17 @@[m [mvoid driverCleanup(PDRIVER_OBJECT driverObject) {[m
 			}[m
 			virtualStorageExtension->file.handle = nullptr;[m
 			DELETE_IF_NOT_NULL(virtualStorageExtension->lowerLevelDevice, IoDetachDevice);[m
[31m-			if (virtualStorageExtension->symbolicLinkName.Buffer) {[m
[31m-				IoSetDeviceInterfaceState(&virtualStorageExtension->symbolicLinkName, false);[m
[31m-				TRACE("Deleting PDO symbolic link name");[m
[31m-				RtlFreeUnicodeString(&virtualStorageExtension->symbolicLinkName);[m
[31m-				RtlZeroMemory(&virtualStorageExtension->symbolicLinkName, sizeof(UNICODE_STRING));[m
[32m+[m			[32mif (virtualStorageExtension->mountSymbolicLinkName.Buffer) {[m[41m[m
[32m+[m				[32mIoSetDeviceInterfaceState(&virtualStorageExtension->mountSymbolicLinkName, false);[m[41m[m
[32m+[m				[32mTRACE("Deleting PDO mount symbolic link name");[m[41m[m
[32m+[m				[32mRtlFreeUnicodeString(&virtualStorageExtension->mountSymbolicLinkName);[m[41m[m
[32m+[m				[32mRtlZeroMemory(&virtualStorageExtension->mountSymbolicLinkName, sizeof(UNICODE_STRING));[m[41m[m
[32m+[m			[32m}[m[41m[m
[32m+[m			[32mif (virtualStorageExtension->diskSymbolicLinkName.Buffer) {[m[41m[m
[32m+[m				[32mIoSetDeviceInterfaceState(&virtualStorageExtension->diskSymbolicLinkName, false);[m[41m[m
[32m+[m				[32mTRACE("Deleting PDO device symbolic link name");[m[41m[m
[32m+[m				[32mRtlFreeUnicodeString(&virtualStorageExtension->diskSymbolicLinkName);[m[41m[m
[32m+[m				[32mRtlZeroMemory(&virtualStorageExtension->diskSymbolicLinkName, sizeof(UNICODE_STRING));[m[41m[m
 			}[m
 			FREE_IF_NOT_NULL(virtualStorageExtension->deviceName.Buffer, 'NveD');[m
 			TRACE("Deleting PDO");[m
[1mdiff --git a/VirtualStorageDevice/VirtualStorageDevice/ManagerIoctlFunctions.cpp b/VirtualStorageDevice/VirtualStorageDevice/ManagerIoctlFunctions.cpp[m
[1mindex 7067db4..c0967e0 100644[m
[1m--- a/VirtualStorageDevice/VirtualStorageDevice/ManagerIoctlFunctions.cpp[m
[1m+++ b/VirtualStorageDevice/VirtualStorageDevice/ManagerIoctlFunctions.cpp[m
[36m@@ -1,6 +1,7 @@[m
 #include "ManagerIoctlFunctions.h"[m
 #include "Common.h"[m
 #include "UtilFunctions.h"[m
[32m+[m[32m#include "ntddstor.h"[m[41m[m
 [m
 namespace manager {[m
 [m
[36m@@ -75,8 +76,12 @@[m [mNTSTATUS ioctlDispatchCreateVirtualStorage(PDEVICE_OBJECT deviceObject, PIRP irp[m
 	}[m
 	[m
 	deviceExtension->lowerLevelDevice = IoAttachDeviceToDeviceStack(fdoDevice, deviceExtension->pdoDevice);[m
[31m-	CHECK_STATUS(IoRegisterDeviceInterface(deviceExtension->pdoDevice, &MOUNTDEV_MOUNTED_DEVICE_GUID, nullptr, &deviceExtension->symbolicLinkName), "IoRegisterDeviceInterface failed");[m
[31m-	CHECK_STATUS(IoSetDeviceInterfaceState(&deviceExtension->symbolicLinkName, true), "IoSetDeviceInterfaceState failed");[m
[32m+[m	[32mCHECK_STATUS(IoRegisterDeviceInterface(deviceExtension->pdoDevice, &GUID_DEVINTERFACE_DISK, nullptr, &deviceExtension->diskSymbolicLinkName), "IoRegisterDeviceInterface failed");[m[41m[m
[32m+[m	[32mCHECK_STATUS(IoSetDeviceInterfaceState(&deviceExtension->diskSymbolicLinkName, true), "IoSetDeviceInterfaceState failed");[m[41m[m
[32m+[m[41m[m
[32m+[m	[32mCHECK_STATUS(IoRegisterDeviceInterface(deviceExtension->pdoDevice, &MOUNTDEV_MOUNTED_DEVICE_GUID, nullptr, &deviceExtension->mountSymbolicLinkName), "IoRegisterDeviceInterface failed");[m[41m[m
[32m+[m	[32mCHECK_STATUS(IoSetDeviceInterfaceState(&deviceExtension->mountSymbolicLinkName, true), "IoSetDeviceInterfaceState failed");[m[41m[m
[32m+[m[41m	[m
 	CHECK_STATUS(IoVerifyVolume(fdoDevice, true), "IoVerifyVolume failed");[m
 cleanup:[m
 	DELETE_IF_NOT_NULL_MACRO(fileObject, ObDereferenceObject);[m
[36m@@ -84,10 +89,15 @@[m [mcleanup:[m
 	if (!NT_SUCCESS(status)) {[m
 		if (deviceExtension) {[m
 			DELETE_IF_NOT_NULL(deviceExtension->lowerLevelDevice, IoDetachDevice);[m
[31m-			if (deviceExtension->symbolicLinkName.Buffer) {[m
[31m-				IoSetDeviceInterfaceState(&deviceExtension->symbolicLinkName, false);[m
[31m-				RtlFreeUnicodeString(&deviceExtension->symbolicLinkName);[m
[31m-				RtlZeroMemory(&deviceExtension->symbolicLinkName, sizeof(UNICODE_STRING));[m
[32m+[m			[32mif (deviceExtension->mountSymbolicLinkName.Buffer) {[m[41m[m
[32m+[m				[32mIoSetDeviceInterfaceState(&deviceExtension->mountSymbolicLinkName, false);[m[41m[m
[32m+[m				[32mRtlFreeUnicodeString(&deviceExtension->mountSymbolicLinkName);[m[41m[m
[32m+[m				[32mRtlZeroMemory(&deviceExtension->mountSymbolicLinkName, sizeof(UNICODE_STRING));[m[41m[m
[32m+[m			[32m}[m[41m[m
[32m+[m			[32mif (deviceExtension->diskSymbolicLinkName.Buffer) {[m[41m[m
[32m+[m				[32mIoSetDeviceInterfaceState(&deviceExtension->diskSymbolicLinkName, false);[m[41m[m
[32m+[m				[32mRtlFreeUnicodeString(&deviceExtension->diskSymbolicLinkName);[m[41m[m
[32m+[m				[32mRtlZeroMemory(&deviceExtension->diskSymbolicLinkName, sizeof(UNICODE_STRING));[m[41m[m
 			}[m
 		}[m
 		DELETE_IF_NOT_NULL(pdoDevice, IoDeleteDevice);[m
[1mdiff --git a/VirtualStorageDevice/VirtualStorageDevice/Utils.h b/VirtualStorageDevice/VirtualStorageDevice/Utils.h[m
[1mindex c4b695a..dd1c232 100644[m
[1m--- a/VirtualStorageDevice/VirtualStorageDevice/Utils.h[m
[1m+++ b/VirtualStorageDevice/VirtualStorageDevice/Utils.h[m
[36m@@ -75,6 +75,19 @@[m
 [m
 #define DEVICE_UNIQUE_ID(deviceId) _DEVICE_UNIQUE_ID(LOW_BYTE(deviceId), LOW_SECOND_BYTE(deviceId), HIGH_SECOND_BYTE(deviceId), HIGH_BYTE(deviceId))[m
 [m
[32m+[m[32m#define FILL_DISK_PHYSICAL_INFO(diskPhysicalInfo, diskSize) \[m[41m[m
[32m+[m	[32mdeviceTracks.QuadPart = ((diskSize).QuadPart / BYTES_PER_SECTOR) / SECTORS_PER_TRACK; \[m[41m[m
[32m+[m	[32mdeviceCylinders.QuadPart = deviceTracks.QuadPart / MAX_TRACK_PER_CYLINDER; \[m[41m[m
[32m+[m	[32mif (!deviceCylinders.QuadPart) { \[m[41m[m
[32m+[m		[32mdeviceCylinders.QuadPart = 1; \[m[41m[m
[32m+[m	[32m} \[m[41m[m
[32m+[m	[32mif (!deviceTracks.QuadPart || deviceTracks.QuadPart > MAX_TRACK_PER_CYLINDER) { \[m[41m[m
[32m+[m		[32mdeviceTracks.QuadPart = MAX_TRACK_PER_CYLINDER; \[m[41m[m
[32m+[m	[32m} \[m[41m[m
[32m+[m	[32m(diskPhysicalInfo).Cylinders.QuadPart = deviceCylinders.QuadPart; \[m[41m[m
[32m+[m	[32m(diskPhysicalInfo).TracksPerCylinder = deviceTracks.LowPart; \[m[41m[m
[32m+[m	[32m(diskPhysicalInfo).SectorsPerTrack = SECTORS_PER_TRACK; \[m[41m[m
[32m+[m	[32m(diskPhysicalInfo).BytesPerSector = BYTES_PER_SECTOR; \[m[41m[m
 [m
 enum class DeviceType : ULONG {[m
 	MANAGER = 0,[m
[36m@@ -104,8 +117,8 @@[m [mtypedef struct VirtualStorageDeviceExtension {[m
 	UNICODE_STRING deviceName;[m
 	PDEVICE_OBJECT pdoDevice;[m
 	PDEVICE_OBJECT lowerLevelDevice;[m
[31m-	UNICODE_STRING symbolicLinkName;[m
[31m-	[m
[32m+[m	[32mUNICODE_STRING mountSymbolicLinkName;[m[41m[m
[32m+[m	[32mUNICODE_STRING diskSymbolicLinkName;[m[41m[m
 } *PVirtualStorageDeviceExtension;[m
 [m
 extern const GUID g_deviceClassGuid;[m
[1mdiff --git a/VirtualStorageDevice/VirtualStorageDevice/VirtualStorageFunctions.cpp b/VirtualStorageDevice/VirtualStorageDevice/VirtualStorageFunctions.cpp[m
[1mindex 6e738c5..58181c7 100644[m
[1m--- a/VirtualStorageDevice/VirtualStorageDevice/VirtualStorageFunctions.cpp[m
[1m+++ b/VirtualStorageDevice/VirtualStorageDevice/VirtualStorageFunctions.cpp[m
[36m@@ -2,6 +2,7 @@[m
 #include <ntdddisk.h>[m
 #include <mountdev.h>[m
 #include <ntddvol.h>[m
[32m+[m[32m#include <ntddscsi.h>[m[41m[m
 #include "VirtualStorageIoctlFunctions.h"[m
 #include "VirtualStoragePnpFunctions.h"[m
 #include "UtilFunctions.h"[m
[36m@@ -9,6 +10,12 @@[m
 // Found this definition in some online implementation. ioctl code = 2d5190 from function FsRtlGetVirtualDiskNestingLevel[m
 #define IOCTL_STORAGE_GET_VIRTUALDISK_NESTING_LEVEL CTL_CODE(IOCTL_STORAGE_BASE, 0x0464, METHOD_BUFFERED, FILE_READ_ACCESS)[m
 [m
[32m+[m[32m// The following ioctl is not defined, found it from windbg + ida. ioctl code = 700f8[m[41m[m
[32m+[m[32m#define IOCTL_DISK_IS_DISK_CLUSTERED CTL_CODE(IOCTL_DISK_BASE, 0x003e, METHOD_BUFFERED, FILE_ANY_ACCESS)[m[41m[m
[32m+[m[41m[m
[32m+[m[32m// Same as above, ioctl code = 704010[m[41m[m
[32m+[m[32m#define IOCTL_DISK_GET_DISK_FLAGS CTL_CODE(112, 0x0004, METHOD_BUFFERED, FILE_READ_ACCESS)[m[41m[m
[32m+[m[41m[m
 namespace virtualStorage {[m
 [m
 NTSTATUS deviceDispatchIoctl(PDEVICE_OBJECT deviceObject, PIRP irp) {[m
[36m@@ -24,6 +31,9 @@[m [mNTSTATUS deviceDispatchIoctl(PDEVICE_OBJECT deviceObject, PIRP irp) {[m
 	case IOCTL_DISK_GET_DRIVE_GEOMETRY:[m
 		status = ioctlDispatchGetDriveGeometry(deviceObject, irp, stackLocation);[m
 		break;[m
[32m+[m	[32mcase IOCTL_DISK_GET_DRIVE_GEOMETRY_EX:[m[41m[m
[32m+[m		[32mstatus = ioctlDispatchGetDriveGeometryEx(deviceObject, irp, stackLocation);[m[41m[m
[32m+[m		[32mbreak;[m[41m[m
 	case IOCTL_DISK_MEDIA_REMOVAL:[m
 		status = ioctlDispatchNoHandleReturnSuccess(deviceObject, irp, stackLocation);[m
 		break;[m
[36m@@ -73,11 +83,32 @@[m [mNTSTATUS deviceDispatchIoctl(PDEVICE_OBJECT deviceObject, PIRP irp) {[m
 	case IOCTL_DISK_GET_PARTITION_INFO_EX:[m
 		status = ioctlDispatchGetPartitionInfo(deviceObject, irp, stackLocation);[m
 		break;[m
[31m-		[m
[32m+[m	[32mcase IOCTL_DISK_GET_DRIVE_LAYOUT:[m[41m[m
[32m+[m		[32mstatus = ioctlDispatchGetDriveLayout(deviceObject, irp, stackLocation);[m[41m[m
[32m+[m		[32mbreak;[m[41m[m
[32m+[m	[32mcase IOCTL_DISK_GET_DRIVE_LAYOUT_EX:[m[41m[m
[32m+[m		[32mstatus = ioctlDispatchGetDriveLayoutEx(deviceObject, irp, stackLocation);[m[41m[m
[32m+[m		[32mbreak;[m[41m[m
[32m+[m	[32mcase IOCTL_STORAGE_GET_DEVICE_NUMBER:[m[41m[m
[32m+[m		[32mstatus = ioctlDispatchGetDeviceNumber(deviceObject, irp, stackLocation);[m[41m[m
[32m+[m		[32mbreak;[m[41m[m
[32m+[m	[32mcase IOCTL_STORAGE_GET_MEDIA_TYPES_EX:[m[41m[m
[32m+[m		[32mstatus = ioctlDispatchGetMediaTypes(deviceObject, irp, stackLocation);[m[41m[m
[32m+[m		[32mbreak;[m[41m[m
[32m+[m	[32mcase IOCTL_DISK_IS_DISK_CLUSTERED:[m[41m[m
[32m+[m		[32mstatus = ioctlDispatchIsDiskClustered(deviceObject, irp, stackLocation);[m[41m[m
[32m+[m		[32mbreak;[m[41m[m
[32m+[m	[32mcase IOCTL_DISK_GET_DISK_ATTRIBUTES:[m[41m[m
[32m+[m		[32mstatus = ioctlDispatchGetDiskAttributes(deviceObject, irp, stackLocation);[m[41m[m
[32m+[m		[32mbreak;[m[41m[m
[32m+[m	[32mcase IOCTL_DISK_GET_DISK_FLAGS:[m[41m[m
[32m+[m		[32mstatus = ioctlDispatchGetDiskFlags(deviceObject, irp, stackLocation);[m[41m[m
[32m+[m		[32mbreak;[m[41m[m
 	// All of the following codes are known and intentionally not handled[m
 	case IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME:[m
 	case IOCTL_MOUNTDEV_QUERY_STABLE_GUID:[m
 	case IOCTL_VOLUME_GET_GPT_ATTRIBUTES:[m
[32m+[m	[32mcase IOCTL_SCSI_GET_ADDRESS:[m[41m[m
 	case 0x4d0010: // Couldn't find macro that defines this IOCTL but from its usage in "mountmgr.sys" it doesn't look important[m
 	case 0x45561088: // Couldn't find macro that defines this IOCTL nor documentation or reference online. Issued from fveapi.dll[m
 		TRACE("Intentionally unahndeled control code");[m
[1mdiff --git a/VirtualStorageDevice/VirtualStorageDevice/VirtualStorageIoctlFunctions.cpp b/VirtualStorageDevice/VirtualStorageDevice/VirtualStorageIoctlFunctions.cpp[m
[1mindex 65c345c..d411b84 100644[m
[1m--- a/VirtualStorageDevice/VirtualStorageDevice/VirtualStorageIoctlFunctions.cpp[m
[1m+++ b/VirtualStorageDevice/VirtualStorageDevice/VirtualStorageIoctlFunctions.cpp[m
[36m@@ -30,27 +30,16 @@[m [mNTSTATUS ioctlDispatchGetDriveGeometry(PDEVICE_OBJECT deviceObject, PIRP irp, PI[m
 	NTSTATUS status = STATUS_SUCCESS;[m
 	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);[m
 	const auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;[m
[31m-	DISK_GEOMETRY* outputInfo = nullptr;[m
[32m+[m	[32mPDISK_GEOMETRY outputInfo = nullptr;[m[41m[m
 	ULONG_PTR dataWritten = 0;[m
 	LARGE_INTEGER deviceTracks;[m
 	LARGE_INTEGER deviceCylinders;	[m
 	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);[m
 	TRACE("ioctlDispatchGetDriveGeometry called");[m
 	CHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(DISK_GEOMETRY)), "Buffer verification failed");[m
[31m-	outputInfo = reinterpret_cast<DISK_GEOMETRY*>(outputBuffer);[m
[31m-	deviceTracks.QuadPart = (virtualStorageExtension->file.size.QuadPart / BYTES_PER_SECTOR) / SECTORS_PER_TRACK;[m
[31m-	deviceCylinders.QuadPart = deviceTracks.QuadPart / MAX_TRACK_PER_CYLINDER;[m
[31m-	if (!deviceCylinders.QuadPart) {[m
[31m-		deviceCylinders.QuadPart = 1;[m
[31m-	}[m
[31m-	if (!deviceTracks.QuadPart || deviceTracks.QuadPart > MAX_TRACK_PER_CYLINDER) {[m
[31m-		deviceTracks.QuadPart = MAX_TRACK_PER_CYLINDER;[m
[31m-	}[m
[31m-	outputInfo->Cylinders.QuadPart = deviceCylinders.QuadPart;[m
[32m+[m	[32moutputInfo = reinterpret_cast<PDISK_GEOMETRY>(outputBuffer);[m[41m[m
 	outputInfo->MediaType = RemovableMedia;[m
[31m-	outputInfo->TracksPerCylinder = deviceTracks.LowPart;[m
[31m-	outputInfo->SectorsPerTrack = SECTORS_PER_TRACK;[m
[31m-	outputInfo->BytesPerSector = BYTES_PER_SECTOR;[m
[32m+[m	[32mFILL_DISK_PHYSICAL_INFO(*outputInfo, virtualStorageExtension->file.size);[m[41m[m
 	dataWritten = sizeof(DISK_GEOMETRY);[m
 cleanup:[m
 	irp->IoStatus.Status = status;[m
[36m@@ -58,6 +47,40 @@[m [mcleanup:[m
 	return STATUS_SUCCESS;[m
 }[m
 [m
[32m+[m[32mNTSTATUS ioctlDispatchGetDriveGeometryEx(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {[m[41m[m
[32m+[m	[32mUNREFERENCED_PARAMETER(deviceObject);[m[41m[m
[32m+[m	[32mNTSTATUS status = STATUS_SUCCESS;[m[41m[m
[32m+[m	[32mconst auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);[m[41m[m
[32m+[m	[32mconst auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;[m[41m[m
[32m+[m	[32mPDISK_GEOMETRY_EX outputInfo = nullptr;[m[41m[m
[32m+[m	[32mULONG_PTR dataWritten = 0;[m[41m[m
[32m+[m	[32mLARGE_INTEGER deviceTracks;[m[41m[m
[32m+[m	[32mLARGE_INTEGER deviceCylinders;[m[41m[m
[32m+[m	[32mPDISK_PARTITION_INFO diskPartitionInfo = nullptr;[m[41m[m
[32m+[m	[32mPDISK_DETECTION_INFO diskDetectionInfo = nullptr;[m[41m[m
[32m+[m	[32mconst auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);[m[41m[m
[32m+[m	[32mTRACE("ioctlDispatchGetDriveGeometryEx called");[m[41m[m
[32m+[m	[32mCHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(DISK_GEOMETRY) + sizeof(LARGE_INTEGER)), "Buffer verification failed");[m[41m[m
[32m+[m	[32moutputInfo = reinterpret_cast<PDISK_GEOMETRY_EX>(outputBuffer);[m[41m[m
[32m+[m	[32mFILL_DISK_PHYSICAL_INFO(outputInfo->Geometry, virtualStorageExtension->file.size);[m[41m[m
[32m+[m	[32moutputInfo->DiskSize = virtualStorageExtension->file.size;[m[41m[m
[32m+[m	[32mdataWritten = sizeof(DISK_GEOMETRY) + sizeof(LARGE_INTEGER);[m[41m[m
[32m+[m	[32mCHECK_AND_SET_STATUS(length >= (sizeof(DISK_GEOMETRY) + sizeof(LARGE_INTEGER) + sizeof(DISK_PARTITION_INFO)), STATUS_BUFFER_TOO_SMALL, "Not adding disk partition info");[m[41m[m
[32m+[m	[32mdiskPartitionInfo = reinterpret_cast<PDISK_PARTITION_INFO>(outputInfo->Data);[m[41m[m
[32m+[m	[32mdiskPartitionInfo->SizeOfPartitionInfo = sizeof(DISK_PARTITION_INFO);[m[41m[m
[32m+[m	[32mdiskPartitionInfo->PartitionStyle = PARTITION_STYLE_RAW;[m[41m[m
[32m+[m	[32mdataWritten += sizeof(DISK_PARTITION_INFO);[m[41m[m
[32m+[m	[32mCHECK_AND_SET_STATUS(length >= (sizeof(DISK_GEOMETRY) + sizeof(LARGE_INTEGER) + sizeof(DISK_PARTITION_INFO) + sizeof(DISK_DETECTION_INFO)), STATUS_BUFFER_TOO_SMALL, "Not adding disk detection info");[m[41m[m
[32m+[m	[32mdiskDetectionInfo = reinterpret_cast<PDISK_DETECTION_INFO>(reinterpret_cast<char*>(outputInfo->Data) + sizeof(DISK_PARTITION_INFO));[m[41m[m
[32m+[m	[32mdiskDetectionInfo->SizeOfDetectInfo = 0;[m[41m[m
[32m+[m	[32mdiskDetectionInfo->DetectionType = DetectNone;[m[41m[m
[32m+[m	[32mdataWritten += sizeof(DISK_DETECTION_INFO);[m[41m[m
[32m+[m[32mcleanup:[m[41m[m
[32m+[m	[32mirp->IoStatus.Status = status;[m[41m[m
[32m+[m	[32mirp->IoStatus.Information = dataWritten;[m[41m[m
[32m+[m	[32mreturn STATUS_SUCCESS;[m[41m[m
[32m+[m[32m}[m[41m[m
[32m+[m[41m[m
 NTSTATUS ioctlDispatchNoHandleReturnSuccess(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {[m
 	UNREFERENCED_PARAMETER(deviceObject);[m
 	UNREFERENCED_PARAMETER(stackLocation);[m
[36m@@ -221,7 +244,6 @@[m [mcleanup:[m
 }[m
 [m
 NTSTATUS ioctlDispatchGetPartitionInfo(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {[m
[31m-	UNREFERENCED_PARAMETER(deviceObject);[m
 	NTSTATUS status = STATUS_SUCCESS;[m
 	TRACE("ioctlDispatchGetPartitionInfo called");[m
 	const auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);[m
[36m@@ -235,6 +257,7 @@[m [mNTSTATUS ioctlDispatchGetPartitionInfo(PDEVICE_OBJECT deviceObject, PIRP irp, PI[m
 	output->PartitionStyle = PARTITION_STYLE_RAW;[m
 	output->StartingOffset.QuadPart = 0;[m
 	output->PartitionLength = virtualStorageExtension->file.size;[m
[32m+[m	[32moutput->PartitionNumber = 0;[m[41m[m
 	output->RewritePartition = false;[m
 #if (NTDDI_VERSION >= NTDDI_WIN10_RS3)  /* ABRACADABRA_WIN10_RS3 */[m
 	output->IsServicePartition = false;[m
[36m@@ -245,4 +268,160 @@[m [mcleanup:[m
 	return status;[m
 }[m
 [m
[32m+[m[32mNTSTATUS ioctlDispatchGetDriveLayout(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {[m[41m[m
[32m+[m	[32mNTSTATUS status = STATUS_SUCCESS;[m[41m[m
[32m+[m	[32mTRACE("ioctlDispatchGetDriveLayout called");[m[41m[m
[32m+[m	[32mconst auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);[m[41m[m
[32m+[m	[32mconst auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;[m[41m[m
[32m+[m	[32mPDRIVE_LAYOUT_INFORMATION output = nullptr;[m[41m[m
[32m+[m	[32mULONG_PTR bytesWritten = 0;[m[41m[m
[32m+[m	[32mconst auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);[m[41m[m
[32m+[m	[32mCHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(DRIVE_LAYOUT_INFORMATION)), "Buffer verification failed");[m[41m[m
[32m+[m	[32moutput = reinterpret_cast<PDRIVE_LAYOUT_INFORMATION>(outputBuffer);[m[41m[m
[32m+[m	[32mbytesWritten = sizeof(DRIVE_LAYOUT_INFORMATION);[m[41m[m
[32m+[m	[32moutput->PartitionCount = 1;[m[41m[m
[32m+[m	[32moutput->Signature = 0;[m[41m[m
[32m+[m	[32moutput->PartitionEntry[0].StartingOffset.QuadPart = 0;[m[41m[m
[32m+[m	[32moutput->PartitionEntry[0].PartitionLength = virtualStorageExtension->file.size;[m[41m[m
[32m+[m	[32moutput->PartitionEntry[0].HiddenSectors = 0;[m[41m[m
[32m+[m	[32moutput->PartitionEntry[0].PartitionNumber = 0;[m[41m[m
[32m+[m	[32moutput->PartitionEntry[0].BootIndicator = false;[m[41m[m
[32m+[m	[32moutput->PartitionEntry[0].RecognizedPartition = true;[m[41m[m
[32m+[m	[32moutput->PartitionEntry[0].RewritePartition = false;[m[41m[m
[32m+[m[32mcleanup:[m[41m[m
[32m+[m	[32mirp->IoStatus.Status = status;[m[41m[m
[32m+[m	[32mirp->IoStatus.Information = bytesWritten;[m[41m[m
[32m+[m	[32mreturn status;[m[41m[m
[32m+[m[32m}[m[41m[m
[32m+[m[41m[m
[32m+[m[32mNTSTATUS ioctlDispatchGetDriveLayoutEx(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {[m[41m[m
[32m+[m	[32mNTSTATUS status = STATUS_SUCCESS;[m[41m[m
[32m+[m	[32mTRACE("ioctlDispatchGetDriveLayoutEx called");[m[41m[m
[32m+[m	[32mconst auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);[m[41m[m
[32m+[m	[32mconst auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;[m[41m[m
[32m+[m	[32mPDRIVE_LAYOUT_INFORMATION_EX output = nullptr;[m[41m[m
[32m+[m	[32mULONG_PTR bytesWritten = 0;[m[41m[m
[32m+[m	[32mconst auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);[m[41m[m
[32m+[m	[32mCHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(DRIVE_LAYOUT_INFORMATION_EX)), "Buffer verification failed");[m[41m[m
[32m+[m	[32moutput = reinterpret_cast<PDRIVE_LAYOUT_INFORMATION_EX>(outputBuffer);[m[41m[m
[32m+[m	[32mbytesWritten = sizeof(DRIVE_LAYOUT_INFORMATION_EX);[m[41m[m
[32m+[m	[32moutput->PartitionStyle = PARTITION_STYLE_RAW;[m[41m[m
[32m+[m	[32moutput->PartitionCount = 1;[m[41m[m
[32m+[m	[32moutput->PartitionEntry[0].PartitionStyle = PARTITION_STYLE_RAW;[m[41m[m
[32m+[m	[32moutput->PartitionEntry[0].StartingOffset.QuadPart = 0;[m[41m[m
[32m+[m	[32moutput->PartitionEntry[0].PartitionLength = virtualStorageExtension->file.size;[m[41m[m
[32m+[m	[32moutput->PartitionEntry[0].PartitionNumber = 0;[m[41m[m
[32m+[m	[32moutput->PartitionEntry[0].RewritePartition = false;[m[41m[m
[32m+[m[32m#if (NTDDI_VERSION >= NTDDI_WIN10_RS3)  /* ABRACADABRA_WIN10_RS3 */[m[41m[m
[32m+[m	[32moutput->IsServicePartition = false;[m[41m[m
[32m+[m[32m#endif[m[41m[m
[32m+[m[32mcleanup:[m[41m[m
[32m+[m	[32mirp->IoStatus.Status = status;[m[41m[m
[32m+[m	[32mirp->IoStatus.Information = bytesWritten;[m[41m[m
[32m+[m	[32mreturn status;[m[41m[m
[32m+[m[32m}[m[41m[m
[32m+[m[41m[m
[32m+[m[32mNTSTATUS ioctlDispatchGetDeviceNumber(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {[m[41m[m
[32m+[m	[32m// __debugbreak();[m[41m[m
[32m+[m	[32mNTSTATUS status = STATUS_SUCCESS;[m[41m[m
[32m+[m	[32mTRACE("ioctlDispatchGetDeviceNumber called");[m[41m[m
[32m+[m	[32mconst auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);[m[41m[m
[32m+[m	[32mconst auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;[m[41m[m
[32m+[m	[32mPSTORAGE_DEVICE_NUMBER output = nullptr;[m[41m[m
[32m+[m	[32mULONG_PTR bytesWritten = 0;[m[41m[m
[32m+[m	[32mconst auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);[m[41m[m
[32m+[m	[32mCHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(STORAGE_DEVICE_NUMBER)), "Buffer verification failed");[m[41m[m
[32m+[m	[32moutput = reinterpret_cast<PSTORAGE_DEVICE_NUMBER>(outputBuffer);[m[41m[m
[32m+[m	[32mbytesWritten = sizeof(STORAGE_DEVICE_NUMBER);[m[41m[m
[32m+[m	[32moutput->DeviceType = deviceObject->DeviceType;[m[41m[m
[32m+[m	[32moutput->DeviceNumber = DEVICE_EXTENSION_HEADER_MAGIC + virtualStorageExtension->id;[m[41m[m
[32m+[m	[32moutput->PartitionNumber = 0;[m[41m[m
[32m+[m[32mcleanup:[m[41m[m
[32m+[m	[32mirp->IoStatus.Status = status;[m[41m[m
[32m+[m	[32mirp->IoStatus.Information = bytesWritten;[m[41m[m
[32m+[m	[32mreturn status;[m[41m[m
[32m+[m[32m}[m[41m[m
[32m+[m[41m[m
[32m+[m[32mNTSTATUS ioctlDispatchGetMediaTypes(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {[m[41m[m
[32m+[m	[32m// __debugbreak();[m[41m[m
[32m+[m	[32mNTSTATUS status = STATUS_SUCCESS;[m[41m[m
[32m+[m	[32mTRACE("ioctlDispatchGetMediaTypes called");[m[41m[m
[32m+[m	[32mconst auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);[m[41m[m
[32m+[m	[32mconst auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;[m[41m[m
[32m+[m	[32mPGET_MEDIA_TYPES output = nullptr;[m[41m[m
[32m+[m	[32mULONG_PTR bytesWritten = 0;[m[41m[m
[32m+[m	[32mLARGE_INTEGER deviceTracks;[m[41m[m
[32m+[m	[32mLARGE_INTEGER deviceCylinders;[m[41m[m
[32m+[m	[32mconst auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);[m[41m[m
[32m+[m	[32mCHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(GET_MEDIA_TYPES)), "Buffer verification failed");[m[41m[m
[32m+[m	[32moutput = reinterpret_cast<PGET_MEDIA_TYPES>(outputBuffer);[m[41m[m
[32m+[m	[32mbytesWritten = sizeof(GET_MEDIA_TYPES);[m[41m[m
[32m+[m	[32moutput->DeviceType = deviceObject->DeviceType;[m[41m[m
[32m+[m	[32moutput->MediaInfoCount = 1;[m[41m[m
[32m+[m	[32moutput->MediaInfo->DeviceSpecific.RemovableDiskInfo.MediaType = static_cast<STORAGE_MEDIA_TYPE>(FixedMedia); // Looks like I need to lie to vds.exe[m[41m[m
[32m+[m	[32moutput->MediaInfo->DeviceSpecific.RemovableDiskInfo.NumberMediaSides = 1;[m[41m[m
[32m+[m	[32moutput->MediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics = MEDIA_READ_WRITE | MEDIA_CURRENTLY_MOUNTED;[m[41m[m
[32m+[m	[32mFILL_DISK_PHYSICAL_INFO(output->MediaInfo->DeviceSpecific.RemovableDiskInfo, virtualStorageExtension->file.size);[m[41m[m
[32m+[m[32mcleanup:[m[41m[m
[32m+[m	[32mirp->IoStatus.Status = status;[m[41m[m
[32m+[m	[32mirp->IoStatus.Information = bytesWritten;[m[41m[m
[32m+[m	[32mreturn status;[m[41m[m
[32m+[m[32m}[m[41m[m
[32m+[m[41m[m
[32m+[m[32mNTSTATUS ioctlDispatchIsDiskClustered(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {[m[41m[m
[32m+[m	[32mUNREFERENCED_PARAMETER(deviceObject);[m[41m[m
[32m+[m	[32mNTSTATUS status = STATUS_SUCCESS;[m[41m[m
[32m+[m	[32mTRACE("ioctlDispatchIsDiskClustered called");[m[41m[m
[32m+[m	[32mULONG bytesWritten = 0;[m[41m[m
[32m+[m	[32mconst auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);[m[41m[m
[32m+[m	[32mconst auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;[m[41m[m
[32m+[m	[32mCHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(BOOLEAN)), "Buffer verification failed");[m[41m[m
[32m+[m	[32m*reinterpret_cast<BOOLEAN*>(outputBuffer) = FALSE;[m[41m[m
[32m+[m	[32mbytesWritten = sizeof(BOOLEAN);[m[41m[m
[32m+[m[32mcleanup:[m[41m[m
[32m+[m	[32mirp->IoStatus.Status = STATUS_SUCCESS;[m[41m[m
[32m+[m	[32mirp->IoStatus.Information = bytesWritten;[m[41m[m
[32m+[m	[32mreturn status;[m[41m[m
[32m+[m[32m}[m[41m[m
[32m+[m[41m[m
[32m+[m[32mNTSTATUS ioctlDispatchGetDiskAttributes(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {[m[41m[m
[32m+[m	[32mUNREFERENCED_PARAMETER(deviceObject);[m[41m[m
[32m+[m	[32mNTSTATUS status = STATUS_SUCCESS;[m[41m[m
[32m+[m	[32mTRACE("ioctlDispatchGetDiskAttributes called");[m[41m[m
[32m+[m	[32mconst auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);[m[41m[m
[32m+[m	[32mconst auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;[m[41m[m
[32m+[m	[32mPGET_DISK_ATTRIBUTES  output = nullptr;[m[41m[m
[32m+[m	[32mULONG_PTR bytesWritten = 0;[m[41m[m
[32m+[m	[32mCHECK_STATUS(verifyBufferSize(outputBuffer, length, sizeof(GET_DISK_ATTRIBUTES)), "Buffer verification failed");[m[41m[m
[32m+[m	[32moutput = reinterpret_cast<PGET_DISK_ATTRIBUTES>(outputBuffer);[m[41m[m
[32m+[m	[32mbytesWritten = sizeof(GET_DISK_ATTRIBUTES);[m[41m[m
[32m+[m	[32moutput->Version = sizeof(GET_DISK_ATTRIBUTES);[m[41m[m
[32m+[m	[32moutput->Reserved1 = 0;[m[41m[m
[32m+[m	[32moutput->Attributes = 0;[m[41m[m
[32m+[m[32mcleanup:[m[41m[m
[32m+[m	[32mirp->IoStatus.Status = status;[m[41m[m
[32m+[m	[32mirp->IoStatus.Information = bytesWritten;[m[41m[m
[32m+[m	[32mreturn status;[m[41m[m
[32m+[m[32m}[m[41m[m
[32m+[m[41m[m
[32m+[m[32m// Obtained from IDA[m[41m[m
[32m+[m[32m#define GET_DISK_FLAGS_SIZE 0x14[m[41m[m
[32m+[m[41m[m
[32m+[m[32mNTSTATUS ioctlDispatchGetDiskFlags(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {[m[41m[m
[32m+[m	[32mUNREFERENCED_PARAMETER(deviceObject);[m[41m[m
[32m+[m	[32mNTSTATUS status = STATUS_SUCCESS;[m[41m[m
[32m+[m	[32mTRACE("ioctlDispatchGetDiskFlags called");[m[41m[m
[32m+[m	[32mconst auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);[m[41m[m
[32m+[m	[32mconst auto length = stackLocation->Parameters.DeviceIoControl.OutputBufferLength;[m[41m[m
[32m+[m	[32mULONG_PTR bytesWritten = 0;[m[41m[m
[32m+[m	[32mCHECK_STATUS(verifyBufferSize(outputBuffer, length, GET_DISK_FLAGS_SIZE), "Buffer verification failed");[m[41m[m
[32m+[m	[32mbytesWritten = GET_DISK_FLAGS_SIZE;[m[41m[m
[32m+[m	[32m// Looks like the only usage is to check if some flags are true, none of them relevant to us[m[41m[m
[32m+[m	[32mRtlZeroMemory(outputBuffer, GET_DISK_FLAGS_SIZE);[m[41m[m
[32m+[m[32mcleanup:[m[41m[m
[32m+[m	[32mirp->IoStatus.Status = status;[m[41m[m
[32m+[m	[32mirp->IoStatus.Information = bytesWritten;[m[41m[m
[32m+[m	[32mreturn status;[m[41m[m
[32m+[m[32m}[m[41m[m
[32m+[m[41m[m
 }[m
[1mdiff --git a/VirtualStorageDevice/VirtualStorageDevice/VirtualStorageIoctlFunctions.h b/VirtualStorageDevice/VirtualStorageDevice/VirtualStorageIoctlFunctions.h[m
[1mindex 047f249..f76d79d 100644[m
[1m--- a/VirtualStorageDevice/VirtualStorageDevice/VirtualStorageIoctlFunctions.h[m
[1m+++ b/VirtualStorageDevice/VirtualStorageDevice/VirtualStorageIoctlFunctions.h[m
[36m@@ -7,6 +7,8 @@[m [mNTSTATUS ioctlDispatchGetLength(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK[m
 [m
 NTSTATUS ioctlDispatchGetDriveGeometry(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);[m
 [m
[32m+[m[32mNTSTATUS ioctlDispatchGetDriveGeometryEx(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);[m[41m[m
[32m+[m[41m[m
 NTSTATUS ioctlDispatchNoHandleReturnSuccess(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);[m
 [m
 NTSTATUS ioctlDispatchGetHotplugInfo(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);[m
[36m@@ -25,4 +27,18 @@[m [mNTSTATUS ioctlDispatchDiskExtents(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STA[m
 [m
 NTSTATUS ioctlDispatchGetPartitionInfo(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);[m
 [m
[32m+[m[32mNTSTATUS ioctlDispatchGetDriveLayout(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);[m[41m[m
[32m+[m[41m[m
[32m+[m[32mNTSTATUS ioctlDispatchGetDriveLayoutEx(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);[m[41m[m
[32m+[m[41m[m
[32m+[m[32mNTSTATUS ioctlDispatchGetDeviceNumber(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);[m[41m[m
[32m+[m[41m[m
[32m+[m[32mNTSTATUS ioctlDispatchGetMediaTypes(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);[m[41m[m
[32m+[m[41m[m
[32m+[m[32mNTSTATUS ioctlDispatchIsDiskClustered(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);[m[41m[m
[32m+[m[41m[m
[32m+[m[32mNTSTATUS ioctlDispatchGetDiskAttributes(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);[m[41m[m
[32m+[m[41m[m
[32m+[m[32mNTSTATUS ioctlDispatchGetDiskFlags(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);[m[41m[m
[32m+[m[41m[m
 }[m
