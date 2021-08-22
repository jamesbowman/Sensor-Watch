/*
 * MIT License
 *
 * Copyright (c) 2020 Joey Castillo
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
/// @file watch.h

#ifndef WATCH_H_
#define WATCH_H_
#include <stdint.h>
#include <stdbool.h>
#include "notes.h"
#include "hpl_calendar.h"

typedef void (*ext_irq_cb_t)();

void delay_ms(int n);

#define BTN_LIGHT 1
#define BTN_MODE 2
#define BTN_ALARM 3
#define TICK  4
#define BUZZER 20
void watch_dispatch_callback(uint8_t pin);

/** @mainpage Sensor Watch Documentation
 *  @brief This documentation covers most of the functions you will use to interact with the Sensor Watch
           hardware. It is divided into the following sections:
            - @ref app - This section covers the functions that you will implement in your app.c file when designing a
                         Sensor Watch app.
            - @ref slcd - This section covers functions related to the Segment LCD display driver, which is responsible
                          for displaying strings of characters and indicators on the main watch display.
            - @ref buttons - This section covers functions related to the three buttons: Light, Mode and Alarm.
            - @ref led - This section covers functions related to the bi-color red/green LED mounted behind the LCD.
            - @ref buzzer - This section covers functions related to the piezo buzzer.
            - @ref rtc - This section covers functions related to the SAM L22's real-time clock peripheral, including
                         date, time and alarm functions.
            - @ref adc - This section covers functions related to the SAM L22's analog-to-digital converter, as well as
                         configuring and reading values from the three analog-capable pins on the 9-pin connector.
            - @ref gpio - This section covers functions related to general-purpose input and output signals.
            - @ref i2c - This section covers functions related to the SAM L22's built-I2C driver, including configuring
                         the I2C bus, putting values directly on the bus and reading data from registers on I2C devices.
            - @ref debug - This section covers functions related to the debug UART, available on pin D1 of the 9-pin connector.
            - @ref deepsleep - This section covers functions related to preparing for and entering BACKUP mode, the
                               deepest sleep mode available on the SAM L22.
 */

/** @addtogroup app Application Framework
  * @brief This section covers the functions that you will implement in your app.c file when designing a Sensor Watch app.
  * @details You should be able to write a watch app by simply implementing these functions and declaring callbacks for
  *          various GPIO and peripheral interrupts. The main.c file takes care of calling these functions for you. The
  *          general flow:
  *
  *            1. Your app_init() function is called.
  *               - This method should only be used to set your initial application state.
  *            2. If your app is waking from BACKUP, app_wake_from_deep_sleep()  is called.
  *               - If you saved state in the RTC's backup registers, you can restore it here.
  *            3. Your app_setup() method is called.
  *               - You may wish to enable some functionality and peripherals here.
  *               - You should definitely set up some interrupts here.
  *            4. The main run loop begins: your app_loop() function is called.
  *               - Run code and update your UI here.
  *               - Return true if your app is prepared to enter STANDBY mode.
  *            5. This step differs depending on the value returned by app_loop:
  *               - If you returned false, execution resumes at (4).
  *               - If you returned true, app_prepare_for_sleep() is called; execution moves on to (6).
  *            6. The microcontroller enters the STANDBY sleep mode.
  *               - No user code will run, and the watch will enter a low power mode.
  *               - The watch will remain in this state until an interrupt wakes it.
  *            7. Once woken from STANDBY, your app_wake_from_sleep() function is called.
  *               - After this, execution resumes at (4).
  */
/// @{
/** @brief A function you will implement to initialize your application state. The app_init function is called before
  *        anything else. Use it to set up any internal data structures or application state required by your app,
  *        but don't configure any peripherals just yet.
  */
void app_init();

/** @brief A function you will implement to wake from deep sleep mode. The app_wake_from_deep_sleep function is only
  *        called if your app is waking from the ultra-low power BACKUP sleep mode. You may have chosen to store some
  *        state in the RTC's backup registers prior to entering this mode. You may restore that state here.
  */
void app_wake_from_deep_sleep();

