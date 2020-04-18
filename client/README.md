Клиент является составной частью приложения и не запускается отдельно от него.
Для работы клиента требуется библиотеки libtorrent-rasterbar и boost. Про необходимую версию boost-а прочитайте в README сервера.

Установка libtorrent : 
0) подготовка к скачиванию , вводим команды : 
	0.1) sudo apt-get update
	0.2) sudo apt-get install build-essential
1) скачиваем версию 1.2.4 (https://github.com/arvidn/libtorrent/releases/tag/libtorrent-1_2_4) или новее : https://github.com/arvidn/libtorrent/releases 
2) распаковываем архив, переходим в распакованную папку.
3) последовательно выполняем команды : 
   3.1) ./configure.sh
   3.2) make
   3.3) sudo make install

Возможные ошибки : 
1) Если возникнет ошибка поиска файлов .so (по типу : error while loading shared libraries: libtorrent-rasterbar.so.10: cannot open shared object file: No such file or directory) , то воспользуйтесь командой : sudo ldconfig
