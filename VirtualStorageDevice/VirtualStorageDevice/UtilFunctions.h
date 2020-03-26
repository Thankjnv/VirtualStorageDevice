#pragma once
#include "Utils.h"

NTSTATUS openFile(PUNICODE_STRING fileName, ULONG desiredAccess, ULONG shareAccess, _Out_ PHANDLE fileHandle);

NTSTATUS verifyDeviceExtensionHeader(PDEVICE_OBJECT deviceObject);

NTSTATUS getDeviceTypeName(PDEVICE_OBJECT deviceObject, PCHAR* deviceTypeName);

NTSTATUS idToDeviceName(ULONG id, _Out_ PUNICODE_STRING deviceName);

NTSTATUS sendIoctlByDevicePointer(PDEVICE_OBJECT deviceObject, ULONG ioctlCode, PVOID inputBuffer, ULONG inputBufferLength,
								  PVOID outputBuffer, ULONG outputBufferLength, _Out_ ULONG* outputBufferFinalLength);

NTSTATUS sendIoctlByDeviceName(PCWSTR deviceNamePtr, ULONG ioctlCode, PVOID inputBuffer, ULONG inputBufferLength,
							   PVOID outputBuffer, ULONG outputBufferLength, _Out_ ULONG* outputBufferFinalLength);

NTSTATUS verifyBufferSize(PVOID buffer, ULONG bufferSize, ULONG requiredSize);

NTSTATUS verifyFileSize(LARGE_INTEGER fileSize, ULONG bytesPerSector, ULONG sectorsPerTrack, ULONG maxTracksPerCylinder);
