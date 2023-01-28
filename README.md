# USB-COM-Enumerator

Автоматичсекий перечислитель устройств с определением нужного по VID, PID и ProductName строке, актуально для CDC поделок. Позволяет получать как уведомление о подключении и отключении нужного устройства, так и номер COM порта.

Актуально для разнообразных поделок с USB CDC, не нужно заморачиваться с своей программе интерфейсом для выбора COM порта, достаточно просто индикатора, подключено отключено. И конечному пользователю так же проще - воткнул, все определилось, не надо лезти в диспетчер устройств.

# Описание

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
