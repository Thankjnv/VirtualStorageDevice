#include "Utils.h"
#include "DriverFunctions.h"

#define DRIVER_NAME L"VirtualStorageDevice"
#define MANAGER_DEVICE_NAME L"VirtualStorageDeviceManager"
#define DEVICE_NAME L"\\Device\\" MANAGER_DEVICE_NAME
#define LINK_NAME L"\\DosDevices\\" MANAGER_DEVICE_NAME

void driverCleanup(PDRIVER_OBJECT driverObject) {
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING linkName = RTL_CONSTANT_STRING(LINK_NAME);
	PDEVICE_OBJECT currDevice = driverObject->DeviceObject;
	PDEVICE_OBJECT nextDevice = nullptr;
	TRACE("Deleting symbolic link");
	status = IoDeleteSymbolicLink(&linkName);
	if (!NT_SUCCESS(status)) {
		TRACE("IoDeleteSymbolicLink failed. status=%lx", status);
	}

	while (currDevice) {
		nextDevice = currDevice->NextDevice;
		if ((reinterpret_cast<PDeviceExtensionHeader>(currDevice->DeviceExtension)->type) == DeviceType::VIRTUAL_STORAGE) {
			TRACE("Deleting virtual storage device");
			const auto virtualStorageExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(currDevice->DeviceExtension);
			TRACE("Closing virtual storage's file");
			status = ZwClose(virtualStorageExtension->file.handle);
			if (!NT_SUCCESS(status)) {
				TRACE("ZwClose failed. status=%lx", status);
			}
			virtualStorageExtension->file.handle = nullptr;
			DELETE_IF_NOT_NULL(virtualStorageExtension->lowerLevelDevice, IoDetachDevice);
			if (virtualStorageExtension->mountSymbolicLinkName.Buffer) {
				IoSetDeviceInterfaceState(&virtualStorageExtension->mountSymbolicLinkName, false);
				TRACE("Deleting PDO mount symbolic link name");
				RtlFreeUnicodeString(&virtualStorageExtension->mountSymbolicLinkName);
				RtlZeroMemory(&virtualStorageExtension->mountSymbolicLinkName, sizeof(UNICODE_STRING));
			}
			if (virtualStorageExtension->diskSymbolicLinkName.Buffer) {
				IoSetDeviceInterfaceState(&virtualStorageExtension->diskSymbolicLinkName, false);
				TRACE("Deleting PDO device symbolic link name");
				RtlFreeUnicodeString(&virtualStorageExtension->diskSymbolicLinkName);
				RtlZeroMemory(&virtualStorageExtension->diskSymbolicLinkName, sizeof(UNICODE_STRING));
			}
			FREE_IF_NOT_NULL(virtualStorageExtension->deviceName.Buffer, 'NveD');
			TRACE("Deleting PDO");
			DELETE_IF_NOT_NULL(virtualStorageExtension->pdoDevice, IoDeleteDevice);
		} else {
			TRACE("Deleting manager");
		}
		TRACE("Deleting device");
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
	PManagerDeviceExtension deviceExtension = nullptr;
	TRACE("%ls entry called", DRIVER_NAME);
	driverObject->DriverUnload = driverUnload;
	driverObject->DriverExtension->AddDevice = virtualStorageAddDevice;
	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; ++i) {
		driverObject->MajorFunction[i] = unimplementedMajorFunction;
	}

	driverObject->MajorFunction[IRP_MJ_CREATE] = deviceDispatchCreate;
	driverObject->MajorFunction[IRP_MJ_CLOSE] = deviceDispatchClose;
	driverObject->MajorFunction[IRP_MJ_CLEANUP] = deviceDispatchCleanup;
	driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = deviceDispatchIoctl;
	driverObject->MajorFunction[IRP_MJ_READ] = deviceDispatchRead;
	driverObject->MajorFunction[IRP_MJ_WRITE] = deviceDispatchWrite;
	driverObject->MajorFunction[IRP_MJ_PNP] = deviceDispatchPnp;
	
	TRACE("Creating manager device");
	CHECK_STATUS(IoCreateDevice(driverObject, sizeof(ManagerDeviceExtension), &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN | FILE_CHARACTERISTIC_PNP_DEVICE, false, &device), "IoCreateDevice failed");
	deviceExtension = reinterpret_cast<PManagerDeviceExtension>(device->DeviceExtension);
	deviceExtension->header.magic = DEVICE_EXTENSION_HEADER_MAGIC;
	deviceExtension->header.type = DeviceType::MANAGER;
	TRACE("Creating symbolic link at %wZ", &linkName);
	CHECK_STATUS(IoCreateSymbolicLink(&linkName, &deviceName), "IoCreateSymbolicLink failed");

	device->Flags |= DO_BUFFERED_IO;
	device->Flags &= ~DO_DEVICE_INITIALIZING;
cleanup:
	if (!NT_SUCCESS(status)) {
		driverCleanup(driverObject);
	}
	return status;
}
