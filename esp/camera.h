#pragma once

#include <stdint.h>
#include <stddef.h>

void camera_config();
int camera_init();
int camera(uint8_t * &buffer, size_t &size);
void camera_free();