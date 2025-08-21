#ifndef GAMELINK_H
#define GAMELINK_H

#include <gba.h>

// Game Link command constants
#define GAMELINK_CMD_BUFFER_SIZE 16
#define GAMELINK_NO_DATA         0xFF

// Serial control bits
#define SIO_8BIT        0x0000
#define SIO_RECV_ONLY   0x0000
#define SIO_ENABLE      0x0080
#define SIO_IRQ_ENABLE  0x4000

// Game Link Cable state
extern u8 gamelink_cmd_buffer[GAMELINK_CMD_BUFFER_SIZE];
extern u8 gamelink_cmd_head;
extern u8 gamelink_cmd_tail;
extern bool gamelink_enabled;

// Function declarations
void gamelink_init(void);
void gamelink_poll(void);
void gamelink_process_command(u8 command);
bool gamelink_has_command(void);
u8 gamelink_get_command(void);

#endif // GAMELINK_H