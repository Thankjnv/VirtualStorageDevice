#include "VirtualStorageFunctions.h"
#include <ntdddisk.h>
#include <mountdev.h>
#include <ntddvol.h>
#include <ntddscsi.h>
#include "VirtualStorageIoctlFunctions.h"
#include "VirtualStoragePnpFunctions.h"
#include "UtilFunctions.h"

// Found this definition in some online implementation. ioctl code = 2d5190 from function FsRtlGetVirtualDiskNestingLevel
#define IOCTL_STORAGE_GET_VIRTUALDISK_NESTING_LEVEL CTL_CODE(IOCTL_STORAGE_BASE, 0x0464, METHOD_BUFFERED, FILE_READ_ACCESS)

// The following ioctl is not defined, found it from windbg + ida. ioctl code = 700f8
#define IOCTL_DISK_IS_DISK_CLUSTERED CTL_CODE(IOCTL_DISK_BASE, 0x003e, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Same as above, ioctl code = 704010
#define IOCTL_DISK_GET_DISK_FLAGS CTL_CODE(112, 0x0004, METHOD_BUFFERED, FILE_READ_ACCESS)

namespace virtualStorage {

NTSTATUS deviceDispatchIoctl(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);
	const auto ioctlCode = stackLocation->Parameters.DeviceIoControl.IoControlCode;
	TRACE("Ioctl dispatch called, ioctl code=%lx", ioctlCode);
	// Each ioctl dispatcher should set irp->IoStatus
	switch (ioctlCode) {
	case IOCTL_DISK_GET_LENGTH_INFO:
		status = ioctlDispatchGetLength(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_GET_DRIVE_GEOMETRY:
		status = ioctlDispatchGetDriveGeometry(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_GET_DRIVE_GEOMETRY_EX:
		status = ioctlDispatchGetDriveGeometryEx(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_MEDIA_REMOVAL:
		status = ioctlDispatchNoHandleReturnSuccess(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_IS_WRITABLE:
		status = ioctlDispatchNoHandleReturnSuccess(deviceObject, irp, stackLocation);
		break;
	case IOCTL_STORAGE_QUERY_PROPERTY:
		status = ioctlDispatchNoHandleReturnSuccess(deviceObject, irp, stackLocation);
		break;
	case IOCTL_STORAGE_GET_HOTPLUG_INFO:
		status = ioctlDispatchGetHotplugInfo(deviceObject, irp, stackLocation);
		break;
	case IOCTL_STORAGE_GET_VIRTUALDISK_NESTING_LEVEL:
		status = ioctlDispatchGetNestingLevel(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_CHECK_VERIFY:
		// Intentional fall-through
	case IOCTL_STORAGE_CHECK_VERIFY:
		status = ioctlDispatchCheckVerify(deviceObject, irp, stackLocation);
		break;
	case IOCTL_MOUNTDEV_QUERY_DEVICE_NAME:
		status = ioctlDispatchQueryDeviceName(deviceObject, irp, stackLocation);
		break;
	case IOCTL_MOUNTDEV_QUERY_UNIQUE_ID:
		status = ioctlDispatchQueryDeviceUniqueId(deviceObject, irp, stackLocation);
		break;
	case IOCTL_MOUNTDEV_LINK_CREATED:
		TRACE("Link to device created");
		status = ioctlDispatchNoHandleReturnSuccess(deviceObject, irp, stackLocation);
		break;
	case IOCTL_MOUNTDEV_LINK_DELETED:
		TRACE("Link to device deleted");
		status = ioctlDispatchNoHandleReturnSuccess(deviceObject, irp, stackLocation);
		break;
	case IOCTL_VOLUME_ONLINE:
		status = ioctlDispatchNoHandleReturnSuccess(deviceObject, irp, stackLocation);
		break;
	case IOCTL_VOLUME_OFFLINE:
		status = ioctlDispatchNoHandleReturnSuccess(deviceObject, irp, stackLocation);
		break;
	case IOCTL_VOLUME_IS_DYNAMIC:
		status = ioctlDispatchVolumeIsDynamic(deviceObject, irp, stackLocation);
		break;
	case IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS:
		status = ioctlDispatchDiskExtents(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_GET_PARTITION_INFO_EX:
		status = ioctlDispatchGetPartitionInfo(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_GET_DRIVE_LAYOUT:
		status = ioctlDispatchGetDriveLayout(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_GET_DRIVE_LAYOUT_EX:
		status = ioctlDispatchGetDriveLayoutEx(deviceObject, irp, stackLocation);
		break;
	case IOCTL_STORAGE_GET_DEVICE_NUMBER:
		status = ioctlDispatchGetDeviceNumber(deviceObject, irp, stackLocation);
		break;
	case IOCTL_STORAGE_GET_MEDIA_TYPES_EX:
		status = ioctlDispatchGetMediaTypes(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_IS_DISK_CLUSTERED:
		status = ioctlDispatchIsDiskClustered(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_GET_DISK_ATTRIBUTES:
		status = ioctlDispatchGetDiskAttributes(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_GET_DISK_FLAGS:
		status = ioctlDispatchGetDiskFlags(deviceObject, irp, stackLocation);
		break;
	// All of the following codes are known and intentionally not handled
	case IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME:
	case IOCTL_MOUNTDEV_QUERY_STABLE_GUID:
	case IOCTL_VOLUME_GET_GPT_ATTRIBUTES:
	case IOCTL_SCSI_GET_ADDRESS:
	case 0x4d0010: // Couldn't find macro that defines this IOCTL but from its usage in "mountmgr.sys" it doesn't look important
	case 0x45561088: // Couldn't find macro that defines this IOCTL nor documentation or reference online. Issued from fveapi.dll
		TRACE("Intentionally unahndeled control code");
		status = STATUS_NOT_IMPLEMENTED;
		irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
		irp->IoStatus.Information = 0;
		break;
	default:
		TRACE("Unknown Ioctl code");
		__debugbreak();
		status = STATUS_NOT_IMPLEMENTED;
		irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
		irp->IoStatus.Information = 0;
		break;
	}
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS doReadWrite(PDEVICE_OBJECT deviceObject, PIRP irp, bool isWrite) {
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);
	auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	if (irp->Flags & IRP_PAGING_IO) {
		outputBuffer = reinterpret_cast<char*>(MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority));
	}
	auto length = stackLocation->Parameters.Read.Length;
	auto offset = stackLocation->Parameters.Read.ByteOffset;
	if (isWrite) {
		length = stackLocation->Parameters.Write.Length;
		offset = stackLocation->Parameters.Write.ByteOffset;
	}
	const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	TRACE("size: %lx, offset: %llx", length, offset.QuadPart);
	CHECK_AND_SET_STATUS(outputBuffer, STATUS_INVALID_PARAMETER, "No buffer provided");
	CHECK_AND_SET_STATUS(!(length % BYTES_PER_SECTOR), STATUS_INVALID_OFFSET_ALIGNMENT, "Size not aligned");
	CHECK_AND_SET_STATUS(!(offset.QuadPart % BYTES_PER_SECTOR), STATUS_INVALID_OFFSET_ALIGNMENT, "Offset not aligned");
	CHECK_AND_SET_STATUS(static_cast<ULONG>(offset.QuadPart + length) >= offset.QuadPart, STATUS_INTEGER_OVERFLOW, "Offset + length overflow");
cleanup:
	if (!NT_SUCCESS(status)) {
		irp->IoStatus.Status = status;
		// If this flow is reached we never call ZwReadFile/ZwWriteFile so the amount of bytes read is 0.
		irp->IoStatus.Information = 0;
	} else {
		// ZwReadFile/ZwWriteFile will set the value of irp->IoStatus for us.
		if (isWrite) {
			status = ZwWriteFile(virtualStorageExtension->file.handle, nullptr, nullptr, nullptr, &(irp->IoStatus), outputBuffer, length, &offset, nullptr);
		} else {
			status = ZwReadFile(virtualStorageExtension->file.handle, nullptr, nullptr, nullptr, &(irp->IoStatus), outputBuffer, length, &offset, nullptr);
		}
		if (NT_SUCCESS(status)) {
			if (stackLocation->FileObject) {
				stackLocation->FileObject->CurrentByteOffset.QuadPart = offset.QuadPart + length;
			}
		} else {
			if (isWrite) {
				TRACE("ZwWriteFile failed. status=%lx", status);
			} else {
				TRACE("ZwReadFile failed. status=%lx", status);
			}
		}
	}
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS deviceDispatchRead(PDEVICE_OBJECT deviceObject, PIRP irp) {
	TRACE("Read dispatch called");
	return doReadWrite(deviceObject, irp, false);
}

NTSTATUS deviceDispatchWrite(PDEVICE_OBJECT deviceObject, PIRP irp) {
	TRACE("Read dispatch called");
	return doReadWrite(deviceObject, irp, true);
}

NTSTATUS deviceDispatchPnp(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);
	const auto deviceExtensionHeader = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	PDEVICE_OBJECT lowerLevelDevice = nullptr;
	CHECK_STATUS(verifyDeviceExtensionHeader(deviceObject), "verifyDeviceExtensionHeader failed");
	// Each pnp minor dispatcher should set irp->IoStatus
	switch (stackLocation->MinorFunction) {
	case IRP_MN_QUERY_DEVICE_RELATIONS:
		status = pnpDispatchQueryDeviceRelations(deviceObject, irp, stackLocation);
		break;
	case IRP_MN_QUERY_PNP_DEVICE_STATE:
		status = pnpDispatchDeviceState(deviceObject, irp, stackLocation);
		break;
	default:
		TRACE("Unknown Pnp minor");
		IoSkipCurrentIrpStackLocation(irp);
		lowerLevelDevice = deviceExtensionHeader->lowerLevelDevice;
		if (!lowerLevelDevice) {
			// This if is here to handle a rare race condition:
			// 1. The following line executes: deviceExtension->lowerLevelDevice = IoAttachDeviceToDeviceStack(fdoDevice, deviceExtension->pdoDevice);
			// 2. IoAttachDeviceToDeviceStack completes but deviceExtension->lowerLevelDevice isn't assigned just yet
			// 3. Context switch
			// 4. Some pnp minor is called
			// The approach of just passing deviceExtensionHeader->lowerLevelDevice to IofCallDriver will BSOD in this case.
			// Our only option is to pass the request to the pdoDevice (which will be the same as lowerLevelDevice because we created it
			// but the good practice still says we need to call the value returned from IoAttachDeviceToDeviceStack to IofCallDriver).
			lowerLevelDevice = deviceExtensionHeader->pdoDevice;
		}
		status = IofCallDriver(lowerLevelDevice, irp);
		TRACE("Lower level device returned status %lx", status);
		return status;
	}
cleanup:
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}

}
