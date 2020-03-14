# HSE-2020-torrent-project
Серверная часть проекта.
Сервер сервер позволяет пирам(пользователям) находить друг-друга, для передачи запрашиваемых данных.
Он даёт клиенту список ip адресов, которые раздают файл.  

Hand-made udp tracker:  
В директрии, tracker/server (где src и include) делаем  
$ cmake .  
$ make  
$ ./server  
  
Наш почти готов, но можно пользоваться этим:  
https://github.com/webtorrent/bittorrent-tracker  
  
$ sudo apt install nodejs  
$ sudo apt install npm  
$ npm install -g bittorrent-tracker  
    
Запуск:  
$ bittorrent-tracker
