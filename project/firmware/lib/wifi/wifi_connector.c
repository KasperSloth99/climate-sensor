#include <lib/wifi_connector.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/wifi_mgmt.h>

LOG_MODULE_REGISTER(WIFI_CONNECTOR);
#define NET_EVENT_WIFI_MASK                                                    \
  (NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT)

#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"
#define WIFI_SSID "Aggvold1b"
#define WIFI_PSK "AggVold-1b"
#define WIFI_KEY_MGMT 1

static struct net_mgmt_event_callback cb;

int try_connect_to_wifi() {
  struct net_if *sta_iface = net_if_get_default();
  if (!sta_iface) {
    LOG_INF("STA: interface not initialized");
    return -EIO;
  }
  struct wifi_connect_req_params sta_config;
  sta_config.ssid = (const uint8_t *)WIFI_SSID;
  sta_config.ssid_length = strlen(WIFI_SSID);
  sta_config.psk = (const uint8_t *)WIFI_PSK;
  sta_config.psk_length = strlen(WIFI_PSK);
  sta_config.security = WIFI_SECURITY_TYPE_PSK;
  sta_config.channel = WIFI_CHANNEL_ANY;
  sta_config.band = WIFI_FREQ_BAND_2_4_GHZ;

  LOG_INF("Connecting to SSID: %s\n", sta_config.ssid);

  int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, sta_iface, &sta_config,
                     sizeof(struct wifi_connect_req_params));
  if (ret) {
    LOG_ERR("Unable to Connect to (%s)", WIFI_SSID);
  }

  return ret;
}

static void wifi_event_handler(struct net_mgmt_event_callback *cb,
                               uint64_t mgmt_event, struct net_if *iface) {
  switch (mgmt_event) {
  case NET_EVENT_WIFI_CONNECT_RESULT: {
    LOG_INF("Connected to %s", WIFI_SSID);
    break;
  }
  case NET_EVENT_WIFI_DISCONNECT_RESULT: {
    LOG_INF("Disconnected from %s", WIFI_SSID);
    break;
  }
  default:
    break;
  }
}

int connect_init(void) {
  net_mgmt_init_event_callback(&cb, wifi_event_handler, NET_EVENT_WIFI_MASK);
  net_mgmt_add_event_callback(&cb);

  try_connect_to_wifi();

  return 0;
}

SYS_INIT(connect_init, APPLICATION, 0);