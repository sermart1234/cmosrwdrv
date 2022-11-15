#include <ntddk.h>
#include "cmosrwdrv_ioctl.h"

//загуглить про секции и страницы памяти и тд
//как происходит переключение между рингами и вызов системных функций // подробнее про прерывания
// IRQL 
//IRP_MJ_READ  IRP_MJ_WRITE IOCTL по аналогии в лине?
//9 глава книги //wdm ничего нипанятна
//что будет если имя девайса NULL


typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT	DeviceObject;
	UNICODE_STRING	ustrSymLinkName;
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

NTSTATUS DeviceControlRoutine(PDEVICE_OBJECT fdo, PIRP Irp);
VOID     UnloadRoutine(PDRIVER_OBJECT DriverObject);
NTSTATUS CloseRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS CreateRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS NotSupportedRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS CompleteIrp(PIRP Irp, NTSTATUS status, ULONG info);

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT  deviceObject;
	UNICODE_STRING  devName;

	for (SIZE_T i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DriverObject->MajorFunction[i] = NotSupportedRoutine;
	}

	DriverObject->DriverUnload = UnloadRoutine;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateRoutine;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseRoutine;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControlRoutine;

	RtlInitUnicodeString(&devName, L"\\Device\\cmosrwdrv");

	status = IoCreateDevice(
		DriverObject,
		sizeof(DEVICE_EXTENSION),
		&devName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&deviceObject);

	if (!NT_SUCCESS(status)) //именно не успешно. есть варнинги, ошибки и инфо
	{
		return status;
	}

	PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION)deviceObject->DeviceExtension;
	deviceExtension->DeviceObject = deviceObject;

	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, SYM_LINK_NAME); //L"\\DosDevices\\cmosrwdrv"
	deviceExtension->ustrSymLinkName = symLinkName;

	status = IoCreateSymbolicLink(&symLinkName, &devName);

	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(deviceObject);
		return status;
	}

	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
VOID UnloadRoutine(PDRIVER_OBJECT DriverObject)
{
	PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION)DriverObject->DeviceObject->DeviceExtension;
	UNICODE_STRING* pLinkName = &(deviceExtension->ustrSymLinkName);

	IoDeleteSymbolicLink(pLinkName);
	IoDeleteDevice(DriverObject->DeviceObject);
}
#pragma code_seg()

NTSTATUS CompleteIrp(PIRP Irp, NTSTATUS status, ULONG info)
{
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS NotSupportedRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	return CompleteIrp(Irp, STATUS_NOT_SUPPORTED, 0);
}

NTSTATUS CreateRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	return CompleteIrp(Irp, STATUS_SUCCESS, 0);
}

NTSTATUS CloseRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	return CompleteIrp(Irp, STATUS_SUCCESS, 0);
}


NTSTATUS DeviceControlRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PIO_STACK_LOCATION ioStackLocation = IoGetCurrentIrpStackLocation(Irp);

	switch (ioStackLocation->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_READ_CMOS:
	{
		if (ioStackLocation->Parameters.DeviceIoControl.InputBufferLength != sizeof(CMD_READ) ||
			ioStackLocation->Parameters.DeviceIoControl.OutputBufferLength != sizeof(RESULT_READ))
		{
			return CompleteIrp(Irp, STATUS_INVALID_BUFFER_SIZE, 0);
		}

		PCMD_READ cmd_read = (PCMD_READ)Irp->AssociatedIrp.SystemBuffer;
		PRESULT_READ result_read = (PRESULT_READ)Irp->AssociatedIrp.SystemBuffer;

		WRITE_PORT_UCHAR((PUCHAR)(ULONG_PTR)0x70, cmd_read->addr);
		result_read->value = READ_PORT_UCHAR((PUCHAR)0x71);

		return CompleteIrp(Irp, STATUS_SUCCESS, sizeof(RESULT_READ));
		//break;
	}
	case IOCTL_WRITE_CMOS:
	{
		if (ioStackLocation->Parameters.DeviceIoControl.InputBufferLength != sizeof(CMD_WRITE) ||
			ioStackLocation->Parameters.DeviceIoControl.OutputBufferLength != 0)
		{
			return CompleteIrp(Irp, STATUS_INVALID_BUFFER_SIZE, 0);
		}

		PCMD_WRITE cmd_write = (PCMD_WRITE)Irp->AssociatedIrp.SystemBuffer;

		WRITE_PORT_UCHAR((PUCHAR)(ULONG_PTR)0x70, cmd_write->addr);
		WRITE_PORT_UCHAR((PUCHAR)(ULONG_PTR)0x71, cmd_write->value);

		return CompleteIrp(Irp, STATUS_SUCCESS, 0);
		//break;
	}
	default:
	{
		return CompleteIrp(Irp, STATUS_NOT_SUPPORTED, 0);
		//break;
	}
	}
}