#pragma once
#include <ntddk.h>

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

#define FREE_IF_NOT_NULL(ptr, tag) \
	if (ptr) { \
		ExFreePoolWithTag(ptr, tag); \
		ptr = nullptr; \
	}
