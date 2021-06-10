#include <windows.h>
#include <stdio.h>
#include <setupapi.h>
#include <initguid.h>
#include <usbiodef.h>
#include <Devpkey.h>

#pragma comment (lib, "Setupapi.lib")

BOOL USBDisable(VOID);

int main(VOID)
{
	DWORD dwError = ERROR_SUCCESS;

	if (!USBDisable())
		goto FAILURE;


	return ERROR_SUCCESS;

FAILURE:
	dwError = GetLastError();

	return dwError;
}

BOOL USBDisable(VOID)
{
	DWORD dwError = ERROR_SUCCESS;
	HDEVINFO DeviceInfoSet = INVALID_HANDLE_VALUE;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD DeviceIndex = 0;
	DWORD dwBuffersize;
	PBYTE devBuffer = 0;
	DWORD dwRegType;

	SP_CLASSINSTALL_HEADER header;
	header.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
	header.InstallFunction = DIF_PROPERTYCHANGE;

	SP_PROPCHANGE_PARAMS ChangeDeviceParams;
	ChangeDeviceParams.ClassInstallHeader = header;
	ChangeDeviceParams.StateChange = DICS_DISABLE;
	ChangeDeviceParams.Scope = DICS_FLAG_GLOBAL;
	ChangeDeviceParams.HwProfile = 0;

	//Open a handle to the device interface information set of all USB devices present.

	DeviceInfoSet = SetupDiGetClassDevsW((LPGUID)&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (DeviceInfoSet == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	ZeroMemory(&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	while (true)
	{
		if (!SetupDiEnumDeviceInfo(DeviceInfoSet, DeviceIndex, &DeviceInfoData))
		{
			dwError = GetLastError();
			if (dwError == ERROR_NO_MORE_ITEMS)
			{
				break;
			}

			else
			{
				goto FAILURE;
			}
		}

		SetupDiGetDeviceRegistryPropertyW(DeviceInfoSet, &DeviceInfoData, SPDRP_HARDWAREID, &dwRegType, NULL, 0, &dwBuffersize);
		

		devBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBuffersize);

		if (devBuffer == NULL)
		{
			goto FAILURE;
		}
			
		if (!SetupDiGetDeviceRegistryPropertyW(DeviceInfoSet, &DeviceInfoData, SPDRP_HARDWAREID, &dwRegType, (PBYTE)devBuffer, dwBuffersize, 0))
		{
			goto FAILURE;
		}

		if (!SetupDiSetClassInstallParamsW(DeviceInfoSet, &DeviceInfoData, (SP_CLASSINSTALL_HEADER*)&ChangeDeviceParams, sizeof(ChangeDeviceParams)))
		{
			goto FAILURE;
		}

		if (!SetupDiChangeState(DeviceInfoSet, &DeviceInfoData))
		{
			goto FAILURE;
		}

		SetupDiDestroyDeviceInfoList(DeviceInfoSet);
		HeapFree(GetProcessHeap(), 0, devBuffer);
		DeviceIndex++;
	}

	return TRUE;

FAILURE:

	dwError = GetLastError();
	HeapFree(GetProcessHeap(), 0, devBuffer);
	SetupDiDestroyDeviceInfoList(DeviceInfoSet);

	return dwError;
}
