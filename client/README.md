Установка libtorrent : 
1) скачиваем версию 1.2.4 c https://github.com/arvidn/libtorrent/releases 
2) следуем этому гайду по установке библиотеки : https://www.howtogeek.com/105413/how-to-compile-and-install-from-source-on-ubuntu/
3) Если возникнет ошибка поиска .so (по типу : error while loading shared libraries: libtorrent-rasterbar.so.10: cannot open shared object file: No such file or directory) , то воспользуйтесь командой : sudo ldconfig

Запуск программы :
./client
