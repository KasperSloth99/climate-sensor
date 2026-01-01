#include "lib/wifi/keys.h"
#include <stdlib.h>
#include <string.h>
#include <zephyr/shell/shell.h>

static int cmd_keys_save_config(const struct shell *sh, size_t argc,
                                char **argv) {
  struct wifi_config config = {};
  if (argc < 3) {
    shell_error(sh, "Usage: wifi set <ssid> <password> [default]");
    return -EINVAL;
  }

  config.ssid = argv[1];
  config.password = argv[2];

  return save_config(&config);
}

/* Register shell commands */
SHELL_STATIC_SUBCMD_SET_CREATE(
    my_wifi_cmds,
    SHELL_CMD_ARG(save, NULL,
                  "Save WiFi config: set <ssid> <password> [default]",
                  cmd_keys_save_config, 3, 1),
    SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(my_wifi, &my_wifi_cmds, "MyWiFi configuration commands",
                   NULL);
