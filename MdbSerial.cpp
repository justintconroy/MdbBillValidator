/*
  MdbSerial.cpp - MDB serial library for Wiring
  Copyright (c) 2014 Justin T. Conroy. All right reserved.
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
  === Changed to MdbSerial.cpp ===
  Modified 31 January 2014 by Justin T. Conroy
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"
#include "wiring_private.h"

// this next line disables the entire MdbSerial.cpp,
// this is so I can support Attiny series and any other chip without a uart
#if defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H) || defined(UBRR2H) || defined(UBRR3H)

#include "MdbSerial.h"

/*
 * on ATmega8, the uart and its bits are not numbered, so there is no "TXC0"
 * definition.
 */
#if !defined(TXC0)
#if defined(TXC)
#define TXC0 TXC
#elif defined(TXC1)
// Some devices have uart1 but no uart0
#define TXC0 TXC1
#else
#error TXC0 not definable in MdbSerial.h
#endif
#endif

// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which head is the index of the location
// to which to write the next incoming character and tail is the index of the
// location from which to read.
#if (RAMEND < 1000)
  #define SERIAL_BUFFER_SIZE 16
#else
  #define SERIAL_BUFFER_SIZE 64
#endif

struct ring_buffer
{
  unsigned char buffer[SERIAL_BUFFER_SIZE];
  unsigned char mode_buffer[SERIAL_BUFFER_SIZE];
  volatile unsigned int head;
  volatile unsigned int tail;
};

#if defined(USBCON)
  ring_buffer rx_buffer = { { 0 }, { 0 }, 0, 0};
  ring_buffer tx_buffer = { { 0 }, { 0 }, 0, 0};
#endif
#if defined(UBRRH) || defined(UBRR0H)
  ring_buffer rx_buffer  =  { { 0 }, { 0 }, 0, 0 };
  ring_buffer tx_buffer  =  { { 0 }, { 0 }, 0, 0 };
#endif
#if defined(UBRR1H)
  ring_buffer rx_buffer1  =  { { 0 }, { 0 }, 0, 0 };
  ring_buffer tx_buffer1  =  { { 0 }, { 0 }, 0, 0 };
#endif
#if defined(UBRR2H)
  ring_buffer rx_buffer2  =  { { 0 }, { 0 }, 0, 0 };
  ring_buffer tx_buffer2  =  { { 0 }, { 0 }, 0, 0 };
#endif
#if defined(UBRR3H)
  ring_buffer rx_buffer3  =  { { 0 }, { 0 }, 0, 0 };
  ring_buffer tx_buffer3  =  { { 0 }, { 0 }, 0, 0 };
#endif

inline void store_char(unsigned char c, unsigned char m, ring_buffer *buffer)
{
  unsigned int i = (unsigned int)(buffer->head + 1) % SERIAL_BUFFER_SIZE;

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if (i != buffer->tail) {
    buffer->buffer[buffer->head] = c;
    buffer->mode_buffer[buffer->head] = m;
    buffer->head = i;
  }
}

#if !defined(USART0_RX_vect) && defined(USART1_RX_vect)
// do nothing - on the 32u4 the first USART is USART1
#else
#if !defined(USART_RX_vect) && !defined(USART0_RX_vect) && \
    !defined(USART_RXC_vect)
  #error "Don't know what the Data Received vector is called for the first UART"
#else
  void serialEvent() __attribute__((weak));
  void serialEvent() {}
  #define serialEvent_implemented
#if defined(USART_RX_vect)
  ISR(USART_RX_vect)
#elif defined(USART0_RX_vect)
  ISR(USART0_RX_vect)
#elif defined(USART_RXC_vect)
  ISR(USART_RXC_vect) // ATmega8
#endif
  {
  #if defined(UDR0)
    if (bit_is_clear(UCSR0A, UPE0)) {
      unsigned char m = (UCSR0B >> 1) & 0x01;
      unsigned char c = UDR0;
      store_char(c, m, &rx_buffer);
    } else {
      unsigned char m = (UCSR0B >> 1) & 0x01;
      unsigned char c = UDR0;
    };
  #elif defined(UDR)
    if (bit_is_clear(UCSRA, PE)) {
      unsigned char m = (UCSRB >> 1) & 0x01;
      unsigned char c = UDR;
      store_char(c, m, &rx_buffer);
    } else {
      unsigned char m = (UCSRB >> 1) & 0x01;
      unsigned char c = UDR;
    };
  #else
    #error UDR not defined
  #endif
  }
#endif
#endif

