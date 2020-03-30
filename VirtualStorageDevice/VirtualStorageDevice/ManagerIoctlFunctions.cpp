#include "ManagerIoctlFunctions.h"
#include "Common.h"
#include "UtilFunctions.h"
#include "ntddstor.h"

namespace manager {

NTSTATUS ioctlDispatchCreateVirtualStorage(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	NTSTATUS status = STATUS_SUCCESS;
	const auto inputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	const auto length = stackLocation->Parameters.DeviceIoControl.InputBufferLength;
	IoctlCreateVirtualStorageParameter* inputParameter = nullptr;
	HANDLE fileHandle = nullptr;
	PFILE_OBJECT fileObject = nullptr;
	UNICODE_STRING fileName;
	PDEVICE_OBJECT fdoDevice = nullptr;
	PDEVICE_OBJECT pdoDevice = nullptr;
	PVirtualStorageDeviceExtension deviceExtension = nullptr;
	ULONG deviceID = 0;
	UNICODE_STRING deviceName = {0, 0, nullptr};
	ULONG targetNameLength = 0;
	PMOUNTMGR_TARGET_NAME targetName = nullptr;
	ULONG outputLength = 0;
	PMOUNTMGR_DRIVE_LETTER_TARGET driveLetterTarget = nullptr;
	MOUNTMGR_DRIVE_LETTER_INFORMATION driveLetterInformation = { false, L'\x00' };
	TRACE("ioctlDispatchCreateVirtualStorage called");
	
	TRACE("Verifying parameters");
	CHECK_STATUS(verifyBufferSize(inputBuffer, length, sizeof(IoctlCreateVirtualStorageParameter)), "Buffer verification failed");
	inputParameter = reinterpret_cast<IoctlCreateVirtualStorageParameter*>(inputBuffer);
	CHECK_AND_SET_STATUS((inputParameter->size + sizeof(IoctlCreateVirtualStorageParameter)) <= length, STATUS_INFO_LENGTH_MISMATCH, "Size mismatch or integer overflow");
	CHECK_AND_SET_STATUS((inputParameter->size + sizeof(IoctlCreateVirtualStorageParameter)) > inputParameter->size, STATUS_INTEGER_OVERFLOW, "Integer overflow");
	fileName.Length = inputParameter->size;
	fileName.MaximumLength = inputParameter->size;
	fileName.Buffer = inputParameter->path;
	CHECK_STATUS(openFile(&fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, &fileHandle), "openFile failed");
	CHECK_STATUS(ObReferenceObjectByHandle(fileHandle, FILE_READ_DATA, *IoFileObjectType, KernelMode, reinterpret_cast<PVOID*>(&fileObject), nullptr), "ObReferenceObjectByHandle failed");
	
	TRACE("Creating PDO");
	CHECK_STATUS(IoReportDetectedDevice(deviceObject->DriverObject, InterfaceTypeUndefined, static_cast<ULONG>(-1), static_cast<ULONG>(-1),
										nullptr, nullptr, false, &pdoDevice), "IoReportDetectedDevice failed");
	deviceID = InterlockedIncrement(reinterpret_cast<volatile LONG*>(&g_currentVirtualStorageDeviceID));
	CHECK_AND_SET_STATUS(deviceID, STATUS_INTEGER_OVERFLOW, "Max devices already created, integer overflow occured");
	TRACE("Generating device name");
	CHECK_STATUS(idToDeviceName(deviceID, &deviceName), "idToDeviceName failed");
	TRACE("Creating virtual storage device");
	CHECK_STATUS(IoCreateDeviceSecure(deviceObject->DriverObject, sizeof(VirtualStorageDeviceExtension), &deviceName, FILE_DEVICE_DISK,
									  FILE_DEVICE_SECURE_OPEN | /*FILE_CHARACTERISTIC_PNP_DEVICE |*/ FILE_REMOVABLE_MEDIA,
									  false, &SDDL_DEVOBJ_SYS_ALL_ADM_ALL, &g_deviceClassGuid, &fdoDevice), "IoCreateDevice failed");
	
	deviceExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(fdoDevice->DeviceExtension);
	RtlZeroMemory(deviceExtension, sizeof(VirtualStorageDeviceExtension));
	deviceExtension->header.magic = DEVICE_EXTENSION_HEADER_MAGIC;
	deviceExtension->header.type = DeviceType::VIRTUAL_STORAGE;
	deviceExtension->file.handle = fileHandle;
	deviceExtension->id = deviceID;
	deviceExtension->pdoDevice = pdoDevice;
	RtlCopyMemory(&(deviceExtension->deviceName), &deviceName, sizeof(UNICODE_STRING));
	CHECK_STATUS(FsRtlGetFileSize(fileObject, &(deviceExtension->file.size)), "FsRtlGetFileSize failed");
	CHECK_STATUS(verifyFileSize(deviceExtension->file.size, BYTES_PER_SECTOR, SECTORS_PER_TRACK, MAX_TRACK_PER_CYLINDER), "verifyFileSize failed");
	fdoDevice->Flags |= DO_VERIFY_VOLUME;
	fdoDevice->Flags |= DO_BUFFERED_IO;
	fdoDevice->Flags &= ~DO_DEVICE_INITIALIZING;
	
	targetNameLength = FIELD_OFFSET(MOUNTMGR_TARGET_NAME, DeviceName) + deviceName.Length;
	targetName = reinterpret_cast<PMOUNTMGR_TARGET_NAME>(ExAllocatePoolWithTag(PagedPool, targetNameLength, 'NveD'));
	CHECK_AND_SET_STATUS(targetName, STATUS_INSUFFICIENT_RESOURCES, "ExAllocatePoolWithTag failed");
	targetName->DeviceNameLength = deviceName.Length;
	RtlCopyMemory(targetName->DeviceName, deviceName.Buffer, targetName->DeviceNameLength);
	CHECK_STATUS(sendIoctlByDeviceName(MOUNTMGR_DEVICE_NAME, IOCTL_MOUNTMGR_VOLUME_ARRIVAL_NOTIFICATION, targetName, targetNameLength, nullptr, 0, &outputLength), "sendIoctlByDeviceName failed");
	// Looks bad but the structs are identical and it'd be a shame to duplicate the code
	driveLetterTarget = reinterpret_cast<PMOUNTMGR_DRIVE_LETTER_TARGET>(targetName);
	CHECK_STATUS(sendIoctlByDeviceName(MOUNTMGR_DEVICE_NAME, IOCTL_MOUNTMGR_NEXT_DRIVE_LETTER, driveLetterTarget, targetNameLength, &driveLetterInformation, sizeof(driveLetterInformation), &outputLength), "sendIoctlByDeviceName failed");
	if (outputLength == sizeof(driveLetterInformation)) {
		TRACE("Drive letter: %lc", driveLetterInformation.CurrentDriveLetter);
	}
	
	deviceExtension->lowerLevelDevice = IoAttachDeviceToDeviceStack(fdoDevice, deviceExtension->pdoDevice);
	CHECK_STATUS(IoRegisterDeviceInterface(deviceExtension->pdoDevice, &GUID_DEVINTERFACE_DISK, nullptr, &deviceExtension->diskSymbolicLinkName), "IoRegisterDeviceInterface failed");
	CHECK_STATUS(IoSetDeviceInterfaceState(&deviceExtension->diskSymbolicLinkName, true), "IoSetDeviceInterfaceState failed");

	CHECK_STATUS(IoRegisterDeviceInterface(deviceExtension->pdoDevice, &MOUNTDEV_MOUNTED_DEVICE_GUID, nullptr, &deviceExtension->mountSymbolicLinkName), "IoRegisterDeviceInterface failed");
	CHECK_STATUS(IoSetDeviceInterfaceState(&deviceExtension->mountSymbolicLinkName, true), "IoSetDeviceInterfaceState failed");
	
	CHECK_STATUS(IoVerifyVolume(fdoDevice, true), "IoVerifyVolume failed");
cleanup:
	DELETE_IF_NOT_NULL_MACRO(fileObject, ObDereferenceObject);
	FREE_IF_NOT_NULL(targetName, 'Nved');
	if (!NT_SUCCESS(status)) {
		if (deviceExtension) {
			DELETE_IF_NOT_NULL(deviceExtension->lowerLevelDevice, IoDetachDevice);
			if (deviceExtension->mountSymbolicLinkName.Buffer) {
				IoSetDeviceInterfaceState(&deviceExtension->mountSymbolicLinkName, false);
				RtlFreeUnicodeString(&deviceExtension->mountSymbolicLinkName);
				RtlZeroMemory(&deviceExtension->mountSymbolicLinkName, sizeof(UNICODE_STRING));
			}
			if (deviceExtension->diskSymbolicLinkName.Buffer) {
				IoSetDeviceInterfaceState(&deviceExtension->diskSymbolicLinkName, false);
				RtlFreeUnicodeString(&deviceExtension->diskSymbolicLinkName);
				RtlZeroMemory(&deviceExtension->diskSymbolicLinkName, sizeof(UNICODE_STRING));
			}
		}
		DELETE_IF_NOT_NULL(pdoDevice, IoDeleteDevice);
		DELETE_IF_NOT_NULL(fdoDevice, IoDeleteDevice);
		DELETE_IF_NOT_NULL(fileHandle, ZwClose);
		FREE_IF_NOT_NULL(deviceName.Buffer, 'NveD');
	}
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = 0;
	return STATUS_SUCCESS;
}

}