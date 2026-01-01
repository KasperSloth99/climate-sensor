zephyr_library()
zephyr_library_sources(keys.c)
zephyr_library_sources_ifdef(CONFIG_MY_WIFI_KEYS_SHELL keys_shell.c)
