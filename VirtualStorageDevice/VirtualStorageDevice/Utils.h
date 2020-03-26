#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include <wdmsec.h>
#include <initguid.h>
#include <mountmgr.h>

#define BYTES_PER_SECTOR 512

#define SECTORS_PER_TRACK 8

#define MAX_TRACK_PER_CYLINDER 1048576

#define DEVICE_EXTENSION_HEADER_MAGIC 0xAD1FD533

#define MAX_LONG_HEX_DIGITS 8

#define SECONDS_TO_NANOSECONDS(seconds) ((ULONG)(seconds * 1000000000))

#define ULONG_MAX 0xFFFFFFFF

// Maximum wait of 0.1 seconds
#define MAX_TIMEOUT_IOCTL (SECONDS_TO_NANOSECONDS(0.1) / 100)

#define TRACE(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, static_cast<ULONG>(-1), "[" __FUNCTION__ ":%d]  " format "\n", __LINE__, __VA_ARGS__)

#define CHECK_AND_SET_STATUS(expr, newStatus, failureMessage) \
	if (!(expr)) { \
		TRACE("%s", failureMessage); \
		status = newStatus; \
		goto cleanup; \
	}

// Set status to itself so it isn't changed
#define CHECK(expr, failureMessage) \
	CHECK_AND_SET_STATUS(expr, status, failureMessage);

#define CHECK_STATUS(expr, failureMessage) \
	status = (expr); \
	if (!NT_SUCCESS(status)) { \
		TRACE("%s. status=%lx", failureMessage, status); \
		goto cleanup; \
	}

// Delete a pointer if its not null, using "deleter" function. Any additional arguments will be passed to the deleter function.
#define DELETE_IF_NOT_NULL(ptr, deleter, ...) \
	if (ptr) { \
		deleter(ptr, __VA_ARGS__); \
		ptr = nullptr; \
	}

// Some deleters are macros and the __VA_ARGS__ syntax fails when calling a macro that doesn't take additional parameters
#define DELETE_IF_NOT_NULL_MACRO(ptr, deleter) \
	if (ptr) { \
		deleter(ptr); \
		ptr = nullptr; \
	}

#define FREE_IF_NOT_NULL(ptr, tag) \
	DELETE_IF_NOT_NULL(ptr, ExFreePoolWithTag, tag);

#define COUNT_OF(s) (sizeof(s) / sizeof((s)[0]))

// Get the LSB and second LSB, respectively
#define LOW_BYTE(num) ((UCHAR)((num) & 0xFF))
#define LOW_SECOND_BYTE(num) ((UCHAR)(((num) >> 8) & 0xFF))

// Get the second MSB and MSB, respectively
#define HIGH_BYTE(num) ((UCHAR)(((num) >> 24) & 0xFF))
#define HIGH_SECOND_BYTE(num) ((UCHAR)(((num) >> 16) & 0xFF))


#define _DEVICE_UNIQUE_ID(byte1, byte2, byte3, byte4) \
	{ 0x60cc79b4, 0x7448, 0x423a, { 0x84, 0xa0, 0x63, 0x13, (byte1), (byte2), (byte3), (byte4) } }; // {60CC79B4-7448-423A-84A0-6313 /* 4 bytes based on device ID*/}

#define DEVICE_UNIQUE_ID(deviceId) _DEVICE_UNIQUE_ID(LOW_BYTE(deviceId), LOW_SECOND_BYTE(deviceId), HIGH_SECOND_BYTE(deviceId), HIGH_BYTE(deviceId))


enum class DeviceType : ULONG {
	MANAGER = 0,
	VIRTUAL_STORAGE,
	MAX_DEVICE_TYPE,
};

// This struct must appear first in the DeviceExtension
typedef struct DeviceExtensionHeader {
	ULONG magic;
	DeviceType type;
} *PDeviceExtensionHeader;

typedef struct ManagerDeviceExtension {
	DeviceExtensionHeader header;
} *PManagerDeviceExtension;

typedef struct File {
	HANDLE handle;
	LARGE_INTEGER size;
} *PFile;

typedef struct VirtualStorageDeviceExtension {
	DeviceExtensionHeader header;
	File file;
	ULONG id;
	UNICODE_STRING deviceName;
	PDEVICE_OBJECT pdoDevice;
	PDEVICE_OBJECT lowerLevelDevice;
	UNICODE_STRING symbolicLinkName;
	
} *PVirtualStorageDeviceExtension;

extern const GUID g_deviceClassGuid;
extern volatile ULONG g_currentVirtualStorageDeviceID;