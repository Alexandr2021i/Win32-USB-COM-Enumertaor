#pragma hdrstop
#include "DeviceEnumerator.h"

#include "dbt.h"
#include "setupapi.h"
#include <initguid.h>
#include <devpkey.h>
#include <devpropdef.h>
#include <winreg.h>

#pragma package(smart_init)

TDeviceEnumerator::TDeviceEnumerator(UnicodeString TargetDesc, WORD TargetPID, WORD TargetVID,
	void (__closure *OnConnectedCbk)(int), void (__closure *OnDisconnectedCbk)())
{
	TargetDeviceDesc = TargetDesc;
	TargetDeviceVID = TargetVID;
	TargetDevicePID = TargetPID;
	TargetOnline = false;
	HasError = false;
	OnConnectedCallBack = OnConnectedCbk;
	OnDisconnectedCallBack = OnDisconnectedCbk;
	TargetPort = -1;

	// Первоначальный скан
    OnSystemConnectDev();
}

TDeviceEnumerator::~TDeviceEnumerator() { }

// Перечислитель устройств
void TDeviceEnumerator::EnumDevices()
{
	const DWORD DataBufSize = 2048;
	wchar_t Buf[DataBufSize];
	HDEVINFO hDevInfo = { 0 };
	SP_DEVINFO_DATA DeviceInfoData = { 0 };
	DWORD DataT = 0;
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	HasError = false;

	// Получаем список всех устройств от перечислителя USB
	const wchar_t* Enum = L"USB";
	hDevInfo = SetupDiGetClassDevs(NULL, Enum,	0, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		HasError = true;
		return;
	}

	// Перечисляем все устройства в наборе
	UnicodeString CurrentDevDescStr;
	WORD CurrentPID, CurrentVID;
	for (DWORD i=0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
	{
        // VID & PID
		if (SetupDiGetDeviceProperty(hDevInfo, &DeviceInfoData, &DEVPKEY_Device_HardwareIds, &DataT, (PBYTE)Buf, sizeof(Buf), NULL, 0))
		{
            // Парсим...
			UnicodeString tmpVP = Buf;
			// VID
			UnicodeString tmpStr = tmpVP.SubString(tmpVP.Pos("VID_") + 4, tmpVP.Pos("&PID_") - (tmpVP.Pos("VID_") + 4));
			CurrentVID = (tmpStr.Length() > 3)? StrToInt("$" + tmpStr) : 0;
            // PID
			tmpStr = tmpVP.SubString(tmpVP.Pos("&PID_") + 5, tmpVP.Pos("&REV_") - (tmpVP.Pos("&PID_") + 5));
			CurrentPID = (tmpStr.Length() > 3)? StrToInt("$" + tmpStr) : 0;
		}

		// Определяемый шиной текстовый описатель
		if (SetupDiGetDeviceProperty(hDevInfo, &DeviceInfoData, &DEVPKEY_Device_BusReportedDeviceDesc, &DataT, (PBYTE)Buf, sizeof(Buf), NULL, 0))
		{
			CurrentDevDescStr = Buf;
		}

		// Проверяем наше или нет устройство
		TargetOnline = ((TargetDeviceDesc == CurrentDevDescStr) &&
						(TargetDeviceVID == CurrentVID) &&
						(TargetDevicePID == CurrentPID));

		if (TargetOnline)
		{
			// Определяем порт
			HKEY DevKey = SetupDiOpenDevRegKey(hDevInfo, &DeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_EXECUTE);
			if (DevKey == INVALID_HANDLE_VALUE)
			{
                // Не удалось определить
                TargetPort = -1;
			}
			else
			{
				// Читаем номер порта
				const wchar_t* ValName = L"PortName";
				DWORD ValSize = sizeof(Buf);
				if (ERROR_SUCCESS == RegQueryValueExW(DevKey, ValName, NULL, &DataT, (PBYTE)Buf, &ValSize))
				{
					UnicodeString Str = Buf;
                    TargetPort = (Str.Length() > 3)? StrToInt(Str.SubString(4, Str.Length() - 3)) : -1;
				}
				else
				{
					// Не удалось прочитать
					TargetPort = -1;
				}
				RegCloseKey(DevKey);
            }

            // Больше искать не надо - выходим
			break;
		}
	}

    // Проверка на ошибки
	if ((GetLastError()!= NO_ERROR) && (GetLastError()!= ERROR_NO_MORE_ITEMS))
	{
		HasError = true;
	}

	// Удаляем список
	SetupDiDestroyDeviceInfoList(hDevInfo);
}

// Подключили устройство
void TDeviceEnumerator::OnSystemConnectDev()
{
	if (TargetConnected()) return;

	EnumDevices();

	if (TargetConnected())
	{
		// Наше подключилось...
		// Определить номер COM порта

		if (OnConnectedCallBack) OnConnectedCallBack(TargetPort);
	}
}

// Отключили устройство
void TDeviceEnumerator::OnSystemDisconnectDev()
{
	if (!TargetConnected()) return;

	EnumDevices();

	if (!TargetConnected())
	{
	    TargetPort = -1;
		if (OnDisconnectedCallBack) OnDisconnectedCallBack();
	}
}

// Смена кол-ва устройств в ОСи
void TDeviceEnumerator::OnSystemDeviceChanged(TMessage& a)
{
  switch (a.WParam)
  {
	case DBT_DEVICEARRIVAL:
	{
		OnSystemConnectDev();
	}
	break;

	case DBT_DEVICEREMOVECOMPLETE:
	{
		OnSystemDisconnectDev();
	}
	break;
  }
}

// Подключено ли
bool TDeviceEnumerator::TargetConnected()
{
	return TargetOnline && !HasError;
}
