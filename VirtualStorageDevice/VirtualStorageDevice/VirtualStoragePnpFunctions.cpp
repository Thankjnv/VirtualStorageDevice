#include "VirtualStoragePnpFunctions.h"
#include "UtilFunctions.h"

namespace virtualStorage {

NTSTATUS pnpDispatchQueryDeviceRelations(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("pnpDispatchQueryDeviceRelations called");
	PDEVICE_RELATIONS deviceRelations = nullptr;
	auto deviceExtension = reinterpret_cast<PVirtualStorageDeviceExtension>(deviceObject->DeviceExtension);
	CHECK_STATUS(verifyDeviceExtensionHeader(deviceObject), "verifyDeviceExtensionHeader failed");
	switch (stackLocation->Parameters.QueryDeviceRelations.Type) {
	case BusRelations:
	case EjectionRelations:
	case PowerRelations:
	case RemovalRelations:
	case TargetDeviceRelation:
		TRACE("Query type: %lx", stackLocation->Parameters.QueryDeviceRelations.Type);
		deviceRelations = reinterpret_cast<PDEVICE_RELATIONS>(ExAllocatePoolWithTag(PagedPool, sizeof(*deviceRelations), 'RveD'));
		CHECK_AND_SET_STATUS(deviceRelations, STATUS_INSUFFICIENT_RESOURCES, "ExAllocatePoolWithTag failed");
		deviceRelations->Count = 1;
		ObReferenceObject(deviceExtension->pdoDevice);
		deviceRelations->Objects[0] = deviceExtension->pdoDevice;
		break;
	default:
		status = STATUS_NOT_IMPLEMENTED;
		break;
	}

cleanup:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = reinterpret_cast<ULONG_PTR>(deviceRelations);
	return status;
}

NTSTATUS pnpDispatchDeviceState(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation) {
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(stackLocation);
	NTSTATUS status = STATUS_SUCCESS;
	TRACE("pnpDispatchDeviceState called");
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = 0;
	return status;
}

}
