#include <msp430.h>
#include <string.h>
#include "misc.h"
#include "utils.h"
#include "main.h"
#include "softuart.h"
#include "uart.h"
#include "sim900.h"


/*
 Global last response and response len;
*/
#define MAX_RESPONSE_SIZE 64
char       response[MAX_RESPONSE_SIZE];
uint8_t    response_len;


int sim900_cmd(char* cmd) {
  uint16_t i;
  char c;
  byte look_for_second=0, done=0;
  char* response_start = response;

  TRACE("Executing cmd '%s'", cmd);
  // Write the command
  softuart_puts(cmd);
  softuart_puts("\r\n");
  
  // Read & throw the echoed chars ->
  // command length
  // '\r\n' of our own command
  // '\r\n' of the reply
  i = strlen(cmd) + 4;
  while (i) {
    if (softuart_getc(&c)) { 
      i--;
    }

  }
  
  // Read the reply until hitting '\r\n' or until reaching *response_len
  while (!done) {
    while (softuart_getc(response+response_len)) {
      // We have read a character.
      response_len++;
      //TRACE("response len: %u", response_len);

      // If we need to check for the '\n' 
      if (look_for_second && (response[response_len-1] =='\n') ) {
        //TRACE("hit n");
        done = 1;
        break;
      // In this case we either didn't hit the '\r' yet, or we did but the current char is NOT '\n'
      } else {
        // Check if its a '\r'
        if (response[response_len-1] == '\r') {
          //TRACE("hit r");
          look_for_second = 1;
        } else {
          look_for_second = 0;
        }
      }

      // Do we overflow?
      if (response_len == MAX_RESPONSE_SIZE) {
        done = 1;
        break;
      }

    }

    /*
    We get here if:
    1) We are waiting for reply (or currently reading it) and didn't hit the '\r\n', but no data available on serial.
    2) We are dpme in a bad way (still need to read data but first clear the response buffer)
    3) We are done in a good way (we have a valid response)
    */
  }
  
  response[response_len] = '\x00';
  if (response_len == MAX_RESPONSE_SIZE) {
    
    TRACE("Response buffer full: %s", response);
    return 1;
  }

  TRACE("Reply: %s", response);  
  return 0;


}

int sim900_clear_response_buf() {
  response_len = 0;
  memset(response, 0, MAX_RESPONSE_SIZE+1);
}

int sim900_cmd_and_verify(char *cmd, char *expected_response) {
  int result = 0;
  sim900_cmd(cmd);
  result = strcmp(response, expected_response);
  if (result) {
    TRACE("Problem executing '%s'", cmd);
    TRACE("Expected: '%s'", expected_response);
    TRACE("Got: '%s'", response);
  } else {
    //TRACE("Verified cmd :)");
  }
  sim900_clear_response_buf();
  return result;

}

int sim900_init(void){
  char c;
  // Empty softuart queue....
  while(softuart_getc(&c));

  // Sync uart...
  sim900_cmd_and_verify("ATH", "OK\r\n");
  // if (sim900_cmd("ATH")) { TRACE("ERROR ATH"); }
  // sim900_clear_response_buf();
  ms_sleep(1000);

  // Switch to smart auto clock (go to sleep if nothing happens.)
  // if (sim900_cmd("AT+CSCLK=2")) { TRACE("ERROR AT+CSCLK=2"); }
  // sim900_clear_response_buf();
  sim900_cmd_and_verify("AT+CSCLK=2", "OK\r\n");
  ms_sleep(1000);

  sim900_cmd_and_verify("AT+CLIP=1", "OK\r\n");
  // if (sim900_cmd("AT+CLIP=1")) { TRACE("ERROR AT+CLIP=1"); }
  // sim900_clear_response_buf();
  ms_sleep(1000);

}