/** @brief A function you will implement to set up your application. The app_setup function is like setup() in Arduino.
  *        It is called once when the program begins. You should set pin modes and enable any peripherals you want to
  *        set up (real-time clock, I2C, etc.) Depending on your application, you may or may not want to configure
  *        sensors on your sensor board here. For example, a low-power accelerometer that will run at all times should
  *        be configured here, whereas you may want to enable a more power-hungry sensor only when you need it.
  * @note If your app enters the ultra-low power BACKUP sleep mode, this function will be called again when it wakes
  *       from that deep sleep state. In this state, the RTC will still be configured with the correct date and time.
  */
void app_setup();

/** @brief A function you will implement to serve as the app's main run loop. This method will be called repeatedly,
           or if you enter STANDBY sleep mode, as soon as the device wakes from sleep.
  * @return You should return true if your app is prepared to enter STANDBY sleep mode. If you return false, your
  *         app's app_loop method will be called again immediately. Note that in STANDBY mode, the watch will consume
  *         only about 95 microamperes of power, whereas if you return false and keep the app awake, it will consume
  *         about 355 microamperes. This is the difference between months of battery life and days. As much as
  *         possible, you should limit the amount of time your app spends awake.
  * @note Only the RTC, the segment LCD controller and the external interrupt controller run in STANDBY mode. If you
  *       are using, e.g. the PWM function to set a custom LED color, you should return false here until you are
  *       finished with that operation. Note however that the peripherals will continue running after waking up,
  *       so e.g. the I2C controller, if configured, will sleep in STANDBY. But you can use it again as soon as your
  *       app wakes up.
  */
bool app_loop();

/** @brief A function you will implement to prepare to enter STANDBY sleep mode. The app_prepare_for_sleep function is
 *         called before the watch goes into the STANDBY sleep mode. In STANDBY mode, most peripherals are shut down,
 *         and no code will run until the watch receives an interrupt (generally either the 1Hz tick or a press on one
 *         of the buttons).
 * @note If you are PWM'ing the LED or playing a sound on the buzzer, the TC/TCC peripherals that drive those operations
 *       will not run in STANDBY. BUT! the output pins will retain the state they had when entering standby. This means
 *       you could end up entering standby with an LED on and draining power, or with a DC potential across the piezo
 *       buzzer that could damage it if left in this state. If your app_loop does not prevent sleep during these
 *       activities, you should make sure to disable these outputs in app_prepare_for_sleep.
 */
void app_prepare_for_sleep();

/** @brief A method you will implement to configure the app after waking from STANDBY sleep mode.
  */
void app_wake_from_sleep();

/// Called by main.c while setting up the app. You should not call this from your app.
void _watch_init();
/// @}


/** @addtogroup slcd Segment LCD Display
  * @brief This section covers functions related to the Segment LCD display driver, which is responsible
  *        for displaying strings of characters and indicators on the main watch display.
  * @details The segment LCD controller consumes about 3 microamperes of power with no segments on, and
  *          about 4 microamperes with all segments on. There is also a slight power impact associated
  *          with updating the screen (about 1 microampere to update at 1 Hz). For the absolute lowest
  *          power operation, update the display only when its contents have changed, and disable the
  *          SLCD peripheral when the screen is not in use.
  *          For a map of all common and segment pins, see <a href="segmap.html">segmap.html</a>. You can
  *          hover over any segment in that diagram to view the common and segment pins associated with
  *          each segment of the display.
  */
/// @{

/// An enum listing the icons and indicators available on the watch.
typedef enum WatchIndicatorSegment {
    WATCH_INDICATOR_SIGNAL = 0, ///< The hourly signal indicator; also useful for indicating that sensors are on.
    WATCH_INDICATOR_BELL,       ///< The small bell indicating that an alarm is set.
    WATCH_INDICATOR_PM,         ///< The PM indicator, indicating that a time is in the afternoon.
    WATCH_INDICATOR_24H,        ///< The 24H indicator, indicating that the watch is in a 24-hour mode.
    WATCH_INDICATOR_LAP         ///< The LAP indicator; the F-91W uses this in its stopwatch UI.
} WatchIndicatorSegment;

/** @brief Enables the Segment LCD display.
  * Call this before attempting to set pixels or display strings.
  */
void watch_enable_display();

/** @brief Sets a pixel. Use this to manually set a pixel with a given common and segment number.
  *        See <a href="segmap.html">segmap.html</a>.
  * @param com the common pin, numbered from 0-2.
  * @param seg the segment pin, numbered from 0-23.
  */
