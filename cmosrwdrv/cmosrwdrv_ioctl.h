//#pragma once
#ifndef __CMOSRWDRV_IOCTL_H__
#define __CMOSRWDRV_IOCTL_H__

#define IOCTL_READ_CMOS CTL_CODE( \
	FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_WRITE_CMOS CTL_CODE(\
	FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS) //эни можно заместить на write / read

#define	SYM_LINK_NAME	L"\\DosDevices\\cmosrwdrv"

/*
typedef struct _DRIVER_CMD
{
	UCHAR addr;
	UCHAR value;
} DRIVER_CMD, *PDRIVER_CMD;*/

/*
typedef struct _DRIVER_RESULT
{
	UCHAR addr;
	UCHAR value;
} DRIVER_RESULT;*/

typedef struct _CMD_WRITE
{
	UCHAR addr;
	UCHAR value;
} CMD_WRITE, * PCMD_WRITE;

typedef struct _CMD_READ
{
	UCHAR addr;
} CMD_READ, * PCMD_READ;

typedef struct _RESULT_READ
{
	UCHAR value;
} RESULT_READ, * PRESULT_READ;

#endif //__CMOSRWDRV_IOCTL_H__

