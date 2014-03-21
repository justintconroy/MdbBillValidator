/*
  MdbSerial.h - MDB serial library for Wiring
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

  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
  === Changed to MdbSerial.cpp ===
  Modified 31 January 2014 by Justin T. Conroy
*/

/*
  This library is designed specifically to support the MDB protocol over the
  standard USART pins. MDB is basically a serial protocol that uses a start
  bit, stop bit, and 9 data bits (8 bits for actual data, 1 bit is a mode
  bit). It always uses 9600 baud.

  For more information on the MDB protocol, refer to the MDB 4.2 specification
  document, which can be found at the following URL:

      http://www.vending.org/images/pdfs/technology/mdb_version_4-2.pdf

  This libary was modified from the HardwareSerial libary that comes with
  the Arduino software. Some copyright and modification information from
  that libary has been preserved above.
*/


#ifndef MdbSerial_h
#define MdbSerial_h

#include <inttypes.h>

#include "Stream.h"

struct ring_buffer;

class MdbSerial
{
  private:
    ring_buffer *_rx_buffer;
    ring_buffer *_tx_buffer;
    volatile uint8_t *_ubrrh;
    volatile uint8_t *_ubrrl;
    volatile uint8_t *_ucsra;
    volatile uint8_t *_ucsrb;
    volatile uint8_t *_ucsrc;
    volatile uint8_t *_udr;
    uint8_t _rxen;
    uint8_t _txen;
    uint8_t _rxcie;
    uint8_t _udrie;
    uint8_t _u2x;
    uint8_t _ucsz2;
    uint8_t _ucsz1;
    uint8_t _ucsz0;
    uint8_t _upm1;
    uint8_t _upm0;
    uint8_t _umsel1;
    uint8_t _umsel0;
    bool transmitting;
  public:
    MdbSerial(ring_buffer *rx_buffer, ring_buffer *tx_buffer,
      volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
      volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
      volatile uint8_t *ucsrc, volatile uint8_t *udr,
      uint8_t rxen, uint8_t txen, uint8_t rxcie, uint8_t udrie, uint8_t u2x,
      uint8_t ucsz2, uint8_t ucsz1, uint8_t ucsz0, uint8_t upm1, uint8_t upm0,
      uint8_t umsel1, uint8_t umsel0);
    void begin();
    void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t, uint8_t);
    //inline size_t write(unsigned long c, unsigned long m) { return write((uint8_t)c, (uint8_t)m); }
    //inline size_t write(long c, long m) { return write((uint8_t)c, (uint8_t)m); }
    //inline size_t write(unsigned int c, unsigned int m) { return write((uint8_t)c, (uint8_t)m); }
    //inline size_t write(int c, int m) { return write((uint8_t)c, (uint8_t)m); }
    //inline size_t write(unsigned char c, unsigned char m) { return write((uint8_t)c, (uint8_t)m); }
    //using Print::write; // pull in write(str) and write(buf, size) from Print

    virtual uint8_t readRegister(int);
    virtual unsigned int getTxHead();
    virtual unsigned int getTxTail();

    operator bool();
};

// Define config for Serial.begin(baud, config);
#define SERIAL_5N1 0x00
#define SERIAL_6N1 0x02
#define SERIAL_7N1 0x04
#define SERIAL_8N1 0x06
#define SERIAL_5N2 0x08
#define SERIAL_6N2 0x0A
#define SERIAL_7N2 0x0C
#define SERIAL_8N2 0x0E
#define SERIAL_5E1 0x20
#define SERIAL_6E1 0x22
#define SERIAL_7E1 0x24
#define SERIAL_8E1 0x26
#define SERIAL_5E2 0x28
#define SERIAL_6E2 0x2A
#define SERIAL_7E2 0x2C
#define SERIAL_8E2 0x2E
#define SERIAL_5O1 0x30
#define SERIAL_6O1 0x32
#define SERIAL_7O1 0x34
#define SERIAL_8O1 0x36
#define SERIAL_5O2 0x38
#define SERIAL_6O2 0x3A
#define SERIAL_7O2 0x3C
#define SERIAL_8O2 0x3E

#if defined(UBRRH) || defined(UBRR0H)
  extern MdbSerial MdbPort;
#endif
#if defined(UBRR1H)
  extern MdbSerial MdbPort1;
#endif
#if defined(UBRR2H)
  extern MdbSerial MdbPort2;
#endif
#if defined(UBRR3H)
  extern MdbSerial MdbPort3;
#endif

extern void serialEventRun(void) __attribute__((weak));

#endif
