#ifndef LIB_WIFI_STORAGE_H_
#define LIB_WIFI_STORAGE_H_

#include <stdbool.h>
#include <stdint.h>

#define WIFI_CONFIG_FILE "/lfs1/wifi_config.json"

struct wifi_config {
  char ssid[64];
  char password[64];
};

const struct wifi_config *get_config();

#endif // LIB_WIFI_STORAGE_H_
