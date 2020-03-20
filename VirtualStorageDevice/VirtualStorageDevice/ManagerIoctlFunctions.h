#pragma once
#include "Utils.h"

namespace manager {

NTSTATUS ioctlDispatchCreateVirtualStorage(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

}


