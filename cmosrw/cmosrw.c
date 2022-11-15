#include <windows.h>
#include <stdio.h>
#include "../cmosrwdrv/cmosrwdrv/cmosrwdrv_ioctl.h"

int validHex(char* str)
{
	DWORD len = (DWORD)strlen(str);
	if (len > 2) return FALSE; // максимум 255
	DWORD i;
	UCHAR ch;
	for (i = 0; i < len; i++)
	{
		ch = str[i];
		if ((ch < '0' || ch > '9') &&
			(ch < 'A' || ch > 'F') &&
			(ch < 'a' || ch > 'f'))
		{
			return FALSE;
		}
	}
	return TRUE;
}

int main(int argc, char** argv)
{
	ULONG ioctlCode = IOCTL_READ_CMOS;
	UCHAR addr = 0;
	UCHAR value = 0;
	DWORD bytesCount = 0;
	BOOL success;

	if (argc == 1) {
		printf("See usage guide: cmosrw --help\n");
		return -1;
	}
	else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
		printf("cmosrw read [address]\n");
		printf("cmosrw write [address] [value]\n\n");
		printf("address and value in HEX format. Max - FF\n");
	}
	else if (!strcmp(argv[1], "write"))
	{
		if (argc != 4)
		{
			printf("Invalid parameters. See usage guide: cmosrw --help\n");
			return -1;
		}
		if (validHex(argv[2]) && validHex(argv[3]))
		{
			addr = (unsigned char)strtol(argv[2], NULL, 16);
			value = (unsigned char)strtol(argv[3], NULL, 16);
			ioctlCode = IOCTL_WRITE_CMOS;
		}
		else
		{
			printf("Invalid parameters. See usage guide: cmosrw --help\n");
			return -1;
		}
	}
	else if (!strcmp(argv[1], "read"))
	{
		if (argc != 3)
		{
			printf("Invalid parameters. See usage guide: cmosrw --help\n");
			return -1;
		}
		if (validHex(argv[2]))
		{
			addr = (unsigned char)strtol(argv[2], NULL, 16);
			value = 0;
			ioctlCode = IOCTL_READ_CMOS;
		}
		else
		{
			printf("Invalid parameters. See usage guide: cmosrw --help\n");
			return -1;
		}
	}
	else {
		printf("Unknown command. See usage guide: cmosrw --help\n");
		return -1;
	}



	HANDLE handle = CreateFile(
		L"\\\\.\\cmosrwdrv",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (handle == INVALID_HANDLE_VALUE)
	{
		printf("ERROR: Can not access driver!\n");
		return -1;
	}

	if (ioctlCode == IOCTL_READ_CMOS)
	{
		CMD_READ cmd_read;
		RESULT_READ result_read;
		cmd_read.addr = addr;

		success = DeviceIoControl(
			handle,
			ioctlCode,
			&cmd_read, sizeof(cmd_read),		// Input
			&result_read, sizeof(result_read),	// Output
			&bytesCount,
			(LPOVERLAPPED)NULL);

		if (!success)
		{
			printf("ERROR: Last Error Code %X\n", GetLastError());
			CloseHandle(handle);
			return -1;
		}

		printf("DEC : addr %u = %u\n", cmd_read.addr, result_read.value);
		printf("HEX : addr %X = %X\n", cmd_read.addr, result_read.value);
		printf("Bytes read : %d\n", bytesCount);
	}
	else //== IOCTL_WRITE_CMOS
	{
		CMD_WRITE cmd_write;
		cmd_write.addr = addr;
		cmd_write.value = value;

		success = DeviceIoControl(
			handle,
			ioctlCode,
			&cmd_write, sizeof(cmd_write),		// Input
			NULL, 0,							// Output
			&bytesCount,
			(LPOVERLAPPED)NULL);

		if (!success)
		{
			printf("ERROR: Last Error Code %X\n", GetLastError());
			CloseHandle(handle);
			return -1;
		}

		printf("Write success");
	}

	CloseHandle(handle);

	return 0;
}