void watch_set_pixel(uint8_t com, uint8_t seg);

/** @brief Clears a pixel. Use this to manually clear a pixel with a given common and segment number.
  *        See <a href="segmap.html">segmap.html</a>.
  * @param com the common pin, numbered from 0-2.
  * @param seg the segment pin, numbered from 0-23.
  */
void watch_clear_pixel(uint8_t com, uint8_t seg);

/** @brief Displays a string at the given position, starting from the top left. There are ten digits.
           A space in any position will clear that digit.
  * @param string A null-terminated string.
  * @param position The position where you wish to start displaying the string. The day of week digits
  *                 are positions 0 and 1; the day of month digits are positions 2 and 3, and the main
  *                 clock line occupies positions 4-9.
  * @note This method does not clear the display; if for example you display a two-character string at
          position 0, positions 2-9 will retain whatever state they were previously displaying.
  */
void watch_display_string(char *string, uint8_t position);

/** @brief Turns the colon segment on.
  */
void watch_set_colon();

/** @brief Turns the colon segment off.
  */
void watch_clear_colon();

/** @brief Sets an indicator on the LCD. Use this to turn on one of the indicator segments.
  * @param indicator One of the indicator segments from the enum. @see WatchIndicatorSegment
  */
void watch_set_indicator(WatchIndicatorSegment indicator);

/** @brief Clears an indicator on the LCD. Use this to turn off one of the indicator segments.
  * @param indicator One of the indicator segments from the enum. @see WatchIndicatorSegment
  */
void watch_clear_indicator(WatchIndicatorSegment indicator);

/** @brief Clears all indicator segments.
  * @see WatchIndicatorSegment
  */
void watch_clear_all_indicators();

/// @}


/** @addtogroup led LED Control
  * @brief This section covers functions related to the bi-color red/green LED mounted behind the LCD.
  * @details The SAM L22 is an exceedingly power efficient chip, whereas the LED's are relatively power-
  *          hungry. The green LED, at full power, consumes more power than the whole chip in active mode,
  *          and the red LED consumes about twelve times as much power! The LED's should thus be used only
  *          sparingly in order to preserve battery life.
  * @todo Explore running the TC3 PWM driver in standby mode; this would require that the user disable it
  *       in app_prepare_for_sleep, but could allow for low power, low duty indicator LED usage.
  */
/// @{
/** @brief Enables the LED.
  * @param pwm if true, enables PWM output for brightness control (required to use @ref watch_set_led_color).
               If false, configures the LED pins as digital outputs.
  * @note The TC driver required for PWM mode does not run in STANDBY mode. You should keep your app awake
          while PWM'ing the LED's, and disable them before going to sleep.
  */
void watch_enable_led(bool pwm);

/** @brief Disables the LEDs.
  * @param pwm if true, disables the PWM output. If false, disables the digital outputs.
  */
void watch_disable_led(bool pwm);

/** @brief Sets the LED to a custom color by modulating each output's duty cycle.
  * @param red The red value.
  * @param green The green value.
  * @note still working on this, 0-65535 works now but these values may change.
  */
void watch_set_led_color(uint16_t red, uint16_t green);

/** @brief Sets the red LED to full brightness, and turns the green LED off.
  * @note Of the two LED's in the RG bi-color LED, the red LED is the less power-efficient one (~4.5 mA).
  */
void watch_set_led_red();

/** @brief Sets the green LED to full brightness, and turns the red LED off.
  * @note Of the two LED's in the RG bi-color LED, the green LED is the more power-efficient one (~0.44 mA).
  */
void watch_set_led_green();

/** @brief Sets both red and green LEDs to full brightness.
  * @note The total current draw between the two LED's in this mode will be ~5 mA, which is more than the
  *       watch draws in any other mode. Take care not to drain the battery.
  */
void watch_set_led_yellow();

/** @brief Turns both the red and the green LEDs off. */
void watch_set_led_off();
/// @}


/** @addtogroup buzzer Buzzer
  * @brief This section covers functions related to the piezo buzzer embedded in the F-91W's back plate.
  */
/// @{
/** @brief Enables the TCC peripheral, which drives the buzzer.
  */
void watch_enable_buzzer();

