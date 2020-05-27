### Введение

Минималистичный торрент-клиент, который позволяет скачивать и раздавать торрент файлы

### Установка TorrentX

* Для начала нужно установить всё то, что нужно для работы с [libtorrent](https://github.com/smirok/HSE-2020-torrent-project/tree/master/client)
* Чтобы запустить gui, вам нужно установить Qt на свой компьютер 
    ```
    sudo apt install qt5-default
    ```
* Сборка программы
    ```
    qmake torrent_gui.pro && make -j7
    ```
* Запуск программы 
    ``` 
    ./torrent_gui
    ```
