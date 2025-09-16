
#include <Arduino.h>

#if !defined(ARDUINO_ESP32C3_DEV) // This is due to a bug in RISC-V compiler, which requires unused function sections :-(.
#define DISABLE_CODE_FOR_RECEIVER // Disables static receiver code like receive timer ISR handler and static IRReceiver and irparams data. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not required.
#endif

#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.
#include <IRremote.hpp>

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);

#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
  
}
const uint8_t rawDataP[] PROGMEM
= { 180, 90 /*Start bit*/, 11, 11, 11, 11, 11, 34, 11, 11/*0010 0x4 of 8 bit address LSB first*/, 11, 11, 11, 11, 11, 11, 11,
        11/*0000*/, 11, 34, 11, 34, 11, 11, 11, 34/*1101 0xB*/, 11, 34, 11, 34, 11, 34, 11, 34/*1111*/, 11, 11, 11, 11, 11, 11, 11,
        34/*0001 0x08 of command LSB first*/, 11, 34, 11, 11, 11, 11, 11, 11/*1000 0x01*/, 11, 34, 11, 34, 11, 34, 11,
        11/*1110 Inverted 8 of command*/, 11, 11, 11, 34, 11, 34, 11, 34/*0111 inverted 1 of command*/, 11 /*stop bit*/};

/*
 * The same frame as above. Values are NOT multiple of 50, but are taken from the NEC timing definitions
 */
const uint16_t rawData[] = { 9000, 4500/*Start bit*/, 560, 560, 560, 560, 560, 1690, 560,
        560/*0010 0x4 of 8 bit address LSB first*/, 560, 560, 560, 560, 560, 560, 560, 560/*0000*/, 560, 1690, 560, 1690, 560, 560,
        560, 1690/*1101 0xB - inverted 0x04*/, 560, 1690, 560, 1690, 560, 1690, 560, 1690/*1111 - inverted 0*/, 560, 560, 560, 560,
        560, 560, 560, 1690/*0001 0x08 of command LSB first*/, 560, 1690, 560, 560, 560, 560, 560, 560/*1000 0x01*/, 560, 1690, 560,
        1690, 560, 1690, 560, 560/*1110 Inverted 8 of command*/, 560, 560, 560, 1690, 560, 1690, 560,
        1690/*1111 inverted 0 of command*/, 560 /*stop bit*/}; // Using exact NEC timing

void loop() {

#if FLASHEND > 0x1FFF // For more than 8k flash => not for ATtiny85 etc.
    /*
     * Send hand crafted data from RAM
     */
    Serial.println(F("Send NEC 8 bit address=0x04 (0xFB04) and command 0x18 (0xE718) with exact timing (16 bit array format)"));
    Serial.flush();
    IrSender.sendRaw(rawData, sizeof(rawData) / sizeof(rawData[0]), NEC_KHZ); // Note the approach used to automatically calculate the size of the array.

    delay(1000); // delay must be greater than 8 ms (RECORD_GAP_MICROS), otherwise the receiver sees it as one long signal

#endif

    
    IrSender.sendRaw_P(rawDataP, sizeof(rawDataP) / sizeof(rawDataP[0]), NEC_KHZ);

    delay(1000); // delay must be greater than 8 ms (RECORD_GAP_MICROS), otherwise the receiver sees it as one long sign

    delay(3000);
}