/** @brief Sets the period of the buzzer.
  * @param period The period of a single cycle for the PWM peripheral. You can use the following formula to
  *               convert a desired frequency to a period for this function: period = 513751 * (freq^−1.0043)
  */
void watch_set_buzzer_period(uint32_t period);

/** @brief Turns the buzzer output on. It will emit a continuous sound at the given frequency.
  * @note The TCC peripheral that drives the buzzer does not run in standby mode; if you wish for buzzer
  *       output to continue, you should prevent your app from going to sleep.
  */
void watch_set_buzzer_on();

/** @brief Turns the buzzer output off.
  */
void watch_set_buzzer_off();

/** @brief Plays the given note for a set duration.
  * @param note The note you wish to play, or BUZZER_NOTE_REST to disable output for the given duration.
  * @param duration_ms The duration of the note.
  * @note Note that this will block your UI for the duration of the note's play time, and it will
  *       after this call, the buzzer period will be set to the period of this note.
  */
void watch_buzzer_play_note(BuzzerNote note, uint16_t duration_ms);

/** @brief An array of periods for all the notes on a piano, corresponding to the names in BuzzerNote.
  */
extern const uint16_t NotePeriods[108];

/// @}


/** @addtogroup rtc Real-Time Clock
  * @brief This section covers functions related to the SAM L22's real-time clock peripheral, including
  *        date, time and alarm functions.
  * @details The real-time clock is the only peripheral that main.c enables for you. It is the cornerstone
  *          of low power operation on the watch, and it is required for several key functions that we
  *          assume will be available, namely the wake from BACKUP mode and the callback on the ALARM button.
  *          It is also required for the operation of the 1 Hz tick interrupt, which you will most likely use
  *          to wake from STANDBY mode.
  */
/// @{
/** @brief Called by main.c to check if the RTC is enabled.
  * You may call this function, but outside of app_init, it sbould always return true.
  */
bool _watch_rtc_is_enabled();

/** @brief Sets the system date and time.
  * @param date_time A struct representing the date and time you wish to set.
  */
void watch_set_date_time(struct calendar_date_time date_time);

/** @brief Returns the system date and time in the provided struct.
  * @param date_time A pointer to a calendar_date_time struct.
                     It will be populated with the correct date and time on return.
  */
void watch_get_date_time(struct calendar_date_time *date_time);

/** @brief Registers a "tick" callback that will be called once per second.
  * @param callback The function you wish to have called when the clock ticks.
  */
void watch_register_tick_callback(ext_irq_cb_t callback);
/// @}


/** @addtogroup adc Analog Input
  * @brief This section covers functions related to the SAM L22's analog-to-digital converter, as well as
  *        configuring and reading values from the three analog-capable pins on the 9-pin connector.
  */
/// @{
/** @brief Enables the ADC peripheral, and configures the selected pin for analog input.
  * @param pin One of pins A0, A1 or A2.
  */
void watch_enable_analog(const uint8_t pin);
/// @}


/** @addtogroup buttons Buttons
  * @brief This section covers functions related to the three buttons: Light, Mode and Alarm.
  * @details The buttons are the core input UI of the watch, and the way the user will interact with
  *          your application. They are active high, pulled down by the microcontroller, and triggered
  *          when one of the "pushers" brings a tab from the metal frame into contact with the edge
  *          of the board. Note that the buttons can only wake the watch from STANDBY mode (except maybe for the
  *          ALARM button; still working on that one). The external interrupt controller runs in
             STANDBY mode, but it does not runin BACKUP mode; to wake from BACKUP, buttons will not cut it,
  */
/// @{
/** @brief Enables the external interrupt controller for use with the buttons.
  * @note The BTN_ALARM button runs off of an interrupt in the the RTC controller, not the EIC. If your
  *       application ONLY makes use of the alarm button, you do not need to call this method; you can
  *       save ~5µA by leaving the EIC disabled and only registering a callback for BTN_ALARM.
  */
void watch_enable_buttons();

/** @brief Configures an external interrupt on one of the button pins.
  * @param pin One of pins BTN_LIGHT, BTN_MODE or BTN_ALARM.
  * @param callback The function you wish to have called when the button is pressed.
  * @note The BTN_ALARM button runs off of an interrupt in the the RTC controller, not the EIC. This
  *       implementation detail should not make any difference to your app,
  */
