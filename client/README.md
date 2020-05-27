## Клиент

### Введение

Клиент является составной частью приложения ```TorrentX```.
Для работы клиента требуется библиотеки ```libtorrent-rasterbar``` и ```boost```.

### Установка libtorrent 

* Подготовка к скачиванию , вводим команды : 
   ```
   sudo apt-get update
	
   sudo apt-get install build-essential
   ```
	
* Cкачиваем версию 1.2.4 https://github.com/arvidn/libtorrent/releases/tag/libtorrent-1_2_4 или новее : https://github.com/arvidn/libtorrent/releases 

* Распаковываем архив, переходим в распакованную папку.

* последовательно выполняем команды : 
   ```
   ./configure.sh
   
   make
   
   sudo make install
   ```
### Установка boost

    sudo apt-get install libboost-all-dev
    
### Возможные ошибки 

* Если возникнет ошибка поиска файлов ```.so``` (по типу : ```error while loading shared libraries: libtorrent-rasterbar.so.10: cannot open shared object file: No such file or directory```) , то воспользуйтесь командой 

    ```
    sudo ldconfig
    ```
