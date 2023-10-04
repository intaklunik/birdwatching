#pragma once

#include <stdint.h>
#include <stddef.h>

int network_init();
void connect_to_server(uint8_t *buffer, uint32_t size);