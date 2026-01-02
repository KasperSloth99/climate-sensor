#include "lib/wifi/connection.h"
#include "lib/wifi/keys.h"
#include <errno.h>
#include <stdlib.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(wifi_app, LOG_LEVEL_INF);

#define CONNECTION_RETRIES_AT_BOOT 5

struct net_management {
  struct k_sem semaphore;
  struct net_mgmt_event_callback cb;
};

struct my_wifi_connection {
  struct net_management wifi;
  struct net_management net;
  struct wifi_connect_req_params params;
  struct net_if *iface;
};

struct my_wifi_connection _wifi_connection;

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                    uint64_t mgmt_event, struct net_if *iface) {
  struct net_management *mgmt = CONTAINER_OF(cb, struct net_management, cb);
  switch (mgmt_event) {
  case NET_EVENT_WIFI_CONNECT_RESULT:
    LOG_INF("WiFi connected");
    k_sem_give(&mgmt->semaphore);
    break;
  case NET_EVENT_WIFI_DISCONNECT_RESULT:
    LOG_INF("WiFi disconnected");
    k_sem_reset(&mgmt->semaphore);
    break;
  default:
    break;
  }
}

static void check_config() {
  const struct wifi_config *config = get_config();
  _wifi_connection.params.ssid = config->ssid;
  _wifi_connection.params.ssid_length = strlen(config->ssid);
  _wifi_connection.params.psk = config->password;
  _wifi_connection.params.psk_length = strlen(config->password);
  _wifi_connection.params.channel = WIFI_CHANNEL_ANY;
  _wifi_connection.params.security = (_wifi_connection.params.psk_length > 0)
                                         ? WIFI_SECURITY_TYPE_PSK
                                         : WIFI_SECURITY_TYPE_NONE;
  _wifi_connection.params.band = WIFI_FREQ_BAND_2_4_GHZ;
  _wifi_connection.params.mfp = WIFI_MFP_OPTIONAL;
}

static void net_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                   uint64_t mgmt_event, struct net_if *iface) {
  struct net_management *mgmt = CONTAINER_OF(cb, struct net_management, cb);

  switch (mgmt_event) {
  case NET_EVENT_IPV4_ADDR_ADD:
    LOG_INF("IPv4 address obtained");
    k_sem_give(&mgmt->semaphore);
    break;
  default:
    break;
  }
}

static int wifi_connect(void) {
  LOG_INF("Connecting to SSID: %s", _wifi_connection.params.ssid);

  if (net_mgmt(NET_REQUEST_WIFI_CONNECT, _wifi_connection.iface,
               &_wifi_connection.params,
               sizeof(struct wifi_connect_req_params))) {
    LOG_ERR("WiFi connection request failed");
    return -1;
  }

  if (k_sem_take(&_wifi_connection.wifi.semaphore, K_SECONDS(30)) != 0) {
    LOG_ERR("WiFi connection timeout");
    return -1;
  }

  LOG_INF("Successfully connected to WiFi, starting DHCP");
  net_dhcpv4_start(_wifi_connection.iface);
  return 0;
}

static int wait_for_ip() {
  LOG_INF("Waiting for IP");
  if (k_sem_take(&_wifi_connection.net.semaphore, K_SECONDS(30)) != 0) {
    LOG_ERR("DHCP timeout");
    return -1;
  }
  LOG_INF("Obtained IP");
  return 0;
}

static int try_connect_wifi(uint32_t retries) {
  check_config();
  while (retries) {
    if (wifi_connect() == 0) {
      return 0;
    }
    retries--;
    LOG_ERR("Failed to connect to WiFi, retrying %d more times", retries);
    k_sleep(K_SECONDS(5));
  }
  return -ECONNREFUSED;
}

static int try_wait_for_ip(uint32_t retries) {
  while (retries) {
    if (wait_for_ip() == 0) {
      return 0;
    }
    retries--;
    LOG_ERR("Didn't obtain an IP, retrying %d more times", retries);
  }
  return -ECONNREFUSED;
}

int init_wifi(void) {
  LOG_INF("WiFi initializing");

  k_sem_init(&_wifi_connection.wifi.semaphore, 0, 1);
  k_sem_init(&_wifi_connection.net.semaphore, 0, 1);

  _wifi_connection.iface = net_if_get_default();

  net_mgmt_init_event_callback(
      &_wifi_connection.wifi.cb, wifi_mgmt_event_handler,
      NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);
  net_mgmt_add_event_callback(&_wifi_connection.wifi.cb);

  net_mgmt_init_event_callback(&_wifi_connection.net.cb, net_mgmt_event_handler,
                               NET_EVENT_IPV4_ADDR_ADD);
  net_mgmt_add_event_callback(&_wifi_connection.net.cb);
  int ret = try_connect_wifi(CONNECTION_RETRIES_AT_BOOT);
  if (ret < 0) {
    return ret;
  }
  return try_wait_for_ip(CONNECTION_RETRIES_AT_BOOT);
}

SYS_INIT(init_wifi, APPLICATION, 99);

#ifdef CONFIG_MY_WIFI_CONNECTION_SHELL
static int cmd_connection_try_wifi(const struct shell *sh, size_t argc,
                                   char **argv) {
  uint32_t retries = 1;
  if (argc > 1) {
    retries = strtoul(argv[1], NULL, 10);
  }
  shell_print(sh, "Trying to connect with %d retrie(s)", retries);
  return try_connect_wifi(retries);
}

SHELL_STATIC_SUBCMD_SET_CREATE(my_connection_cmds,
                               SHELL_CMD_ARG(connect, NULL,
                                             "Try connecting to wifi",
                                             cmd_connection_try_wifi, 1, 1),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(my_connection, &my_connection_cmds, "MyConnection commands",
                   NULL);
#endif
