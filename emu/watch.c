#include "watch.h"
#include <stdlib.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////////////////
// Initialization

void _watch_init() {
}
//////////////////////////////////////////////////////////////////////////////////////////
// Segmented Display

static const uint8_t Character_Set[] =
{
    0b00000000, //  
    0b00000000, // !
    0b00100010, // "
    0b01100011, // # (degree symbol, hash mark doesn't fit)
    0b00000000, // $
    0b00000000, // %
    0b01000100, // &
    0b00100000, // '
    0b00000000, // (
    0b00000000, // )
    0b00000000, // *
    0b11000000, // +
    0b00000100, // ,
    0b01000000, // -
    0b01000000, // .
    0b00010010, // /
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b00000000, // :
    0b00000000, // ;
    0b01011000, // <
    0b01001000, // =
    0b01001100, // >
    0b01010011, // ?
    0b11111111, // @
    0b01110111, // A
    0b01111111, // B
    0b00111001, // C
    0b00111111, // D
    0b01111001, // E
    0b01110001, // F
    0b00111101, // G
    0b01110110, // H
    0b10001001, // I
    0b00001110, // J
    0b11101010, // K
    0b00111000, // L
    0b10110111, // M
    0b00110111, // N
    0b00111111, // O
    0b01110011, // P
    0b01100111, // Q
    0b11110111, // R
    0b01101101, // S
    0b10000001, // T
    0b00111110, // U
    0b00111110, // V
    0b10111110, // W
    0b01111110, // X
    0b01101110, // Y
    0b00011011, // Z
    0b00111001, // [
    0b00100100, // backslash
    0b00001111, // ]
    0b00100110, // ^
    0b00001000, // _
    0b00000010, // `
    0b01011111, // a
    0b01111100, // b
    0b01011000, // c
    0b01011110, // d
    0b01111011, // e
    0b01110001, // f
    0b01101111, // g
    0b01110100, // h
    0b00010000, // i
    0b01000010, // j
    0b11101010, // k
    0b00110000, // l
    0b10110111, // m
    0b01010100, // n
    0b01011100, // o
    0b01110011, // p
    0b01100111, // q
    0b01010000, // r
    0b01101101, // s
    0b01111000, // t
    0b01100010, // u
    0b01100010, // v
    0b10111110, // w
    0b01111110, // x
    0b01101110, // y
    0b00011011, // z
    0b00111001, // {
    0b00110000, // |
    0b00001111, // }
    0b00000001, // ~
};

static const uint64_t Segment_Map[] = {
    0x4e4f0e8e8f8d4d0d, // Position 8
    0xc8c4c4c8b4b4b0b,  // Position 9
    0xc049c00a49890949, // Position 6
    0xc048088886874707, // Position 7
    0xc053921252139352, // Position 0
    0xc054511415559594, // Position 1
    0xc057965616179716, // Position 2
    0xc041804000018a81, // Position 3
    0xc043420203048382, // Position 4
    0xc045440506468584, // Position 5
};

static const uint8_t Num_Chars = 10;

void watch_enable_display() {
}

void watch_set_pixel(uint8_t com, uint8_t seg) {
    printf("set (%d, %d)\n", com, seg);
}

void watch_clear_pixel(uint8_t com, uint8_t seg) {
}

void watch_display_character(uint8_t character, uint8_t position) {
    uint64_t segmap = Segment_Map[position];
    uint64_t segdata = Character_Set[character - 0x20];

    for (int i = 0; i < 8; i++) {
        uint8_t com = (segmap & 0xFF) >> 6;
        if (com > 2) {
            // COM3 means no segment exists; skip it.
            segmap = segmap >> 8;
            segdata = segdata >> 1;
            continue;
        }
        uint8_t seg = segmap & 0x3F;
        // slcd_sync_seg_off(&SEGMENT_LCD_0, SLCD_SEGID(com, seg));
        // if (segdata & 1) slcd_sync_seg_on(&SEGMENT_LCD_0, SLCD_SEGID(com, seg));
        if (segdata & 1)
          watch_set_pixel(com, seg);
        else
          watch_clear_pixel(com, seg);
        segmap = segmap >> 8;
        segdata = segdata >> 1;
    }
}

void watch_display_string(char *string, uint8_t position) {
    printf("watch_display_string(\"%s\", %d)\n", string, position);
    size_t i = 0;
    while(string[i] != 0) {
        watch_display_character(string[i], position + i);
        i++;
        if (i >= Num_Chars) break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Buttons

void watch_enable_buttons() {
}

void watch_register_button_callback(const uint32_t pin, ext_irq_cb_t callback) {
}

//////////////////////////////////////////////////////////////////////////////////////////
// LED

bool PWM_0_enabled = false;

void watch_enable_led(bool pwm) {
}

void watch_disable_led(bool pwm) {
}

void watch_set_led_color(uint16_t red, uint16_t green) {
}

void watch_set_led_red() {
}

void watch_set_led_green() {
}

void watch_set_led_yellow() {
}

void watch_set_led_off() {
}

//////////////////////////////////////////////////////////////////////////////////////////
// Real-time Clock

bool watch_rtc_is_enabled() {
    return 0;
}

void watch_set_date_time(struct calendar_date_time date_time) {
}

void watch_get_date_time(struct calendar_date_time *date_time) {
}

static ext_irq_cb_t tick_user_callback;

static void tick_callback(struct calendar_dev *const dev) {
    tick_user_callback();
}

void watch_register_tick_callback(ext_irq_cb_t callback) {
    tick_user_callback = callback;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Analog Input

static bool ADC_0_ENABLED = false;

void watch_enable_analog(const uint8_t pin) {
}

//////////////////////////////////////////////////////////////////////////////////////////
// Digital IO

void watch_enable_digital_input(const uint8_t pin) {
}

void watch_enable_pull_up(const uint8_t pin) {
}

void watch_enable_pull_down(const uint8_t pin) {
}

bool watch_get_pin_level(const uint8_t pin) {
}

void watch_enable_digital_output(const uint8_t pin) {
}

void watch_disable_digital_output(const uint8_t pin) {
}

void watch_set_pin_level(const uint8_t pin, const bool level) {
}

//////////////////////////////////////////////////////////////////////////////////////////
// I2C

struct io_descriptor *I2C_0_io;

void watch_enable_i2c() {
}

void watch_i2c_send(int16_t addr, uint8_t *buf, uint16_t length) {
}

void watch_i2c_receive(int16_t addr, uint8_t *buf, uint16_t length) {
}

void watch_i2c_write8(int16_t addr, uint8_t reg, uint8_t data) {
}

uint8_t watch_i2c_read8(int16_t addr, uint8_t reg) {
    return 0;
}

uint16_t watch_i2c_read16(int16_t addr, uint8_t reg) {
    return 0;
}

uint32_t watch_i2c_read24(int16_t addr, uint8_t reg) {
    return 0;
}

uint32_t watch_i2c_read32(int16_t addr, uint8_t reg) {
}

//////////////////////////////////////////////////////////////////////////////////////////
// Deep Sleep

void watch_store_backup_data(uint32_t data, uint8_t reg) {
}

uint32_t watch_get_backup_data(uint8_t reg) {
    return 0;
}

static void extwake_callback(struct calendar_dev *const dev) {
    // this will never get called since we are basically waking from reset
}

void watch_enter_deep_sleep() {
}

void delay_ms(int n) {
}

int main(int argc, char *argv[]) {
    printf("Started\n");
    app_setup();
    app_loop();
    printf("Stopped\n");
    return 0;
}