void watch_register_button_callback(const uint8_t pin, ext_irq_cb_t callback);
/// @}


/** @addtogroup gpio Digital Input and Output
  * @brief This section covers functions related to general-purpose input and output signals.
  */
/// @{
/** @brief Configures the selected pin for digital input.
  * @param pin The pin that you wish to act as an input.
  */
void watch_enable_digital_input(const uint8_t pin);

/** @brief Enables a pull-up resistor on the selected pin.
  * @param pin The pin that you wish to configure.
  */
void watch_enable_pull_up(const uint8_t pin);

/** @brief Enables a pull-down resistor on the selected pin.
  * @param pin The pin that you wish to configure.
  */
void watch_enable_pull_down(const uint8_t pin);

/** @brief Gets the level of the selected pin.
  * @param pin The pin whose value you wish to read.
  * @return true if the pin was logic high; otherwise, false.
  */
bool watch_get_pin_level(const uint8_t pin);

/** @brief Configures the selected pin for digital output.
  * @param pin The pin that you wish to act as an output.
  */
void watch_enable_digital_output(const uint8_t pin);

/** @brief Disables digital output on the selected pin.
  * @param pin The pin that you wish disable.
  */
void watch_disable_digital_output(const uint8_t pin);

/** @brief Sets the level of the selected pin.
  * @param pin The pin whose value you wish to set.
  * @param level The level you wish to set: true for high, false for low.
  */
void watch_set_pin_level(const uint8_t pin, const bool level);
/// @}


/** @addtogroup i2c I2C Controller Driver
  * @brief This section covers functions related to the SAM L22's built-I2C driver, including
  *        configuring the I2C bus, putting values directly on the bus and reading data from
  *        registers on I2C devices.
  */
/// @{
/** @brief Enables the I2C peripheral. Call this before attempting to interface with I2C devices.
  */
void watch_enable_i2c();

/** @brief Sends a series of values to a device on the I2C bus.
  * @param addr The address of the device you wish to talk to.
  * @param buf A series of unsigned bytes; the data you wish to transmit.
  * @param length The number of bytes in buf that you wish to send.
  */
void watch_i2c_send(int16_t addr, uint8_t *buf, uint16_t length);

/** @brief Receives a series of values from a device on the I2C bus.
  * @param addr The address of the device you wish to hear from.
  * @param buf Storage for the incoming bytes; on return, it will contain the received data.
  * @param length The number of bytes that you wish to receive.
  */
void watch_i2c_receive(int16_t addr, uint8_t *buf, uint16_t length);

/** @brief Writes a byte to a register in an I2C device.
  * @param addr The address of the device you wish to address.
  * @param reg The register on the device that you wish to set.
  * @param data The value that you wish to set the register to.
  */
void watch_i2c_write8(int16_t addr, uint8_t reg, uint8_t data);

/** @brief Reads a byte from a register in an I2C device.
  * @param addr The address of the device you wish to address.
  * @param reg The register on the device that you wish to read.
  * @return An unsigned byte representing the value of the register that was read.
  */
uint8_t watch_i2c_read8(int16_t addr, uint8_t reg);

/** @brief Reads an unsigned little-endian word from a register in an I2C device.
  * @param addr The address of the device you wish to address.
  * @param reg The register on the device that you wish to read.
  * @return An unsigned word representing the value of the register that was read.
  * @note This reads two bytes into the word in bus order. If the device returns
          the LSB first and then the MSB, you can use this value as returned.
          If the device returns the data in big-endian order or uses some other
          kind of fancy bit packing, you may need to shuffle some bits around.
  */
uint16_t watch_i2c_read16(int16_t addr, uint8_t reg);

/** @brief Reads three bytes as an unsigned little-endian int from a register in an I2C device.
  * @param addr The address of the device you wish to address.
  * @param reg The register on the device that you wish to read.
  * @return An unsigned word representing the value of the register that was read.
  * @note This reads three bytes into the word in bus order. If the device returns
          these bytes LSB first, you can use this value as returned. If there is a
          sign bit, the device returns the data in big-endian order, or it uses some
          other kind of fancy bit packing, you may need to shuffle some bits around.
  */
