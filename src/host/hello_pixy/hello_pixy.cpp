//
// begin license header
//
// This file is part of Pixy CMUcam5 or "Pixy" for short
//
// All Pixy source code is provided under the terms of the
// GNU General Public License v2 (http://www.gnu.org/licenses/gpl-2.0.html).
// Those wishing to use Pixy source code, software and/or
// technologies under different licensing terms should contact us at
// cmucam@cs.cmu.edu. Such licensing terms are available for
// all portions of the Pixy codebase presented here.
//
// end license header
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "pixy.h"

#define PIXY_ID_A            0
#define PIXY_ID_B            1
#define BLOCK_BUFFER_SIZE    25

// Pixy Block buffer // 
struct Block blocks_A[BLOCK_BUFFER_SIZE];
struct Block blocks_B[BLOCK_BUFFER_SIZE];

static bool run_flag = true;

void handle_SIGINT(int unused)
{
  // On CTRL+C - abort! //

  run_flag = false;
}

int main(int argc, char * argv[])
{
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  int      i_A = 0;
  int      i_B = 0;
  int      index_A;
  int      index_B;
  int      blocks_copied_A;
  int      blocks_copied_B;
  int      pixy_init_status_A;
  int      pixy_init_status_B;
  char     buf_A[128];
  char     buf_B[128];

  // Catch CTRL+C (SIGINT) signals //
  signal(SIGINT, handle_SIGINT);

  fprintf(stderr, "Hello Pixy:\n libpixyusb Version: %s\n", __LIBPIXY_VERSION__);

  // Connect to Pixy //

  // Was there an error initializing pixy? //
  pixy_init_status_A = pixy_init(PIXY_ID_A);
  if(pixy_init_status_A != 0)
  {
    // Error initializing Pixy //
    fprintf(stderr, "pixy_init(): ");
    pixy_error(PIXY_ID_A, pixy_init_status_A);

    return pixy_init_status_A;
  }

  pixy_init_status_B = pixy_init(PIXY_ID_B);
  if(pixy_init_status_B != 0)
  {
    // Error initializing Pixy //
    fprintf(stderr, "pixy_init(): ");
    pixy_error(PIXY_ID_B, pixy_init_status_B);

    return pixy_init_status_B;
  }

  // Request Pixy firmware version Camera A //
  {
    uint16_t major;
    uint16_t minor;
    uint16_t build;
    int      return_value;

    return_value = pixy_get_firmware_version(PIXY_ID_A, &major, &minor, &build);

    if (return_value) {
      // Error //
      fprintf(stderr, "Failed to retrieve Pixy firmware version. ");
      pixy_error(PIXY_ID_A, return_value);

      return return_value;
    } else {
      // Success //
      fprintf(stderr, " Pixy Firmware Version: %d.%d.%d\n", major, minor, build);
    }
  }

  // Request Pixy firmware version for Camera B //
  {
    uint16_t major;
    uint16_t minor;
    uint16_t build;
    int      return_value;

    return_value = pixy_get_firmware_version(PIXY_ID_B, &major, &minor, &build);

    if (return_value) {
      // Error //
      fprintf(stderr, "Failed to retrieve Pixy firmware version. ");
      pixy_error(PIXY_ID_B, return_value);

      return return_value;
    } else {
      // Success //
      fprintf(stderr, " Pixy Firmware Version: %d.%d.%d\n", major, minor, build);
    }
  }

#if 0
  // Pixy Command Examples //
  {
    int32_t response;
    int     return_value;

    // Execute remote procedure call "cam_setAWB" with one output (host->pixy) parameter (Value = 1)
    //
    //   Parameters:                 Notes:
    //
    //   pixy_command("cam_setAWB",  String identifier for remote procedure
    //                        0x01,  Length (in bytes) of first output parameter
    //                           1,  Value of first output parameter
    //                           0,  Parameter list seperator token (See value of: END_OUT_ARGS)
    //                   &response,  Pointer to memory address for return value from remote procedure call
    //                           0); Parameter list seperator token (See value of: END_IN_ARGS)
    //

    // Enable auto white balance //
    pixy_command("cam_setAWB", UINT8(0x01), END_OUT_ARGS,  &response, END_IN_ARGS);

    // Execute remote procedure call "cam_getAWB" with no output (host->pixy) parameters
    //
    //   Parameters:                 Notes:
    //
    //   pixy_command("cam_setAWB",  String identifier for remote procedure
    //                           0,  Parameter list seperator token (See value of: END_OUT_ARGS)
    //                   &response,  Pointer to memory address for return value from remote procedure call
    //                           0); Parameter list seperator token (See value of: END_IN_ARGS)
    //

    // Get auto white balance //
    return_value = pixy_command("cam_getAWB", END_OUT_ARGS, &response, END_IN_ARGS);

    // Set auto white balance back to disabled //
    pixy_command("cam_setAWB", UINT8(0x00), END_OUT_ARGS,  &response, END_IN_ARGS);
  }
#endif

  fprintf(stderr, "Detecting blocks...\n");
  while(run_flag)
  {
    // Wait for new blocks to be available //
    while(!pixy_blocks_are_new(PIXY_ID_A) && run_flag); 
    while(!pixy_blocks_are_new(PIXY_ID_B) && run_flag); 

    // Get blocks from Pixy //
    blocks_copied_A = pixy_get_blocks(PIXY_ID_A, BLOCK_BUFFER_SIZE, &blocks_A[0]);
    blocks_copied_B = pixy_get_blocks(PIXY_ID_B, BLOCK_BUFFER_SIZE, &blocks_B[0]);

    if(blocks_copied_A < 0) {
      // Error: pixy_get_blocks //
      fprintf(stderr, "pixy_get_blocks(): ");
      pixy_error(PIXY_ID_A, blocks_copied_A);
    }

    if(blocks_copied_B < 0) {
      // Error: pixy_get_blocks //
      fprintf(stderr, "pixy_get_blocks(): ");
      pixy_error(PIXY_ID_B, blocks_copied_B);
    }

    // Display received blocks //
    fprintf(stderr, "Camera A frame %d:\n", i_A);
    for(index_A = 0; index_A != blocks_copied_A; ++index_A) {    
       blocks_A[index_A].print(buf_A);
       fprintf(stderr, "  %s\n", buf_A);
    }

    // Display received blocks //
    fprintf(stderr, "Camera B frame %d:\n", i_B);
    for(index_B = 0; index_B != blocks_copied_B; ++index_B) {    
       blocks_B[index_B].print(buf_B);
       fprintf(stderr, "  %s\n", buf_B);
    }

    i_A++;
    i_B++;
  }

  pixy_close(PIXY_ID_A);
  pixy_close(PIXY_ID_B);
}
