/*
	Istominsoft, Istomin A.O., 06.2022
	Модуль автоматического перечисления устройств USB и определения порта
    а так же подключения и отключения нужного
*/

#ifndef DeviceEnumeratorH
#define DeviceEnumeratorH

// Класс перечислитель устройств USB
class TDeviceEnumerator
{
	private:
		UnicodeString TargetDeviceDesc;
		WORD TargetDeviceVID, TargetDevicePID;
		bool TargetOnline;
		bool HasError;
		int TargetPort;

		void EnumDevices();
		void OnSystemConnectDev();
		void OnSystemDisconnectDev();

		void (__closure *OnConnectedCallBack)(int);
		void (__closure *OnDisconnectedCallBack)();

	public:
		TDeviceEnumerator(UnicodeString TargetDesc, WORD TargetPID, WORD TargetVID,
			void (__closure *OnConnectedCbk)(int),
			void (__closure *OnDisconnectedCbk)());
		~TDeviceEnumerator();
		bool TargetConnected();
		void OnSystemDeviceChanged(TMessage& a);
};

/*

[!] Для корректной работы необходимо вызывать обработчик сообщения WM_DEVICECHANGE.
Для этого пишем в заголовочнике класса главного окна программы в public'е:

	// Регаем обработчик
	BEGIN_MESSAGE_MAP
	   MESSAGE_HANDLER(WM_DEVICECHANGE, TMessage, OnWinDevicesChange)
	END_MESSAGE_MAP(TForm)
	// Сам обработчик
	void __fastcall OnWinDevicesChange(TMessage& a);
	// Калбак подключения
	void OnMyDeviceConnect(int PortNo);
	// Калбак отключения
	void OnMyDeviceDisconnect();


Далее в коде юнита главного окна:

    // Енумератор
	TDeviceEnumerator* MainUSBEnum = NULL;

	// Обработчик сообщения о смене устройств ОС
	void __fastcall Tfrm::OnWinDevicesChange(TMessage& a)
	{
		if (MainUSBEnum != NULL) MainUSBEnum->OnSystemDeviceChanged(a);
	}

	// Калбак подключения
	void Tfrm::OnMyDeviceConnect(int PortNo)
	{
		// ...
	}

	// Калбак отключения
	void Tfrm::OnMyDeviceDisconnect()
	{
		// ...
	}

В событии показа формы, ну, или где надо создаем экземпляр:

	void __fastcall Tfrm::FormShow(TObject *Sender)
	{
		MainUSBEnum = new TDeviceEnumerator(<строка имя устройства шинное>, <PID HEX>, <VID HEX>,
			OnMyDeviceConnect, OnMyDeviceDisconnect);
	}

P.S.: <строка имя устройства шинное> - PRODUCT_STRING (Product Identifier) указанное в дескрипторе USB

*/

#endif
