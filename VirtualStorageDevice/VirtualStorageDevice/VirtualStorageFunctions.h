#pragma once
#include "Utils.h"

namespace virtualStorage {

NTSTATUS deviceDispatchIoctl(PDEVICE_OBJECT deviceObject, PIRP irp);

NTSTATUS deviceDispatchRead(PDEVICE_OBJECT deviceObject, PIRP irp);

NTSTATUS deviceDispatchWrite(PDEVICE_OBJECT deviceObject, PIRP irp);

NTSTATUS deviceDispatchPnp(PDEVICE_OBJECT deviceObject, PIRP irp);

}