#if defined(USART1_RX_vect)
  void serialEvent1() __attribute__((weak));
  void serialEvent1() {}
  #define serialEvent1_implemented
  ISR(USART1_RX_vect)
  {
    if (bit_is_clear(UCSR1A, UPE1)) {
      unsigned char m = (UCSR1B >> 1) & 0x01;
      unsigned char c = UDR1;
      store_char(c, m, &rx_buffer1);
    } else {
      unsigned char m = (UCSR1B >> 1) & 0x01;
      unsigned char c = UDR1;
    };
  }
#endif

#if defined(USART2_RX_vect) && defined(UDR2)
  void serialEvent2() __attribute__((weak));
  void serialEvent2() {}
  #define serialEvent2_implemented
  ISR(USART2_RX_vect)
  {
    if (bit_is_clear(UCSR2A, UPE2)) {
      unsigned char m = (UCSR2B >> 1) & 0x01;
      unsigned char c = UDR2;
      store_char(c, m, &rx_buffer2);
    } else {
      unsigned char m = (UCSR2B >> 1) & 0x01;
      unsigned char c = UDR2;
    };
  }
#endif

#if defined(USART3_RX_vect) && defined(UDR3)
  void serialEvent3() __attribute__((weak));
  void serialEvent3() {}
  #define serialEvent3_implemented
  ISR(USART3_RX_vect)
  {
    if (bit_is_clear(UCSR3A, UPE3)) {
      unsigned char m = (UCSR3B >> 1) & 0x01;
      unsigned char c = UDR3;
      store_char(c, m, &rx_buffer3);
    } else {
      unsigned char m = (UCSR3B >> 1) & 0x01;
      unsigned char c = UDR3;
    };
  }
#endif

void serialEventRun(void)
{
#ifdef serialEvent_implemented
  if (MdbPort.available()) serialEvent();
#endif
#ifdef serialEvent1_implemented
  if (MdbPort1.available()) serialEvent1();
#endif
#ifdef serialEvent2_implemented
  if (MdbPort2.available()) serialEvent2();
#endif
#ifdef serialEvent3_implemented
  if (MdbPort3.available()) serialEvent3();
#endif
}


#if !defined(USART0_UDRE_vect) && defined(USART1_UDRE_vect)
// do nothing - on the 32u4 the first USART is USART1
#else
#if !defined(UART0_UDRE_vect) && !defined(UART_UDRE_vect) && !defined(USART0_UDRE_vect) && !defined(USART_UDRE_vect)
  #error "Don't know what the Data Register Empty vector is called for the first UART"
#else
#if defined(UART0_UDRE_vect)
ISR(UART0_UDRE_vect)
#elif defined(UART_UDRE_vect)
ISR(UART_UDRE_vect)
#elif defined(USART0_UDRE_vect)
ISR(USART0_UDRE_vect)
#elif defined(USART_UDRE_vect)
ISR(USART_UDRE_vect)
#endif
{
  if (tx_buffer.head == tx_buffer.tail) {
    // Buffer empty, so disable interrupts
#if defined(UCSR0B)
    cbi(UCSR0B, UDRIE0);
#else
    cbi(UCSRB, UDRIE);
#endif
  }
  else {
    // There is more data in the output buffer. Send the next byte
    unsigned char c = tx_buffer.buffer[tx_buffer.tail];
    unsigned char m = tx_buffer.mode_buffer[tx_buffer.tail];
    tx_buffer.tail = (tx_buffer.tail + 1) % SERIAL_BUFFER_SIZE;

  #if defined(UDR0)
    // Write mode bit.
    UCSR0B &= ~(1 << TXB80);
    if (m)
      UCSR0B |= (1 << TXB80);

    // Write data byte.
    UDR0 = c;
  #elif defined(UDR)
    // Write mode bit.
    UCSRB &= ~(1 << TXB8);
    if (m)
      UCSRB |= (1 << TXB8);

    // Write data byte.
    UDR = c;
  #else
    #error UDR not defined
  #endif
  }
}
#endif
#endif

#ifdef USART1_UDRE_vect
ISR(USART1_UDRE_vect)
{
  if (tx_buffer1.head == tx_buffer1.tail) {
    // Buffer empty, so disable interrupts
    cbi(UCSR1B, UDRIE1);
  }
  else {
    // There is more data in the output buffer. Send the next byte
    unsigned char c = tx_buffer1.buffer[tx_buffer1.tail];
    unsigned char m = tx_buffer1.mode_buffer[tx_buffer.tail];
    tx_buffer1.tail = (tx_buffer1.tail + 1) % SERIAL_BUFFER_SIZE;

    // Write mode bit.
    if (m)
      UCSR1B |= (1 << TXB8);
    else
      UCSR1B &= ~(1 << TXB8);

    // Write data byte.
    UDR1 = c;
  }
}
#endif

