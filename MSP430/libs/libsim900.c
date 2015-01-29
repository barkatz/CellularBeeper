#include <sim900.h>
#include <lcdi2c.h>
#include <utils.h>
#include <uart.h>

#define MAX_SIM900_LINE_LEN 64

sim900_status_t sim900_status;

static char pending_rx_line[MAX_SIM900_LINE_LEN];
static word pending_rx_line_len;

static word try_rx_line();
static void parse_rx_line(char *rx_line, word rx_line_len);
void handle_rx_cmt_pdu(char *pdu, word pdu_len);

void sim900_init() {
  // TODO turn on the device
  
  // TODO for now we assume that the device is already ready on init
  sim900_status = SIM900STS_READY;

  pending_rx_line_len = 0;
}

void sim900_do_work() {
  word rx_line_len;

  rx_line_len = try_rx_line();
  if (rx_line_len == 0)
    return;

  parse_rx_line(pending_rx_line, rx_line_len);
}

static word try_rx_line() {
  word rx_line_len;

  // Try reading a new byte from the SIM900 UART
  if (uart_getc((byte*)&pending_rx_line[pending_rx_line_len])) {
    pending_rx_line_len++;

    // Check if we have reached the end of the line
    if (pending_rx_line_len >= 2 &&
        (pending_rx_line[pending_rx_line_len-2] == '\r') &&
        (pending_rx_line[pending_rx_line_len-1] == '\n')) {

      rx_line_len = pending_rx_line_len;
      // Get ready for a new line to RX
      pending_rx_line_len = 0;

      // Return the length of the received line
      return rx_line_len;
    } else if (pending_rx_line_len >= MAX_SIM900_LINE_LEN) {
      ASSERT(0);
    }
  }

  return 0;
}

typedef enum parse_state_t {
  STATE_IDLE,

  STATE_CMT_RXED, // Recived the +CMT unsolicited reply, which means an SMS payload is on its way
} parse_state_t;

parse_state_t parse_state = STATE_IDLE;

static void parse_rx_line(char *rx_line, word rx_line_len) {
  switch (parse_state) {
    case STATE_IDLE:
      if (startswith(rx_line, "+CMT: "))
        parse_state = STATE_CMT_RXED;
      break;

    case STATE_CMT_RXED:
      handle_rx_cmt_pdu(rx_line, rx_line_len);
      parse_state = STATE_IDLE;
      break;

    default:
      ASSERT(0);
  }
}

static byte *safe_consume(char **pdu, word *pdu_len, byte count) {
  char *retval;

  if (*pdu_len < count)
    return 0;

  retval = *pdu;
  *pdu += count;
  *pdu_len -= count;

  return (byte*)retval;
}

static byte hex2nibble(char hex) { 
  if ((hex >= '0') && (hex <= '9')) {
    return (byte)(hex - '0');
  } else if (hex >= 'A' && hex <= 'F') {
    return (byte)(hex - 'A' + 10);
  } else {
    ASSERT(0);
  }
}

static char nibble2hex(byte nib) {
  if (nib < 10)
    return '0' + nib;
  else
    return 'A' + (nib-10);
}

static word decode_hex(char* pdu, word pdu_len) {
  int i;

  ASSERT(pdu_len % 2 == 0);

  for(i = 0; i < pdu_len; i += 2)
    pdu[i/2] = hex2nibble(pdu[i])<<4 | hex2nibble(pdu[i+1]);
  
  return pdu_len/2;
}

static void trace_byte(byte b) {
  lcdi2c_putc(nibble2hex((b&0xf0)>>4));
  lcdi2c_putc(nibble2hex(b&0xf));
  lcdi2c_putc('#');
}

static void trace_pbyte(byte *b) {
  if (b == 0)
    lcdi2c_putc('*');
  else
    trace_byte(*b);
}

