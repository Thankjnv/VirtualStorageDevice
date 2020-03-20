#include "DriverFunctions.h"
#include "ManagerFunctions.h"
#include "VirtualStorageFunctions.h"
#include "UtilFunctions.h"

NTSTATUS unimplementedMajorFunction(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);
	char* deviceTypeName = reinterpret_cast<char*>("Unknown");
	CHECK_STATUS(getDeviceTypeName(deviceObject, &deviceTypeName), "getDeviceTypeName failed");
cleanup:
	TRACE("%s: Unimplemented major function called. Major=%lu, Minor=%lu", deviceTypeName, stackLocation->MajorFunction, stackLocation->MinorFunction);
	irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS deviceDispatchCreate(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS status = STATUS_SUCCESS;
	char* deviceTypeName = reinterpret_cast<char*>("Unknown");
	CHECK_STATUS(getDeviceTypeName(deviceObject, &deviceTypeName), "getDeviceTypeName failed");
cleanup:
	TRACE("%s: Create dispatch called", deviceTypeName);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS deviceDispatchClose(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS status = STATUS_SUCCESS;
	char* deviceTypeName = reinterpret_cast<char*>("Unknown");
	CHECK_STATUS(getDeviceTypeName(deviceObject, &deviceTypeName), "getDeviceTypeName failed");
cleanup:
	TRACE("%s: Close dispatch called", deviceTypeName);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS deviceDispatchCleanup(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS status = STATUS_SUCCESS;
	char* deviceTypeName = reinterpret_cast<char*>("Unknown");
	CHECK_STATUS(getDeviceTypeName(deviceObject, &deviceTypeName), "getDeviceTypeName failed");
cleanup:
	TRACE("%s: Cleanup dispatch called", deviceTypeName);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS deviceDispatchIoctl(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS status = STATUS_SUCCESS;
	bool requestHandled = false;
	auto deviceExtensionHeader = reinterpret_cast<DeviceExtensionHeader*>(deviceObject->DeviceExtension);
	CHECK_AND_SET_STATUS(deviceExtensionHeader, STATUS_BAD_DATA, "Device has no device extension");
	CHECK_AND_SET_STATUS(deviceExtensionHeader->type < DeviceType::MAX_DEVICE_TYPE, STATUS_BAD_DATA, "Device type out of range");
	switch (deviceExtensionHeader->type) {
	case DeviceType::MANAGER:
		status = manager::deviceDispatchIoctl(deviceObject, irp);
		break;
	case DeviceType::VIRTUAL_STORAGE:
		status = virtualStorage::deviceDispatchIoctl(deviceObject, irp);
		break;
	default:
		TRACE("ERROR: Unreachable code reached");
		break;
	}
	requestHandled = true;
cleanup:
	if (!requestHandled) {
		TRACE("Request wasn't passed to device handler");
		irp->IoStatus.Status = status;
		irp->IoStatus.Information = 0;
		IoCompleteRequest(irp, IO_NO_INCREMENT);
	}
	return status;
}

NTSTATUS deviceDispatchRead(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS status = STATUS_SUCCESS;
	bool requestHandled = false;
	auto deviceExtensionHeader = reinterpret_cast<DeviceExtensionHeader*>(deviceObject->DeviceExtension);
	TRACE("Read dispatch called");
	CHECK_AND_SET_STATUS(deviceExtensionHeader, STATUS_BAD_DATA, "Device has no device extension");
	CHECK_AND_SET_STATUS(deviceExtensionHeader->type < DeviceType::MAX_DEVICE_TYPE, STATUS_BAD_DATA, "Device type out of range");
	switch (deviceExtensionHeader->type) {
	case DeviceType::MANAGER:
		status = unimplementedMajorFunction(deviceObject, irp);
		break;
	case DeviceType::VIRTUAL_STORAGE:
		status = virtualStorage::deviceDispatchRead(deviceObject, irp);
		break;
	default:
		TRACE("ERROR: Unreachable code reached");
		break;
	}
	requestHandled = true;
cleanup:
	if (!requestHandled) {
		TRACE("Request wasn't passed to device handler");
		irp->IoStatus.Status = status;
		irp->IoStatus.Information = 0;
		IoCompleteRequest(irp, IO_NO_INCREMENT);
	}
	return status;
}

NTSTATUS deviceDispatchWrite(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS status = STATUS_SUCCESS;
	bool requestHandled = false;
	auto deviceExtensionHeader = reinterpret_cast<DeviceExtensionHeader*>(deviceObject->DeviceExtension);
	TRACE("Write dispatch called");
	CHECK_AND_SET_STATUS(deviceExtensionHeader, STATUS_BAD_DATA, "Device has no device extension");
	CHECK_AND_SET_STATUS(deviceExtensionHeader->type < DeviceType::MAX_DEVICE_TYPE, STATUS_BAD_DATA, "Device type out of range");
	switch (deviceExtensionHeader->type) {
	case DeviceType::MANAGER:
		status = unimplementedMajorFunction(deviceObject, irp);
		break;
	case DeviceType::VIRTUAL_STORAGE:
		status = virtualStorage::deviceDispatchWrite(deviceObject, irp);
		break;
	default:
		TRACE("ERROR: Unreachable code reached");
		break;
	}
	requestHandled = true;
cleanup:
	if (!requestHandled) {
		TRACE("Request wasn't passed to device handler");
		irp->IoStatus.Status = status;
		irp->IoStatus.Information = 0;
		IoCompleteRequest(irp, IO_NO_INCREMENT);
	}
	return status;
}
