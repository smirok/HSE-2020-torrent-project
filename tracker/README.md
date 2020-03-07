# HSE-2020-torrent-project
Серверная часть проекта.
Сервер сервер позволяет пирам(пользователям) находить друг-друга, для передачи запрашиваемых данных.  
Он даёт клиенту список ip адресов, которые раздают файл.  

Но он еще не готов, так что пользуемся этим:  
https://github.com/webtorrent/bittorrent-tracker  
  
$ sudo apt install nodejs  
$ sudo apt install npm  
$ npm install -g bittorrent-tracker  
    
Запуск:  
$ bittorrent-tracker
