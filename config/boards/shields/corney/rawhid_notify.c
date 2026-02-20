#include <zephyr/kernel.h>
#include <stdint.h>
#include <string.h>

#if IS_ENABLED(CONFIG_RAW_HID)
#include <raw_hid/events.h>
#endif

#if IS_ENABLED(CONFIG_RAW_HID)
static uint8_t report_buf[CONFIG_RAW_HID_REPORT_SIZE];
#else
static uint8_t report_buf[32];
#endif

void rawhid_send_layer(uint8_t layer, uint8_t flags)
{
    report_buf[0] = 0x01;  // message type: layer update
    report_buf[1] = layer;
    report_buf[2] = flags;

    // остальные байты нули
    memset(report_buf + 3, 0, sizeof(report_buf) - 3);

#if IS_ENABLED(CONFIG_RAW_HID)
    raise_raw_hid_sent_event(
        (struct raw_hid_sent_event){.data = report_buf, .length = sizeof(report_buf)});
#endif
}
