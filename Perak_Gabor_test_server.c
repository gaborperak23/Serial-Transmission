/*!
 * @file test_task.c
 *
 * @brief This file includes the actual implementation for the test_task.
 *
 * @version 01.00.00
 */

// #include "test_task.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

/*
 * @covers: DD-1
 * @brief: This DD element intiailize a struct with the required data structure
*/
typedef struct {
    uint8_t type;
    int8_t rssi;
    uint16_t length;
    int8_t* wifi_data;
    uint32_t timestamp;
} WifiPacket;


/*
 * @covers: DD-2
 * @brief: This function generates random values for the struct.
*/

WifiPacket gen_rand_packet() {
    WifiPacket packet;
    packet.type = rand() % 256;
    packet.rssi = (rand() % 101) - 100;
    packet.length = (rand() % 10) + 1;

    packet.wifi_data = (int8_t*)malloc(packet.length);
    for (uint16_t i = 0; i < packet.length; i++) {
        packet.wifi_data[i] = rand() % 256;
    }

    packet.timestamp = (uint32_t)time(NULL);
    return packet;
}


/*
 * @covers: DD-3
 * @brief: This function creates cyclic redundancy check.
*/

uint8_t calc_checksum(const uint8_t* data, uint16_t length) {
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    return checksum;
}


/*
 * @covers: DD-4
 * @brief: This function serializes the WifiPacket, allocates memory
*/

// DD-4.1 - Serialize packets
uint8_t* serialize_wifi_packet(const WifiPacket* packet, uint16_t* payload_size) {
    if (!packet || !payload_size) {
        return NULL;
    }

// DD-4.2 - Calculate payload size
    *payload_size = sizeof(packet->type) +
                    sizeof(packet->rssi) +
                    sizeof(packet->length) +
                    packet->length +
                    sizeof(packet->timestamp) +
                    sizeof(uint8_t);

//DD-4.3 - Allocate memory for the data
    uint8_t* payload = (uint8_t*)malloc(*payload_size);
    if (!payload) {
        return NULL;
    }

//DD-4.4 - Serialize each parameter into the buffer
    uint16_t offset = 0;
    memcpy(payload + offset, &packet->type, sizeof(packet->type));
    offset += sizeof(packet->type);
    memcpy(payload + offset, &packet->rssi, sizeof(packet->rssi));
    offset += sizeof(packet->rssi);
    memcpy(payload + offset, &packet->length, sizeof(packet->length));
    offset += sizeof(packet->length);

    if (packet->length > 0 && packet->wifi_data != NULL) {
        memcpy(payload + offset, packet->wifi_data, packet->length);
        offset += packet->length;
    }

    memcpy(payload + offset, &packet->timestamp, sizeof(packet->timestamp));
    offset += sizeof(packet->timestamp);


/*
 * @covers: DD-3
 * @brief: This function sets checksum
*/

    uint8_t checksum = calc_checksum(payload, offset);
    memcpy(payload + offset, &checksum, sizeof(uint8_t));

    return payload;
}

/*
 * @covers: DD-5
 * @brief: This function checks UART availability, write bytes and close connection
*/

// DD-5.1 - Write to UART
void uart_write_bytes(const char* buffer, size_t buffer_size) {
    int fd = open("/dev/serial0", O_WRONLY | O_NOCTTY | O_NDELAY);

// DD-5.2 - Error handling: check UART availability
    if (fd == -1) {
        perror("Can't reach UART");
        return;
    }

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B9600); //set Baud rate to 9600
    cfsetospeed(&options, B9600); //set Baud rate to 9600
    options.c_cflag |= (CLOCAL | CREAD);
    tcsetattr(fd, TCSANOW, &options);

// DD-5.3 - Write to UART and close connection
    ssize_t bytes_written = write(fd, buffer, buffer_size);
    if (bytes_written < 0) {
        perror("Error writing to UART");
    }

    close(fd);
}


    // Baud ráta beállítása


    // Írás a soros portra
    ssize_t bytes_written = write(fd, buffer, buffer_size);
    if (bytes_written < 0) {
        perror("Error writing to UART");
    }

    close(fd);


/*
 * @covers: DD-6
 * @brief: This function sends packets and gives a feedback of the sent payload
*/

void send_uart(const WifiPacket* packet) {
//serialization
    uint16_t payload_size;
    uint8_t* payload = serialize_wifi_packet(packet, &payload_size);

// send packets if there is payload
    if (payload) {
        uart_write_bytes((const char*)payload, payload_size);
        printf("Sent %d bytes over UART\n", payload_size);

// Debug: print out payload in hexadecimal format
        printf("Payload: ");
        for (int i = 0; i < payload_size; i++) {
            printf("%02X ", payload[i]);
        }
        printf("\n");

//free memory
        free(payload);
    }
}


/*
 * @covers: DD-7
 * @brief: This function free wifi packet
*/

void free_wifipacket(WifiPacket* packet) {
    if (packet->wifi_data) {
        free(packet->wifi_data);
    }
}

int main() {
    srand(time(NULL));

    while (1) {
        WifiPacket packet = gen_rand_packet();
        printf("Generated Packet - Type: %d, RSSI: %d, Length: %d, Timestamp: %u\n",
               packet.type, packet.rssi, packet.length, packet.timestamp);

        send_uart(&packet);
        free_wifipacket(&packet);

        sleep(1); // refresh window: 1sec
    }

    return 0;
}