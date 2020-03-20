#pragma once
#include "Utils.h"

namespace manager {

NTSTATUS deviceDispatchIoctl(PDEVICE_OBJECT deviceObject, PIRP irp);

}
