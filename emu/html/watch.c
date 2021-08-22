/*
 * MIT License
 *
 * Copyright (c) 2021 Joey Castillo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "watch.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <emscripten.h>
#include <assert.h>

#define SLCD_SEGID(com, seg) (((com) << 6) | ((seg) & 0x3f))

void delay_ms(int n) {
  emscripten_sleep(n);
}

//////////////////////////////////////////////////////////////////////////////////////////
// User callbacks and other definitions

ext_irq_cb_t btn_alarm_callback;
ext_irq_cb_t a2_callback;
ext_irq_cb_t d1_callback;

static void extwake_callback(uint8_t reason);


//////////////////////////////////////////////////////////////////////////////////////////
// Initialization

void _watch_init() {
}


//////////////////////////////////////////////////////////////////////////////////////////
// Segmented Display

static const uint8_t Character_Set[] =
{
    0b00000000, //  
    0b00000000, // ! (unused)
    0b00100010, // "
    0b01100011, // # (degree symbol, hash mark doesn't fit)
    0b00000000, // $ (unused)
    0b00000000, // % (unused)
    0b01000100, // & ("lowercase 7" for positions 4 and 6)
    0b00100000, // '
    0b00111001, // (
    0b00001111, // )
    0b00000000, // * (unused)
    0b11000000, // + (only works in position 0)
    0b00000100, // ,
    0b01000000, // -
    0b01000000, // . (same as -, semantically most useful)
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
    0b00000000, // : (unused)
    0b00000000, // ; (unused)
    0b01011000, // <
    0b01001000, // =
    0b01001100, // >
    0b01010011, // ?
    0b11111111, // @ (all segments on)
    0b01110111, // A
    0b01111111, // B
    0b00111001, // C
    0b00111111, // D
    0b01111001, // E
    0b01110001, // F
    0b00111101, // G
    0b01110110, // H
    0b10001001, // I (only works in position 0)
    0b00001110, // J
    0b01110101, // K
    0b00111000, // L
    0b10110111, // M (only works in position 0)
    0b00110111, // N
    0b00111111, // O
    0b01110011, // P
    0b01100111, // Q
    0b11110111, // R (only works in position 1)
    0b01101101, // S
    0b10000001, // T (only works in position 0; set (1, 12) to make it work in position 1)
    0b00111110, // U
    0b00111110, // V
    0b10111110, // W (only works in position 0)
    0b01111110, // X
    0b01101110, // Y
    0b00011011, // Z
    0b00111001, // [
    0b00100100, // backslash
    0b00001111, // ]
    0b00100011, // ^
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
    0b01000010, // j (appears as superscript to work in more positions)
    0b01110101, // k
    0b00110000, // l
    0b10110111, // m (only works in position 0)
    0b01010100, // n
    0b01011100, // o
    0b01110011, // p
    0b01100111, // q
    0b01010000, // r
    0b01101101, // s
    0b01111000, // t
    0b01100010, // u (appears as superscript to work in more positions)
    0b01100010, // v (appears as superscript to work in more positions)
    0b10111110, // w (only works in position 0)
    0b01111110, // x
    0b01101110, // y
    0b00011011, // z
    0b00111001, // {
    0b00110000, // |
    0b00001111, // }
    0b00000001, // ~
};

static const uint64_t Segment_Map[] = {
    0x4e4f0e8e8f8d4d0d, // Position 0, mode
    0xc8c4c4c8b4b4b0b,  // Position 1, mode (Segments B and C shared, as are segments E and F)
    0xc049c00a49890949, // Position 2, day of month (Segments A, D, G shared; missing segment F)
    0xc048088886874707, // Position 3, day of month
    0xc053921252139352, // Position 4, clock hours (Segments A and D shared)
    0xc054511415559594, // Position 5, clock hours
    0xc057965616179716, // Position 6, clock minutes (Segments A and D shared)
    0xc041804000018a81, // Position 7, clock minutes
    0xc043420203048382, // Position 8, clock seconds
    0xc045440506468584, // Position 9, clock seconds
};

static const uint8_t Num_Chars = 10;

static const uint8_t IndicatorSegments[6][2] = {
    {0, 17}, // WATCH_INDICATOR_SIGNAL
    {0, 16}, // WATCH_INDICATOR_BELL
    {2, 17}, // WATCH_INDICATOR_PM
    {2, 16}, // WATCH_INDICATOR_24H
    {1, 10}, // WATCH_INDICATOR_LAP
};

void watch_enable_display() {
  int com, seg;
  for (com = 0; com < 3; com++)
    for (seg = 0; seg < 0x40; seg++)
      watch_clear_pixel(com, seg);
}

static void set_pixel(int com, int seg, int v) {
  // char scr[80];
  // sprintf(scr, "set_pixel(%d, %d, %d)", com, seg, v);
  // emscripten_run_script(scr);
  EM_ASM({
    set_pixel($0, $1, $2);
  }, com, seg, v);
}

inline void watch_set_pixel(uint8_t com, uint8_t seg) {
  set_pixel(com, seg, 1);
}

inline void watch_clear_pixel(uint8_t com, uint8_t seg) {
  set_pixel(com, seg, 0);
}

void watch_display_character(uint8_t character, uint8_t position) {
    // handle lowercase 7 if needed
    if (character == '7' && (position == 4 || position == 6)) character = '&';

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
        if (segdata & 1)
          watch_set_pixel(com, seg);
        else
          watch_clear_pixel(com, seg);
        segmap = segmap >> 8;
        segdata = segdata >> 1;
    }
}

void watch_display_string(char *string, uint8_t position) {
    size_t i = 0;
    while(string[i] != 0) {
        watch_display_character(string[i], position + i);
        i++;
        if (i >= Num_Chars) break;
    }
}

inline void watch_set_colon() {
    watch_set_pixel(1, 16);
}

inline void watch_clear_colon() {
    watch_clear_pixel(1, 16);
}

inline void watch_set_indicator(WatchIndicatorSegment indicator) {
    watch_set_pixel(IndicatorSegments[indicator][0], IndicatorSegments[indicator][1]);
}

inline void watch_clear_indicator(WatchIndicatorSegment indicator) {
    watch_clear_pixel(IndicatorSegments[indicator][0], IndicatorSegments[indicator][1]);
}

void watch_clear_all_indicators() {
    int i;
    for (i = 0; i < 5; i++)
        watch_clear_indicator(i);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Buttons

void watch_enable_buttons() {
}

ext_irq_cb_t cbs[5];

void watch_register_button_callback(const uint8_t pin, ext_irq_cb_t callback) {
    switch (pin) {
      case BTN_LIGHT:
      case BTN_MODE:
      case BTN_ALARM:
        break;
      default:
        assert(0);
    }
    cbs[pin] = callback;
}

void watch_dispatch_callback(uint8_t pin) {
  if (cbs[pin])
    cbs[pin]();
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

static void set_leds(int r, int g) {
  char scr[80];
  sprintf(scr, "set_leds(%d, %d)", r, g);
  emscripten_run_script(scr);
}

void watch_set_led_red() {
  set_leds(1, 0);
}

void watch_set_led_green() {
  set_leds(0, 1);
}

void watch_set_led_yellow() {
  set_leds(1, 1);
}

void watch_set_led_off() {
  set_leds(0, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Buzzer

inline void watch_enable_buzzer() {
}

inline void watch_set_buzzer_period(uint32_t period) {
}

inline void watch_set_buzzer_on() {
}

inline void watch_set_buzzer_off() {
}

const uint16_t NotePeriods[108] = {31047, 29301, 27649, 26079, 24617, 23224, 21923, 20683, 19515, 18418, 17377, 16399, 15477, 14603, 13780, 13004, 12272, 11580, 10926, 10311, 9730, 9181, 8664, 8175, 7714, 7280, 6869, 6483, 6117, 5772, 5447, 5140, 4850, 4577, 4319, 4076, 3846, 3629, 3425, 3232, 3050, 2878, 2715, 2562, 2418, 2282, 2153, 2032, 1917, 1809, 1707, 1611, 1520, 1435, 1354, 1277, 1205, 1137, 1073, 1013, 956, 902, 851, 803, 758, 715, 675, 637, 601, 567, 535, 505, 476, 450, 424, 400, 378, 357, 336, 317, 300, 283, 267, 252, 238, 224, 212, 200, 188, 178, 168, 158, 149, 141, 133, 125, 118, 112, 105, 99, 94, 89, 84, 79, 74, 70, 66, 63};

void watch_buzzer_play_note(BuzzerNote note, uint16_t duration_ms) {
}

//////////////////////////////////////////////////////////////////////////////////////////
// Real-time Clock

bool _watch_rtc_is_enabled() {
  return 0;
}

void watch_set_date_time(struct calendar_date_time date_time) {
}

void watch_get_date_time(struct calendar_date_time *date_time) {
  int arr[6];
  EM_ASM({
    var d = new Date();
    HEAP32[($0+ 0)>>2] = d.getFullYear();
    HEAP32[($0+ 4)>>2] = d.getMonth() + 1;
    HEAP32[($0+ 8)>>2] = d.getDate();
    HEAP32[($0+12)>>2] = d.getHours();
    HEAP32[($0+16)>>2] = d.getMinutes();
    HEAP32[($0+20)>>2] = d.getSeconds();
    }, arr);
  date_time->date.year  = arr[0];
  date_time->date.month = arr[1];
  date_time->date.day   = arr[2];
  date_time->time.hour  = arr[3];
  date_time->time.min   = arr[4];
  date_time->time.sec   = arr[5];
}

void watch_register_tick_callback(ext_irq_cb_t callback) {
  cbs[TICK] = callback;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Analog Input

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
  return 0;
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
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Debug UART

/*
 * UART methods are Copyright (c) 2014-2017, Alex Taradov <alex@taradov.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

void watch_enable_debug_uart(uint32_t baud) {
}

void watch_debug_putc(char c) {
}

void watch_debug_puts(char *s) {
}

//////////////////////////////////////////////////////////////////////////////////////////
// Deep Sleep

static void extwake_callback(uint8_t reason) {
}

void watch_register_extwake_callback(uint8_t pin, ext_irq_cb_t callback) {
}

void watch_store_backup_data(uint32_t data, uint8_t reg) {
}

uint32_t watch_get_backup_data(uint8_t reg) {
  return 0;
}

void watch_enter_deep_sleep() {
}
