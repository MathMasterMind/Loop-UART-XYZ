#ifndef XYZ_H
#define XYZ_H

#include <stdint.h>
#include <math.h>
#include <stdlib.h>

typedef struct {
    uint32_t xyz0_count;
    uint32_t xyz1_count;
    uint32_t xyz2_count;
    uint32_t codeword;
} XYZ_Data;

extern XYZ_Data g_xyz_data;
uint8_t *buffer;
uint8_t length;
uint8_t seed;
uint8_t valid;
uint8_t idx;
uint8_t type;

void init_xyz (void);
void process_xyz (uint8_t byte);
uint8_t CRC_calc (uint8_t* payload, uint8_t length);
uint8_t add_xyz0 ();
uint8_t add_xyz1 (float value, uint8_t op, uint8_t iter);
uint8_t add_xyz2 (uint32_t value, uint8_t op, uint8_t iter);

#endif