#pragma once
#include "Common.h"

NTSTATUS ioctlDispatchTest(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	auto inputBuffer = reinterpret_cast<char*>(irp->AssociatedIrp.SystemBuffer);
	auto length = stackLocation->Parameters.DeviceIoControl.InputBufferLength;
	char* bufferCopy = nullptr;
	TRACE("ioctlDispatchTest called");
	if (!inputBuffer || !length) {
		TRACE("No input buffer provided");
		status = STATUS_INVALID_PARAMETER;
		goto cleanup;
	}
	bufferCopy = reinterpret_cast<char*>(ExAllocatePoolWithTag(PagedPool, length + 1, 'TSET'));
	CHECK(bufferCopy, "ExAllocatePoolWithTag failed");
	RtlCopyMemory(bufferCopy, inputBuffer, length);
	bufferCopy[length] = '\x00';
	TRACE("User buffer: %s", bufferCopy);
cleanup:
	FREE_IF_NOT_NULL(bufferCopy, 'TSET');
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = 0;
	return STATUS_SUCCESS;
}

