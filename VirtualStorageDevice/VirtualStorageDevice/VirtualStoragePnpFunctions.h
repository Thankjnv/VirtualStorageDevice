#pragma once
#include "Utils.h"

namespace virtualStorage {

NTSTATUS pnpDispatchQueryDeviceRelations(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS pnpDispatchDeviceState(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

}
