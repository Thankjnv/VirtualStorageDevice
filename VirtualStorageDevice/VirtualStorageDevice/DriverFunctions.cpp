#include "DriverFunctions.h"
#include "IoctlCodes.h"
#include "IoctlFunctions.h"

NTSTATUS unimplementedMajorFunction(PDEVICE_OBJECT deviceObject, PIRP irp) {
	UNREFERENCED_PARAMETER(deviceObject);
	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);
	TRACE("Unimplemented major function called. Major=%lu, Minor=%lu", stackLocation->MajorFunction, stackLocation->MinorFunction);
	irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS deviceDispatchCreate(PDEVICE_OBJECT deviceObject, PIRP irp) {
	UNREFERENCED_PARAMETER(deviceObject);
	TRACE("Create dispatch called");
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS deviceDispatchClose(PDEVICE_OBJECT deviceObject, PIRP irp) {
	UNREFERENCED_PARAMETER(deviceObject);
	TRACE("Close dispatch called");
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS deviceDispatchCleanup(PDEVICE_OBJECT deviceObject, PIRP irp) {
	UNREFERENCED_PARAMETER(deviceObject);
	TRACE("Cleanup dispatch called");
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS deviceDispatchIoctl(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);
	auto ioctlCode = stackLocation->Parameters.DeviceIoControl.IoControlCode;
	TRACE("Ioctl dispatch called, ioctl code=%lx", ioctlCode);
	// Each ioctl dispatcher should set irp->IoStatus
	switch (ioctlCode) {
	case IOCTL_TEST_CODE:
		status = ioctlDispatchTest(deviceObject, irp, stackLocation);
		break;
	default:
		status = STATUS_NOT_IMPLEMENTED;
		irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
		irp->IoStatus.Information = 0;
		break;
	}
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}