uint32_t watch_i2c_read24(int16_t addr, uint8_t reg);


/** @brief Reads an unsigned little-endian int from a register in an I2C device.
  * @param addr The address of the device you wish to address.
  * @param reg The register on the device that you wish to read.
  * @return An unsigned word representing the value of the register that was read.
  * @note This reads three bytes into the word in bus order. If the device returns
          these bytes LSB first, you can use this value as returned. If the device
          returns the data in big-endian order, or it uses some other kind of fancy
          bit packing, you may need to shuffle some bits around.
  */
uint32_t watch_i2c_read32(int16_t addr, uint8_t reg);
/// @}

/** @addtogroup debug Debug UART
  * @brief This section covers functions related to the debug UART, available on
  *        pin D1 of the 9-pin connector.
  * @todo Refactor this as a USB CDC so that folks can debug over USB.
  */
/// @{
/** @brief Initializes the debug UART.
  * @param baud The baud rate
  */
void watch_enable_debug_uart(uint32_t baud);

/** @brief Outputs a single character on the debug UART.
  * @param c The character you wish to output.
  */
void watch_debug_putc(char c);

/** @brief Outputs a string on the debug UART.
  * @param s A null-terminated string.
  */
void watch_debug_puts(char *s);
/// @}


/** @addtogroup deepsleep Deep Sleep Control
  * @brief This section covers functions related to preparing for and entering BACKUP mode, the
  *        deepest sleep mode available on the SAM L22
  */
/// @{
/** @brief Registers a callback on one of the RTC's external wake pins, which can wake the device
  * from deep sleep mode.
  * @param pin Either pin A2 or pin D1, the two external wake pins on the nine-pin connector.
  * @param callback The callback to be called if this pin triggers outside of deep sleep mode.
  * @note When in normal or STANDBY mode, this will function much like a standard external interrupt
  *       situation: these pins will wake from standby, and your callback will be called. However,
  *       if the device enters deep sleep and one of these pins wakes the device, your callback
  *       WILL NOT be called.
  */
void watch_register_extwake_callback(uint8_t pin, ext_irq_cb_t callback);

/** @brief Stores data in one of the RTC's backup registers, which retain their data in deep sleep.
  * @param data An unsigned 32 bit integer with the data you wish to store.
  * @param reg A register from 0-7.
  */
void watch_store_backup_data(uint32_t data, uint8_t reg);

/** @brief Gets 32 bits of data from the RTC's backup register, which retains its data in deep sleep.
  * @param reg A register from 0-7.
  * @return An unsigned 32 bit integer with the from the backup register.
  */
uint32_t watch_get_backup_data(uint8_t reg);

/** @brief Enters the SAM L22's lowest-power mode, BACKUP.
  * @details This function does some housekeeping before entering BACKUP mode. It first disables all
  *          peripherals except for the RTC, and disables the tick interrupt (since that would wake)
  *          us up from deep sleep. It also sets an external wake source on the ALARM button, if one
  *          was not already set. If you wish to wake from another source, such as one of the external
  *          wake interrupt pins on the 9-pin connector, set that up prior to calling this function.
  * @note If you have a callback set for an external wake interrupt, it will be called if triggered while
  *       in ACTIVE, IDLE or STANDBY modes, but it *will not be called* when waking from BACKUP.
  *       Waking from backup is effectively like waking from reset, except that your @ref
  *       app_wake_from_deep_sleep function will be called.
  * @warning In initial testing, it seems like the ALARM_BTN pin (PA02 RTC/IN2) cannot wake the device
  *          from deep sleep mode. There is an errata note (Reference: 15010, linked) that says that
  *          due to a silicon bug, PB01 cannot be used as RTC/IN2. It seems though that this bug may
  *          also affect PA02. As a result — and I'm very bummed about this — you cannot use deep sleep
  *          mode unless you set up an external wake interrupt using a device on the nine-pin connector
  *          (i.e. an accelerometer with an interrupt pin). Otherwise your only option for waking will
  *          be to unscrew the watch case and press the reset button on the back of the board.
  *          http://ww1.microchip.com/downloads/en/DeviceDoc/SAM_L22_Family_Errata_DS80000782B.pdf
  */
void watch_enter_deep_sleep();
/// @}

#endif /* WATCH_H_ */
