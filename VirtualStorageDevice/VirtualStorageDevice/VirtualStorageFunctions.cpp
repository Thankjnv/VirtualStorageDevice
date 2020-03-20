#include "VirtualStorageFunctions.h"
#include <ntdddisk.h>
#include "VirtualStorageIoctlFunctions.h"

// Found this definition in some online implementation. ioctl code = 2d5190 from function FsRtlGetVirtualDiskNestingLevel
#define IOCTL_STORAGE_GET_VIRTUALDISK_NESTINGLEVEL CTL_CODE(IOCTL_STORAGE_BASE, 0x0464, METHOD_BUFFERED, FILE_READ_ACCESS)

namespace virtualStorage {

NTSTATUS deviceDispatchIoctl(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);
	auto ioctlCode = stackLocation->Parameters.DeviceIoControl.IoControlCode;
	TRACE("Ioctl dispatch called, ioctl code=%lx", ioctlCode);
	// Each ioctl dispatcher should set irp->IoStatus
	switch (ioctlCode) {
	case IOCTL_DISK_GET_LENGTH_INFO:
		status = ioctlDispatchGetLength(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_GET_DRIVE_GEOMETRY:
		status = ioctlDispatchGetDriveGeometry(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_MEDIA_REMOVAL:
		status = ioctlDispatchMediaRemoval(deviceObject, irp, stackLocation);
		break;
	case IOCTL_DISK_IS_WRITABLE:
		status = ioctlDispatchDiskIsWritable(deviceObject, irp, stackLocation);
		break;
	case IOCTL_STORAGE_QUERY_PROPERTY:
		status = ioctlDispatchQueryProperty(deviceObject, irp, stackLocation);
		break;
	case IOCTL_STORAGE_GET_HOTPLUG_INFO:
		status = ioctlDispatchGetHotplugInfo(deviceObject, irp, stackLocation);
		break;
	case IOCTL_STORAGE_GET_VIRTUALDISK_NESTINGLEVEL:
		status = ioctlDispatchGetNestingLevel(deviceObject, irp, stackLocation);
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

NTSTATUS deviceDispatchRead(PDEVICE_OBJECT deviceObject, PIRP irp) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);
	auto outputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	if (irp->Flags & IRP_PAGING_IO) {
		outputBuffer = reinterpret_cast<char*>(MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority));
	}
	const auto length = stackLocation->Parameters.Read.Length;
	const auto offset = stackLocation->Parameters.Read.ByteOffset;
	ULONG_PTR bytesRead = 0;
	TRACE("Read size: %lx, offset: %llx", length, offset.QuadPart);
	CHECK_AND_SET_STATUS(outputBuffer, STATUS_INVALID_PARAMETER, "No output buffer provided");
	CHECK_AND_SET_STATUS(!(length % BYTES_PER_SECTOR), STATUS_INVALID_OFFSET_ALIGNMENT, "Read size not aligned");
	CHECK_AND_SET_STATUS(!(offset.QuadPart % BYTES_PER_SECTOR), STATUS_INVALID_OFFSET_ALIGNMENT, "Read offset not aligned");
	// TODO: implement logic
	RtlZeroMemory(outputBuffer, length);
	bytesRead = static_cast<ULONG_PTR>(length);
	if (stackLocation->FileObject) {
		stackLocation->FileObject->CurrentByteOffset.QuadPart = offset.QuadPart + length;
	}
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesRead;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS deviceDispatchWrite(PDEVICE_OBJECT deviceObject, PIRP irp) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);
	auto inputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	if (irp->Flags & IRP_PAGING_IO) {
		inputBuffer = reinterpret_cast<char*>(MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority));
	}
	const auto length = stackLocation->Parameters.Write.Length;
	const auto offset = stackLocation->Parameters.Write.ByteOffset;
	ULONG_PTR bytesWritten = 0;
	TRACE("Write size: %lx, offset: %llx", length, offset.QuadPart);
	CHECK_AND_SET_STATUS(inputBuffer, STATUS_INVALID_PARAMETER, "No input buffer provided");
	CHECK_AND_SET_STATUS(!(length % BYTES_PER_SECTOR), STATUS_INVALID_OFFSET_ALIGNMENT, "Write size not aligned");
	CHECK_AND_SET_STATUS(!(offset.QuadPart % BYTES_PER_SECTOR), STATUS_INVALID_OFFSET_ALIGNMENT, "Write offset not aligned");
	UNREFERENCED_PARAMETER(inputBuffer);
	// TODO: implement logic
	bytesWritten = static_cast<ULONG_PTR>(length);
	if (stackLocation->FileObject) {
		stackLocation->FileObject->CurrentByteOffset.QuadPart = offset.QuadPart + length;
	}
cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesWritten;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}

}
