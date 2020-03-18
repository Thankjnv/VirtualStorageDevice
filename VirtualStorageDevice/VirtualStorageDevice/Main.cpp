#include "Common.h"
#include "DriverFunctions.h"
#define DRIVER_NAME L"VirtualStorageDevice"
#define DEVICE_NAME L"\\Device\\" DRIVER_NAME
#define LINK_NAME L"\\DosDevices\\" DRIVER_NAME

void driverCleanup(PDRIVER_OBJECT driverObject) {
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING linkName = RTL_CONSTANT_STRING(LINK_NAME);
	PDEVICE_OBJECT currDevice = driverObject->DeviceObject;
	PDEVICE_OBJECT nextDevice = nullptr;
	status = IoDeleteSymbolicLink(&linkName);
	if (!NT_SUCCESS(status)) {
		TRACE("IoDeleteSymbolicLink failed. status=%lx", status);
	}

	while (currDevice) {
		nextDevice = currDevice->NextDevice;
		IoDeleteDevice(currDevice);
		currDevice = nextDevice;
	}
}

void driverUnload(PDRIVER_OBJECT driverObject) {
	TRACE("%ls unload called", DRIVER_NAME);
	driverCleanup(driverObject);
}

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath) {
	UNREFERENCED_PARAMETER(registryPath);
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(DEVICE_NAME);
	UNICODE_STRING linkName = RTL_CONSTANT_STRING(LINK_NAME);
	PDEVICE_OBJECT device = nullptr;
	driverObject->DriverUnload = driverUnload;
	TRACE("%ls entry called", DRIVER_NAME);
	CHECK_STATUS(IoCreateDevice(driverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, false, &device), "IoCreateDevice failed");
	CHECK_STATUS(IoCreateSymbolicLink(&linkName, &deviceName), "IoCreateSymbolicLink failed");

	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; ++i) {
		driverObject->MajorFunction[i] = unimplementedMajorFunction;
	}

	driverObject->MajorFunction[IRP_MJ_CREATE] = deviceDispatchCreate;
	driverObject->MajorFunction[IRP_MJ_CLOSE] = deviceDispatchClose;
	driverObject->MajorFunction[IRP_MJ_CLEANUP] = deviceDispatchCleanup;
	driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = deviceDispatchIoctl;
	
	device->Flags |= DO_BUFFERED_IO;
	device->Flags &= ~DO_DEVICE_INITIALIZING;
	
cleanup:
	if (!NT_SUCCESS(status)) {
		driverCleanup(driverObject);
	}
	return status;
}
