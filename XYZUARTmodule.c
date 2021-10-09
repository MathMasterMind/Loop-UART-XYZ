#include"XYZUARTmodule.h"
#include <stdio.h>
XYZ_Data g_xyz_data;

// Initializes the XYZ uart module so that all old values will be erased
void init_xyz (void) {
    XYZ_Data g_xyz_data;
    buffer = (uint8_t*)calloc(256, sizeof(uint8_t));
    seed = 0;
    valid = 0;

    g_xyz_data.xyz0_count = 0;
    g_xyz_data.xyz1_count = 0;
    g_xyz_data.xyz2_count = 0;
    g_xyz_data.codeword = 0;
}

/* */
void process_xyz (uint8_t byte) {
    // Searches for a head that contains XYZ!
    // Deletes all characters that came before it
    if (valid == 0) {
        buffer[0] = buffer[1];
        buffer[1] = buffer[2];
        buffer[2] = buffer[3];
        buffer[3] = byte;

        // Enables bytes to be stored after the head
        if (buffer[0] == 'X' 
                && buffer[1] == 'Y' 
                && buffer[2] == 'Z' 
                && buffer[3] == '!') {
            valid = 1;
            idx = 4;
        }
        return;
    }
    
    buffer[idx] = byte;

    // Determines the XYZ protocol type after the L (length) N (type) and CRC (checksum) are stored in the buffer
    if (idx == 6) {
        // If the checksum is correct, the length is 2, and the type corresponds to XYZ0, then add the XYZ0 protocol
        if (CRC_calc (buffer + 4, 3) == 0xff && buffer[4] == 2 && buffer[5] == 0) {
            add_xyz0();
        // If the checksum is not correct but it is type XYZ0, it will be a candidate for XYZ1 and XYZ2 and fail for those
        } else {
            type = buffer[6];
            length = buffer[3 + type];
        }

        // Checks for types that are 0, 1, and 2
        // If not, it will search for a new head
        if (type > 2) {
            valid = 0;
            return;
        }

        // Checks if the length corresponds to the correct type
        // If not, it will search for a new head
        if (length + type != 9) {
            valid = 0;
            return;
        }
    }

    // Once the payload is finished, the XYZ1 and XYZ2 protocols can be added
    if (idx == 12) {
        // If the checksum is correct the protocol is valid
        if (CRC_calc (buffer + 4, 9) == 0xff) {
            // Adds combines the 4 bytes of the buffer into one variable
            int value = buffer[7] + (buffer[8] << 8) + (buffer[9] << 16) + (buffer[10] << 24);
            // Converts the int to float bitwise
            float *converted = (float*)&value;
            int op = buffer[11];
            int iter = buffer[12];

            if (type == 1)
                add_xyz1(converted, op, iter);
            else
                add_xyz2(value, op, iter);
        } else {
            valid = 0;
        }
    }
    
    idx++;
}

// Calculates the checksum of the bytes
uint8_t CRC_calc (uint8_t* payload, uint8_t length) {
    uint8_t CRC_out = 0;
    for(uint8_t elem = 0; elem < length; elem++){
        CRC_out = CRC_out ^ payload[elem];
    }
    return CRC_out;
}

// Adds the XYZ0 protocol but only the codeword needs to be updated with the new count of XYZ0
uint8_t add_xyz0 () {
    g_xyz_data.xyz0_count++;
    
    // Uses pointers so that a bitwise conversion between IEEE754 float and integer can occur rather than arithmatic conversion
    uint32_t* converted = (uint32_t*)&seed;
    g_xyz_data.codeword = *converted + g_xyz_data.xyz0_count + g_xyz_data.xyz1_count + g_xyz_data.xyz2_count;
    
    // Searches for the new head
    valid = 0;
}


// Adds the XYZ1 protocol according to specifications that modify the seed using floats
uint8_t add_xyz1 (float* value, uint8_t op, uint8_t iter) {
    g_xyz_data.xyz1_count++;

    // Applies an operation iter times
    for (uint8_t i = 0; i < iter; i++) {
        // Determines the operation to use for the seed
        if (op == 0)
            seed = ceil(seed + *value);
        if (op == 1)
            seed = ceil(seed - *value);
        if (op == 2)
            seed = ceil(seed * *value);
        if (op == 3)
            seed = ceil(seed / *value);
    }

    // Uses pointers so that a bitwise conversion between IEEE754 float and integer can occur rather than arithmatic conversion
    uint32_t* converted = (uint32_t*)&seed;
    g_xyz_data.codeword = *converted + g_xyz_data.xyz0_count + g_xyz_data.xyz1_count + g_xyz_data.xyz2_count;
    
    // Searches for the new head
    valid = 0;
}

// Adds the XYZ2 protocol according to specifications that modify the seed using signed integers
uint8_t add_xyz2 (int32_t value, uint8_t op, uint8_t iter) {
    g_xyz_data.xyz2_count++;

    // Applies an operation iter times
    for (uint8_t i = 0; i < iter; i++) {
        // Determines the operation to use for the seed
        if (op == 0)
            seed = ceil(seed + value);
        if (op == 1)
            seed = ceil(seed - value);
        if (op == 2)
            seed = ceil(seed * value);
        if (op == 3)
            seed = ceil(seed / value);
    }
    
    // Uses pointers so that a bitwise conversion between IEEE754 float and integer can occur rather than arithmatic conversion
    uint32_t* converted = (uint32_t*)&seed;
    g_xyz_data.codeword = *converted + g_xyz_data.xyz0_count + g_xyz_data.xyz1_count + g_xyz_data.xyz2_count;
    
    // Searches for the new head
    valid = 0;
}

/*
int main()
{
    init_xyz ();
    
    printf("0: %#08x\n", g_xyz_data.codeword);
    
    // XYZ0
    process_xyz ('X');
    process_xyz ('Y');
    process_xyz ('Z');
    process_xyz ('!');
    process_xyz (2);
    process_xyz (0);
    process_xyz (253);
    
    printf("1: %#08x\n", g_xyz_data.codeword);
    
    // XYZ1(FLOAT=0.75f, OP=0, ITER=12)
    process_xyz ('X');
    process_xyz ('Y');
    process_xyz ('Z');
    process_xyz ('!');
    process_xyz (8);
    process_xyz (133);
    process_xyz (1);
    process_xyz (0x00); //0x3f400000 = 0.75f
    process_xyz (0x00);
    process_xyz (0x40);
    process_xyz (0x3f);
    process_xyz (0);
    process_xyz (12);
    
    printf("2: %#08x\n", g_xyz_data.codeword);
    
    // XYZ2(INT=-3, OP=3, ITER=2)
    process_xyz ('X');
    process_xyz ('Y');
    process_xyz ('Z');
    process_xyz ('!');
    process_xyz (249);
    process_xyz (7);
    process_xyz (2);
    process_xyz (0xfd);
    process_xyz (0xff);
    process_xyz (0xff);
    process_xyz (0xff);
    process_xyz (3);
    process_xyz (2);
    
    printf("3: %#08x\n", g_xyz_data.codeword);
    
    return 0;
}
*/