static void decode_7bit(byte *encoded, word encoded_len, byte *decoded, byte *decoded_data_len) {
  byte *decoded_base = decoded;
  byte lowMask = 0x7f;
  byte highOffset = 7;
  byte nextByte = 0;
  
  while (encoded_len--) {
    *decoded++ = ((*encoded & lowMask) << (7 - highOffset)) | nextByte;
    nextByte = (*encoded++ & ~lowMask) >> highOffset;
    if (lowMask > 1) {
      highOffset -= 1;
      lowMask >>= 1;
    } else {
      *decoded++ = nextByte;
      lowMask = 0x7f;
      highOffset = 7;
      nextByte = 0;
    }
  }

  *decoded_data_len = (decoded - decoded_base);
}

void handle_rx_cmt_pdu(char *pdu, word pdu_len) {
  // The PDU is parsed according to:
  // - http://www.smartposition.nl/resources/sms_pdu.html 
  // - 3GPP TS 23.040 (Technical realization of the Short Message Service (SMS)):
  //   http://www.etsi.org/deliver/etsi_ts/123000_123099/123040/12.02.00_60/ts_123040v120200p.pdf
  // - 3GPP TS 23.038 (Alphabets and language-specific information):
  //   http://www.etsi.org/deliver/etsi_ts/123000_123099/123038/12.00.00_60/ts_123038v120000p.pdf

  byte smsc_len;
  byte addr_len;
  byte addr_len_in_bytes;
  byte *addr;
  byte data_len;
  byte data[16];
  byte decoded_data_len;

  byte i;
  byte trash;

    // Decode the buffer.
  pdu_len = decode_hex(pdu, pdu_len-2);

    // Read the SMSC address len
  smsc_len = *safe_consume(&pdu, &pdu_len, 1);

    // Consume the SMSC address
  trash = *safe_consume(&pdu, &pdu_len, smsc_len);

    // Verify the SMS-DELIVER octet (WTF?)
  trash = *safe_consume(&pdu, &pdu_len, 1);
  if (trash != 0x04)
  {
    TRACE("invalid SMS-DELIVER: %u", trash);
    return;
  }

    // Read the sender address len
  addr_len = *safe_consume(&pdu, &pdu_len, 1);

    // Verify the sender addr type
  trash = *safe_consume(&pdu, &pdu_len, 1);
  if (trash != 0x91)
  {
    TRACE("invalid sender addr type: %u", trash);
    return;
  }

    // Parse the sender addr
  addr_len_in_bytes = (addr_len+1)/2;
    // TODO parse this
  addr = safe_consume(&pdu, &pdu_len, addr_len_in_bytes);
  for (i = 0; i < addr_len_in_bytes; i++) {
    lcdi2c_putc(nibble2hex(addr[i]&0xf));
    lcdi2c_putc(nibble2hex(addr[i]>>4));
  }
  lcdi2c_puts(": ");

  TRACE("1");

    // Verify that the protocol identifier is 'Default store and forward short message'
  trash = *safe_consume(&pdu, &pdu_len, 1);
  if (trash != 0x00)
  {
    TRACE("invalid protocol identifier: %u", trash);
    return;
  }

  TRACE("2");

    // Verify the data encoding scheme (WTF?)
  trash = *safe_consume(&pdu, &pdu_len, 1);
  if (trash != 0x00)
  {
    TRACE("invalid data encoding scheme: %u", trash);
    return;
  }

  TRACE("3");

    // Consume the timestamp
    // TODO parse this
  trash = *safe_consume(&pdu, &pdu_len, 7);

  TRACE("4");

    // Read the user data len
  data_len = *safe_consume(&pdu, &pdu_len, 1);
  lcdi2c_putc(nibble2hex(data_len>>4));
  lcdi2c_putc(nibble2hex(data_len&0xf));
  lcdi2c_newline();

  TRACE("data_len = %d", data_len);

    // Read the user data
  if (data_len > 14)
  {
    TRACE("too large data len: %u", data_len);
    return;
  }

  TRACE("5");

  addr = safe_consume(&pdu, &pdu_len, data_len);
  TRACE("6");
  decode_7bit(addr, data_len, data, &decoded_data_len);
  TRACE("7");
  for (i = 0; i < decoded_data_len; i++) {
    lcdi2c_putc(data[i]);
  }
  lcdi2c_newline();
}