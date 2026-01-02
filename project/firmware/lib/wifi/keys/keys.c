#include "lib/wifi/keys.h"
#include <stdlib.h>
#include <string.h>
#include <zephyr/data/json.h>
#include <zephyr/fs/fs.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(keys, LOG_LEVEL_INF);

static struct wifi_config _wifi_config;

static const struct json_obj_descr wifi_config_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct wifi_config, ssid, JSON_TOK_STRING_BUF),
    JSON_OBJ_DESCR_PRIM(struct wifi_config, password, JSON_TOK_STRING_BUF),
};

const struct wifi_config *get_config() { return &_wifi_config; }

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

  memcpy(&_wifi_config, config, sizeof(_wifi_config));
  return ret;
}

static int load_config() {
  char buf[sizeof(_wifi_config)];
  struct fs_file_t fd;
  ssize_t bytes_read;
  int ret;

  fs_file_t_init(&fd);
  ret = fs_open(&fd, "/lfs1/wifi_config.json", FS_O_READ);
  if (ret < 0) {
    LOG_ERR("File open failed (%d)", ret);
    return ret;
  }

  bytes_read = fs_read(&fd, buf, sizeof(buf) - 1);
  fs_close(&fd);

  if (bytes_read < 0) {
    LOG_ERR("File read failed (%d)", bytes_read);
    return bytes_read;
  }

  LOG_INF("Read %d bytes from file", bytes_read);

  ret = json_obj_parse(buf, bytes_read, wifi_config_descr,
                       ARRAY_SIZE(wifi_config_descr), &_wifi_config);
  if (ret < 0) {
    LOG_ERR("JSON parse failed (%d)", ret);
    return ret;
  }

  LOG_INF("Loaded config - SSID: %s, Password: %s", _wifi_config.ssid,
          _wifi_config.password);
  return 0;
}

// This needs to happen after FILE_SYSTEM_INIT_PRIORITY which is
// POST_KERNEL 99, so just do it at application init prio
SYS_INIT(load_config, APPLICATION, 0);

#ifdef CONFIG_MY_WIFI_KEYS_SHELL
static int cmd_keys_save_config(const struct shell *sh, size_t argc,
                                char **argv) {
  struct wifi_config config;
  if (argc < 3) {
    shell_error(sh, "Usage: wifi set <ssid> <password>");
    return -EINVAL;
  }

  strcpy(config.ssid, argv[1]);
  strcpy(config.password, argv[2]);

  return save_config(&config);
}

static int cmd_keys_get_config(const struct shell *sh, size_t argc,
                               char **argv) {
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);
  shell_print(sh, "SSID: %s Password: %s", _wifi_config.ssid,
              _wifi_config.password);
  return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
    my_wifi_cmds,
    SHELL_CMD_ARG(save, NULL, "Save WiFi config: save <ssid> <password>",
                  cmd_keys_save_config, 3, 0),
    SHELL_CMD(get, NULL, "Get current wifi config", cmd_keys_get_config),
    SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(my_wifi, &my_wifi_cmds, "MyWiFi configuration commands",
                   NULL);
#endif
