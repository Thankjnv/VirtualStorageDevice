#pragma once
#include "Utils.h"

namespace virtualStorage {

NTSTATUS ioctlDispatchGetLength(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchGetDriveGeometry(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchMediaRemoval(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchDiskIsWritable(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchQueryProperty(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchGetHotplugInfo(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchGetNestingLevel(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

}
