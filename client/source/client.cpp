#include "client.h"
#include <string>
void client::init() {
    lt::settings_pack pack; // пакет выводимых команд для уведомлений о закаче
    pack.set_int(lt::settings_pack::alert_mask, lt::alert::error_notification
                                                | lt::alert::storage_notification
                                                | lt::alert::status_notification);

    s.apply_settings(pack); //добавляем пакет в сессию
}

client::client() {
    init();
}

void client::set_path(const std::string& path) {
    p.save_path = path;
}

void client::set_file(const std::string& file_name){
    p.ti = std::make_shared<lt::torrent_info>(file_name);
}


