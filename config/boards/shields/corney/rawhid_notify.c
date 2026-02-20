#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/hid.h>
#include <string.h>

#define REPORT_SIZE 32

static uint8_t report_buf[REPORT_SIZE];

void rawhid_send_layer(uint8_t layer, uint8_t flags)
{
    report_buf[0] = 0x01;  // message type: layer update
    report_buf[1] = layer;
    report_buf[2] = flags;

    // остальные байты нули
    memset(report_buf + 3, 0, REPORT_SIZE - 3);

    hid_int_ep_write(report_buf, REPORT_SIZE, NULL);
}