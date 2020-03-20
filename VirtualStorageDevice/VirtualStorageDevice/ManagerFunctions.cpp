#include "ManagerFunctions.h"
#include "ManagerIoctlFunctions.h"
#include "Common.h"

namespace manager {

NTSTATUS deviceDispatchIoctl(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);
	auto ioctlCode = stackLocation->Parameters.DeviceIoControl.IoControlCode;
	TRACE("Ioctl dispatch called, ioctl code=%lx", ioctlCode);
	// Each ioctl dispatcher should set irp->IoStatus
	switch (ioctlCode) {
	case IOCTL_CREATE_VIRTUAL_STORAGE_CODE:
		status = ioctlDispatchCreateVirtualStorage(deviceObject, irp, stackLocation);
		break;
	default:
		TRACE("Unknown Ioctl code");
		status = STATUS_NOT_IMPLEMENTED;
		irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
		irp->IoStatus.Information = 0;
		break;
	}
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}

}