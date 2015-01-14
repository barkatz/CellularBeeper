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
static void handle_rx_cmt_pdu(char *pdu, word pdu_len);

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

parse_state_t parse_state;

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

static void handle_rx_cmt_pdu(char *pdu, word pdu_len) {
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

  byte i;
  byte trash;

    // Decode the buffer.
  pdu_len = decode_hex(pdu, pdu_len-2);

    // Read the SMSC address len
  smsc_len = *safe_consume(&pdu, &pdu_len, 1);

    // Consume the SMSC address
  trash = *safe_consume(&pdu, &pdu_len, smsc_len);

    // Verify the SMS-DELIVER octet (WTF?)
  if (*safe_consume(&pdu, &pdu_len, 1) != 0x04)
    ASSERT(0);

    // Read the sender address len
  addr_len = *safe_consume(&pdu, &pdu_len, 1);

    // Verify the sender addr type
  if (*safe_consume(&pdu, &pdu_len, 1) != 0x91)
    ASSERT(0);

    // Parse the sender addr
  addr_len_in_bytes = (addr_len+1)/2;
    // TODO parse this
  addr = safe_consume(&pdu, &pdu_len, addr_len_in_bytes);
  for (i = 0; i < addr_len_in_bytes; i++) {
    lcdi2c_putc(nibble2hex(addr[i]&0xf));
    lcdi2c_putc(nibble2hex(addr[i]>>4));
  }
  lcdi2c_puts(": ");

    // Verify that the protocol identifier is 'Default store and forward short message'
  if (*safe_consume(&pdu, &pdu_len, 1) != 0x00)
    ASSERT(0);

    // Verify the data encoding scheme (WTF?)
  if (*safe_consume(&pdu, &pdu_len, 1) != 0x00)
    ASSERT(0);

    // Consume the timestamp
    // TODO parse this
  trash = *safe_consume(&pdu, &pdu_len, 7);

    // Read the user data len
  data_len = *safe_consume(&pdu, &pdu_len, 1);
  lcdi2c_putc(nibble2hex(data_len>>4));
  lcdi2c_putc(nibble2hex(data_len&0xf));
  lcdi2c_newline();
      // Verify that the data len is even (UCS-2 encoding)
  if (data_len % 2 != 0)
    ASSERT(0);

    // Read the user data
  addr = safe_consume(&pdu, &pdu_len, data_len);
      // TODO parse this
  for (i = 0; i < data_len; i++) {
    lcdi2c_putc(nibble2hex(addr[i]>>4));
    lcdi2c_putc(nibble2hex(addr[i]&0xf));
    lcdi2c_putc(' ');
  }
  lcdi2c_newline();
}