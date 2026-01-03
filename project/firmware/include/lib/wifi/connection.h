#ifndef PROJECT_LIB_WIFI_CONNECTION_H_
#define PROJECT_LIB_WIFI_CONNECTION_H_

#include <stdbool.h>
#include <zephyr/sys/clock.h>

bool wifi_connected(k_timeout_t timeout);
bool has_ip(k_timeout_t timeout);

#endif // PROJECT_LIB_WIFI_CONNECTION_H_
