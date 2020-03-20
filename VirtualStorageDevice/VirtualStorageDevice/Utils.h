#pragma once
#include <ntifs.h>
#include <ntddk.h>

#define BYTES_PER_SECTOR 512

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

#define FREE_IF_NOT_NULL(ptr, tag) \
	DELETE_IF_NOT_NULL(ptr, ExFreePoolWithTag, tag);

enum class DeviceType : ULONG {
	MANAGER = 0,
	VIRTUAL_STORAGE,
	MAX_DEVICE_TYPE,
};

// This struct must appear first in the DeviceExtension
struct DeviceExtensionHeader {
	DeviceType type;
};

struct ManagerDeviceExtension {
	DeviceExtensionHeader header;
};

struct VirtualStorageDeviceExtension {
	DeviceExtensionHeader header;
	HANDLE fileHandle;
};
