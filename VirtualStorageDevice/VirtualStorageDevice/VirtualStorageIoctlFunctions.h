#pragma once
#include "Utils.h"

namespace virtualStorage {

NTSTATUS ioctlDispatchGetLength(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchGetDriveGeometry(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchNoHandleReturnSuccess(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchGetHotplugInfo(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchGetNestingLevel(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchCheckVerify(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchQueryDeviceName(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchQueryDeviceUniqueId(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchVolumeIsDynamic(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchDiskExtents(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

NTSTATUS ioctlDispatchGetPartitionInfo(PDEVICE_OBJECT deviceObject, PIRP irp, PIO_STACK_LOCATION stackLocation);

}
