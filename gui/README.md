1. Для начала нужно установить всё то, что нужно для работы с libtorrent(вы найдёте инструкцию выше в папке client).
2. Чтобы запустить gui, вам нужно установить Qt на свой компьютер -- sudo apt install qt5-default.
3. Сборка программы -- qmake qmake torrent_gui.pro
                       make -j7
4. Запуск программы -- ./torrent_gui
