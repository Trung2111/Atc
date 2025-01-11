#ifndef ATC_H
#define ATC_H


#ifdef __cplusplus
extern "C"
{
#endif


/************************************************************************************************************
**************    Include Headers
************************************************************************************************************/

// #include "usart.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/************************************************************************************************************
**************    Public Definitions
************************************************************************************************************/

#define CAT_CMD_STATE_NOT_MATCH (0)
#define CAT_CMD_STATE_PARTIAL_MATCH (1U)
#define CAT_CMD_STATE_FULL_MATCH (2U)

/************************************************************************************************************
**************    Public struct/enum
************************************************************************************************************/

/* enum type with function status */
typedef enum {
    CAT_STATUS_ERROR = -1,
    CAT_STATUS_OK = 0,
    CAT_STATUS_BUSY = 1,
    CAT_STATUS_HOLD = 2
} cat_status;

/* enum type with main at parser fsm state */
typedef enum {
    CAT_STATE_ERROR = -1,
    CAT_STATE_IDLE,
    CAT_STATE_PARSE_PREFIX,
    CAT_STATE_PARSE_COMMAND_CHAR,
    CAT_STATE_UPDATE_COMMAND_STATE,
    CAT_STATE_WAIT_READ_ACKNOWLEDGE,
    CAT_STATE_SEARCH_COMMAND,
    CAT_STATE_COMMAND_FOUND,
    CAT_STATE_COMMAND_NOT_FOUND,
    CAT_STATE_PARSE_COMMAND_ARGS,
    CAT_STATE_PARSE_WRITE_ARGS,
    CAT_STATE_FORMAT_READ_ARGS,
    CAT_STATE_WAIT_TEST_ACKNOWLEDGE,
    CAT_STATE_FORMAT_TEST_ARGS,
    CAT_STATE_WRITE_LOOP,
    CAT_STATE_READ_LOOP,
    CAT_STATE_TEST_LOOP,
    CAT_STATE_RUN_LOOP,
    CAT_STATE_HOLD,
    CAT_STATE_FLUSH_IO_WRITE_WAIT,
    CAT_STATE_FLUSH_IO_WRITE,
    CAT_STATE_AFTER_FLUSH_RESET,
    CAT_STATE_AFTER_FLUSH_OK,
    CAT_STATE_AFTER_FLUSH_FORMAT_READ_ARGS,
    CAT_STATE_AFTER_FLUSH_FORMAT_TEST_ARGS,
    CAT_STATE_PRINT_CMD,
} cat_state;

// /* strcuture with unsolicited command buffered infos */
// struct cat_unsolicited_cmd {
//     struct cat_command const *cmd; /* pointer to commands used to unsolicited event */
//     cat_cmd_type type; /* type of unsolicited event */
// };

/* enum type with fsm type */
typedef enum {
    CAT_FSM_TYPE_ATCMD,
    CAT_FSM_TYPE_UNSOLICITED,
    CAT_FSM_TYPE__TOTAL_NUM,
} cat_fsm_type;



/* enum type with unsolicited events fsm state */
typedef enum {
        CAT_UNSOLICITED_STATE_IDLE,
        CAT_UNSOLICITED_STATE_FORMAT_READ_ARGS,
        CAT_UNSOLICITED_STATE_FORMAT_TEST_ARGS,
        CAT_UNSOLICITED_STATE_READ_LOOP,
        CAT_UNSOLICITED_STATE_TEST_LOOP,
        CAT_UNSOLICITED_STATE_FLUSH_IO_WRITE_WAIT,
        CAT_UNSOLICITED_STATE_FLUSH_IO_WRITE,
        CAT_UNSOLICITED_STATE_AFTER_FLUSH_RESET,
        CAT_UNSOLICITED_STATE_AFTER_FLUSH_OK,
        CAT_UNSOLICITED_STATE_AFTER_FLUSH_FORMAT_READ_ARGS,
        CAT_UNSOLICITED_STATE_AFTER_FLUSH_FORMAT_TEST_ARGS,
} cat_unsolicited_state;

/* structure with at command descriptor */
struct cat_command {
    const char *name; /* at command name (case-insensitivity) */
    const char *description; /* at command description (optionally - can be null) */

    cat_cmd_write_handler write; /* write command handler */
    cat_cmd_read_handler read; /* read command handler */
    cat_cmd_run_handler run; /* run command handler */
    cat_cmd_test_handler test; /* test command handler */

    struct cat_variable const *var; /* pointer to array of variables assiocated with this command */
    size_t var_num; /* number of variables in array */

    bool need_all_vars; /* flag to need all vars parsing */
    bool only_test; /* flag to disable read/write/run commands (only test auto description) */
    bool disable; /* flag to completely disable command */
    bool implicit_write; /* flag to mark command as implicit write */
};

struct cat_command_group {
    const char *name; /* command group name (optional, for identification purpose) */

    struct cat_command const *cmd; /* pointer to array of commands descriptor */
    size_t cmd_num; /* number of commands in array */

    bool disable; /* flag to completely disable all commands in group */
};

/* structure with at command parser descriptor */
struct cat_descriptor {
    struct cat_command_group* const *cmd_group; /* pointer to array of commands group descriptor */
    size_t cmd_group_num; /* number of commands group in array */

    uint8_t *buf; /* pointer to working buffer (used to parse command argument) */
    size_t buf_size; /* working buffer length */

    /* optional unsolicited buffer, if not configured (NULL) */
    /* then the buf will be divided into two smaller buffers */
    uint8_t *unsolicited_buf; /* pointer to unsolicited working buffer (used to parse command argument) */
    size_t unsolicited_buf_size; /* unsolicited working buffer length */
};

/* strcuture with unsolicited command buffered infos */
struct cat_unsolicited_cmd {
    struct cat_command const *cmd; /* pointer to commands used to unsolicited event */
    cat_cmd_type type; /* type of unsolicited event */
};

/* enum type with type of command request */
typedef enum {
    CAT_CMD_TYPE_NONE = -1,
    CAT_CMD_TYPE_RUN,
    CAT_CMD_TYPE_READ,
    CAT_CMD_TYPE_WRITE,
    CAT_CMD_TYPE_TEST,
    CAT_CMD_TYPE__TOTAL_NUM
} cat_cmd_type;

/* structure with io interface functions */
struct cat_io_interface {
    int (*write)(char ch); /* write char to output stream. return 1 if byte wrote successfully. */
    int (*read)(char *ch); /* read char from input stream. return 1 if byte read successfully. */
};

/* structure with main at command parser object */
struct cat_object {
    struct cat_descriptor const *desc; /* pointer to at command parser descriptor */
    struct cat_io_interface const *io; /* pointer to at command parser io interface */
    char atc_receive[128];
    uint8_t count_char_received;
    size_t index; /* index used to iterate over commands and variables */
    size_t partial_cntr; /* partial match commands counter */
    size_t length; /* length of input command name and command arguments */
    // size_t position; /* position of actually parsed char in arguments string */
    // size_t write_size; /* size of parsed buffer hex or buffer string */
    size_t commands_num; /* computed total number of registered commands */

    struct cat_command const *cmd; /* pointer to current command descriptor */
    struct cat_variable const *var; /* pointer to current variable descriptor */
    cat_cmd_type cmd_type; /* type of command request */

    char current_char; /* current received char from input stream */
    cat_state state; /* current fsm state */
    bool cr_flag; /* flag for detect <cr> char in input string */
    // bool hold_state_flag; /* status of hold state (independent from fsm states) */
    // int hold_exit_status; /* hold exit parameter with status */
    // char const *write_buf; /* working buffer pointer used for asynch writing to io */
    // int write_state; /* before, data, after flush io write state */
    // cat_state write_state_after; /* parser state to set after flush io write */
    // bool implicit_write_flag; /* flag that implicit write was detected */

    // struct cat_unsolicited_fsm unsolicited_fsm;
};

/************************************************************************************************************
**************    Public Functions
************************************************************************************************************/
extern uint8_t cmd_state;
cat_status  cat_service(char array_in ,struct cat_object *self);
// void cat_init(struct cat_object *self, const struct cat_descriptor *desc, const struct cat_io_interface *io);
// struct cat_command const* get_command_by_index(struct cat_object *self, size_t index);
// uint8_t  get_cmd_state(struct cat_object *self, size_t i);
// char* get_atcmd_buf(struct cat_object *self);
#ifdef __cplusplus
}
#endif

#endif // ATC_H