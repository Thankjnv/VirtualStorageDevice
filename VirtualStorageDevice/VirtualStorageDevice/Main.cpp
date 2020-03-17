#include "Common.h"

void DriverUnload(PDRIVER_OBJECT driverObject) {
	UNREFERENCED_PARAMETER(driverObject);
 	TRACE("Goodbye Driver");
}

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath) {
	UNREFERENCED_PARAMETER(registryPath);
	driverObject->DriverUnload = DriverUnload;
	TRACE("Hello Driver");
	return STATUS_SUCCESS;
}
