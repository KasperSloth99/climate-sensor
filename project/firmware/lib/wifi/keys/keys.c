#include "lib/wifi/keys.h"
#include <zephyr/data/json.h>
#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(keys, LOG_LEVEL_INF);

static const struct json_obj_descr wifi_config_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct wifi_config, ssid, JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(struct wifi_config, password, JSON_TOK_STRING),
};

int save_config(struct wifi_config *config) {
  uint8_t buf[0x100];
  struct fs_file_t fd;
  int ret =
      json_obj_encode_buf(wifi_config_descr, ARRAY_SIZE(wifi_config_descr),
                          config, buf, sizeof(buf));
  if (ret < 0) {
    LOG_ERR("JSON encode failed (%d)", ret);
    return ret;
  }
  fs_file_t_init(&fd);
  ret = fs_open(&fd, WIFI_CONFIG_FILE, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
  if (ret < 0) {
    return ret;
  }

  ret = fs_write(&fd, buf, strlen(buf));
  if (ret) {
    LOG_INF("Wrote %d bytes to %s", ret, WIFI_CONFIG_FILE);
  }

  fs_close(&fd);
  return ret;
}