#ifdef USART2_UDRE_vect
ISR(USART2_UDRE_vect)
{
  if (tx_buffer2.head == tx_buffer2.tail) {
    // Buffer empty, so disable interrupts
    cbi(UCSR2B, UDRIE2);
  }
  else {
    // There is more data in the output buffer. Send the next byte
    unsigned char c = tx_buffer2.buffer[tx_buffer2.tail];
    unsigned char m = tx_buffer2.mode_buffer[tx_buffer.tail];
    tx_buffer2.tail = (tx_buffer2.tail + 1) % SERIAL_BUFFER_SIZE;

    // Write mode bit.
    if (m)
      UCSR2B |= (1 << TXB8);
    else
      UCSR2B &= ~(1 << TXB8);

    // Write data byte.
    UDR2 = c;
  }
}
#endif

#ifdef USART3_UDRE_vect
ISR(USART3_UDRE_vect)
{
  if (tx_buffer3.head == tx_buffer3.tail) {
    // Buffer empty, so disable interrupts
    cbi(UCSR3B, UDRIE3);
  }
  else {
    // There is more data in the output buffer. Send the next byte
    unsigned char c = tx_buffer3.buffer[tx_buffer3.tail];
    unsigned char m = tx_buffer3.mode_buffer[tx_buffer.tail];
    tx_buffer3.tail = (tx_buffer3.tail + 1) % SERIAL_BUFFER_SIZE;

    // Write mode bit.
    if (m)
      UCSR3B |= (1 << TXB8);
    else
      UCSR3B &= ~(1 << TXB8);

    // Write data byte.
    UDR3 = c;
  }
}
#endif


// Constructors ////////////////////////////////////////////////////////////////

MdbSerial::MdbSerial(ring_buffer *rx_buffer, ring_buffer *tx_buffer,
  volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
  volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
  volatile uint8_t *ucsrc, volatile uint8_t *udr,
  uint8_t rxen, uint8_t txen, uint8_t rxcie, uint8_t udrie, uint8_t u2x,
  uint8_t ucsz2, uint8_t ucsz1, uint8_t ucsz0, uint8_t upm1, uint8_t upm0,
  uint8_t umsel1, uint8_t umsel0)
{
  _rx_buffer = rx_buffer;
  _tx_buffer = tx_buffer;
  _ubrrh = ubrrh;
  _ubrrl = ubrrl;
  _ucsra = ucsra;
  _ucsrb = ucsrb;
  _ucsrc = ucsrc;
  _udr = udr;
  _rxen = rxen;
  _txen = txen;
  _rxcie = rxcie;
  _udrie = udrie;
  _u2x = u2x;
  _ucsz2 = ucsz2;
  _ucsz1 = ucsz1;
  _ucsz0 = ucsz0;
  _upm1 = upm1;
  _upm0 = upm0;
  _umsel1 = umsel1;
  _umsel0 = umsel0;

}

// Public Methods //////////////////////////////////////////////////////////////

// Read back the value of one of the registers listed below. For DEBUG purposes.
uint8_t MdbSerial::readRegister(int regNum)
{
  uint8_t value = 0;

  switch(regNum)
  {
    case 0:
      value = UCSR0A;
      break;
    case 1:
      value = UCSR0B;
      break;
    case 2:
      value = UCSR0C;
      break;
    case 3:
      value = UDR0;
    default:
      value = 0;
      break;
  }

  return value;
}

// Get the value of the current TX buffer head. Used for DEBUG purposes.
unsigned int MdbSerial::getTxHead()
{
  return _tx_buffer->head;
}

// Get the value of the current TX buffer tail. Used for DEBUG purposes.
unsigned int MdbSerial::getTxTail()
{
  return _tx_buffer->tail;
}

void MdbSerial::begin()
{
  // Force 9600 baud, it's the only speed supported for MDB.
  unsigned long baud = 9600;
  uint16_t baud_setting;

  *_ucsra = 0;
  baud_setting = (F_CPU / 8 / baud - 1) / 2;

  // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
  *_ubrrh = baud_setting >> 8;
  *_ubrrl = baud_setting;

  transmitting = false;

  // Receiver Enable.
  sbi(*_ucsrb, _rxen);
  // Transmitter Enable.
  sbi(*_ucsrb, _txen);
  // RX Complete Interrupt Enable.
  sbi(*_ucsrb, _rxcie);
  // Disable Data Register Empty Interrupt.
  cbi(*_ucsrb, _udrie);

  // Set Character size to 9-bit.
  sbi(*_ucsrb, _ucsz2);
  sbi(*_ucsrc, _ucsz1);
  sbi(*_ucsrc, _ucsz0);

  // Disable the parity bit.
  cbi(*_ucsrc, _upm1);
  cbi(*_ucsrc, _upm0);
}

void MdbSerial::end()
{
  // wait for transmission of outgoing data
  while (_tx_buffer->head != _tx_buffer->tail)
    ;

  cbi(*_ucsrb, _rxen);
  cbi(*_ucsrb, _txen);
  cbi(*_ucsrb, _rxcie);
  cbi(*_ucsrb, _udrie);

  // clear any received data
  _rx_buffer->head = _rx_buffer->tail;
}

int MdbSerial::available(void)
{
  return (unsigned int)(SERIAL_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % SERIAL_BUFFER_SIZE;
}

int MdbSerial::peek(void)
{
  if (_rx_buffer->head == _rx_buffer->tail) {
    return -1;
  } else {
    unsigned char m = _rx_buffer->mode_buffer[_rx_buffer->tail];
    unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
    return ((m << 8) | c);
  }
}

int MdbSerial::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if (_rx_buffer->head == _rx_buffer->tail) {
    return -1;
  } else {
    // Read the mode bit first.
    unsigned char m = _rx_buffer->mode_buffer[_rx_buffer->tail];
    unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
    _rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % SERIAL_BUFFER_SIZE;

    // Return the mode and the data together in a single integer.
    return ((m << 8) | c);
  }
}

void MdbSerial::flush()
{
  // UDR is kept full while the buffer is not empty, so TXC triggers when EMPTY && SENT
  while (transmitting && ! (*_ucsra & _BV(TXC0)));
  transmitting = false;
}

size_t MdbSerial::write(uint8_t c, uint8_t m)
{
  unsigned int i = (_tx_buffer->head + 1) % SERIAL_BUFFER_SIZE;

  // If the output buffer is full, there's nothing for it other than to
  // wait for the interrupt handler to empty it a bit
  // ???: return 0 here instead?
  while (i == _tx_buffer->tail)
    ;

  // Bitwise invert everything.
  //c = ~c;
  //m = (~m) & 0x01;

  _tx_buffer->buffer[_tx_buffer->head] = c;
  _tx_buffer->mode_buffer[_tx_buffer->head] = m;
  _tx_buffer->head = i;

  sbi(*_ucsrb, _udrie);
  // clear the TXC bit -- "can be cleared by writing a one to its bit location"
  transmitting = true;
  sbi(*_ucsra, TXC0);

  return 1;
}

MdbSerial::operator bool() {
  return true;
}

// Preinstantiate Objects //////////////////////////////////////////////////////

#if defined(UBRRH) && defined(UBRRL)
  MdbSerial MdbPort(&rx_buffer, &tx_buffer, &UBRRH, &UBRRL, &UCSRA, &UCSRB, &UCSRC, &UDR, RXEN, TXEN, RXCIE, UDRIE, U2X, UCSZ2, UCSZ1, UCSZ0, UPM1, UPM0, UMSEL1, UMSEL0);
#elif defined(UBRR0H) && defined(UBRR0L)
  MdbSerial MdbPort(&rx_buffer, &tx_buffer, &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0, RXEN0, TXEN0, RXCIE0, UDRIE0, U2X0, UCSZ02, UCSZ01, UCSZ00, UPM01, UPM00, UMSEL01, UMSEL00);
#elif defined(USBCON)
  // do nothing - Serial object and buffers are initialized in CDC code
#else
  #error no serial port defined  (port 0)
#endif

#if defined(UBRR1H)
  MdbSerial MdbPort1(&rx_buffer1, &tx_buffer1, &UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, &UCSR1C, &UDR1, RXEN1, TXEN1, RXCIE1, UDRIE1, U2X1, UCSZ12, UCSZ11, UCSZ10, UPM11, UPM10, UMSEL11, UMSEL10);
#endif
#if defined(UBRR2H)
  MdbSerial MdbPort2(&rx_buffer2, &tx_buffer2, &UBRR2H, &UBRR2L, &UCSR2A, &UCSR2B, &UCSR2C, &UDR2, RXEN2, TXEN2, RXCIE2, UDRIE2, U2X2, UCSZ22, UCSZ21, UCSZ20, UPM21, UPM20, UMSEL21, UMSEL20);
#endif
#if defined(UBRR3H)
  MdbSerial MdbPort3(&rx_buffer3, &tx_buffer3, &UBRR3H, &UBRR3L, &UCSR3A, &UCSR3B, &UCSR3C, &UDR3, RXEN3, TXEN3, RXCIE3, UDRIE3, U2X3, UCSZ32, UCSZ31, UCSZ30, UPM31, UPM30, UMSEL31, UMSEL30);
#endif

#endif // whole file

