#include "unity.h"
#include "atc.h"

struct cat_object test_parser;
static int8_t var1;
static char buf[256];

struct cat_command cmds[] = {
    { .name = "+TEST0"},
    { .name = "+TEST1"},
    { .name = "+CMD2" },
    { .name = "+CMD3" },
    { .name = "+CMD4" },
    { .name = "+CMD5" },
    { .name = "#HELP" }
};

// static struct cat_variable vars[] = {
//     {
//         .type = CAT_VAR_INT_DEC,
//         .data = &var1,
//         .data_size = sizeof(var1),
//         .write = var1_write
//     }
// };

static struct cat_command_group cmd_group = {
    .cmd = cmds,
    .cmd_num = sizeof(cmds) / sizeof(cmds[0]),
};

static struct cat_command_group *cmd_desc[] = {
    &cmd_group
};

static struct cat_descriptor desc = {
    .cmd_group = cmd_desc,
    .cmd_group_num = sizeof(cmd_desc) / sizeof(cmd_desc[0]),

    .buf = buf,
    .buf_size = sizeof(buf),
};

void setUp(void) {
    test_parser.desc = &desc;
    test_parser.state = CAT_STATE_IDLE;
    test_parser.count_char_received = 0;
    memset(test_parser.atc_receive, 0, sizeof(test_parser.atc_receive));
    test_parser.commands_num = 0;
    test_parser.index = 0;
    test_parser.length = 0;
    for (int i = 0; i < test_parser.desc->cmd_group_num; i++) {
        test_parser.commands_num += test_parser.desc->cmd_group[i]->cmd_num;
    }
}

void test_cat_service_idle_to_parse_prefix(void) {
    // Test: Nhận ký tự 'A' chuyển từ IDLE -> PARSE_PREFIX
    char input = 'A';
    cat_status status = cat_service_get_fsm(input, &test_parser);
    TEST_ASSERT_EQUAL(CAT_STATE_PARSE_PREFIX, test_parser.state); // Kiểm tra state
    printf("Expected state: %d, Actual state: %d\n", CAT_STATE_PARSE_PREFIX, test_parser.state);
    
    TEST_ASSERT_EQUAL('A', test_parser.atc_receive[0]);           // Kiểm tra ký tự lưu vào buffer
    printf("Expected character: %c, Actual character: %c\n", 'A', test_parser.atc_receive[0]);
    TEST_ASSERT_EQUAL(CAT_STATUS_BUSY, status); 
}

void test_cat_service_error_idle_to_parse_prefix(void) {
    setUp();
    // Test: Nhận ký tự 'A' chuyển từ IDLE -> PARSE_PREFIX
    char input = 'E';
    cat_status status = cat_service_get_fsm(input, &test_parser);
    TEST_ASSERT_NOT_EQUAL(CAT_STATE_PARSE_PREFIX, test_parser.state); // Kiểm tra state
    printf("Expected state: %d, Actual state: %d\n", CAT_STATE_PARSE_PREFIX, test_parser.state);
    
    TEST_ASSERT_NOT_EQUAL('A', test_parser.atc_receive[0]);           // Kiểm tra ký tự lưu vào buffer
    printf("Expected character: %c, Actual character: %c\n", 'A', test_parser.atc_receive[0]);
}

void test_cat_service_idle_to_parse_command(void) {
    setUp();
    // Test: Nhận ký tự 'A' chuyển từ IDLE -> PARSE_PREFIX
    char input_A = 'A';
    char input_T = 'T';
    // Nhận ký tự 'A'
    cat_status status = cat_service_get_fsm(input_A, &test_parser);
    TEST_ASSERT_EQUAL(CAT_STATE_PARSE_PREFIX, test_parser.state); // Kiểm tra state
    printf("After receiving 'A': State: %d, Expected: %d\n", test_parser.state, CAT_STATE_PARSE_PREFIX);
    TEST_ASSERT_EQUAL('A', test_parser.atc_receive[0]);           // Kiểm tra buffer
    printf("Character at buffer[0]: %c, Expected: 'A'\n", test_parser.atc_receive[0]);

    // Nhận ký tự 'T'
    status = cat_service_get_fsm(input_T, &test_parser);
    TEST_ASSERT_EQUAL(CAT_STATE_PARSE_COMMAND_CHAR, test_parser.state); // Kiểm tra state
    printf("After receiving 'T': State: %d, Expected: %d\n", test_parser.state, CAT_STATE_PARSE_COMMAND_CHAR);
    TEST_ASSERT_EQUAL('T', test_parser.atc_receive[1]);           // Kiểm tra buffer
    printf("Character at buffer[1]: %c, Expected: 'T'\n", test_parser.atc_receive[1]);

    // Kiểm tra trạng thái trả về
    TEST_ASSERT_EQUAL(CAT_STATUS_BUSY, status);
}

void test_cat_service_idle_to_parse_command_char(void){
    setUp();
    cat_status status;
    
    const char *input = "AT+TEST?\r\n";
    for (int i = 0; i < strlen(input); i++) {
        status = cat_service_get_fsm(input[i], &test_parser);
        
        printf("After receiving %c : State: %d\n", input[i],test_parser.state);
    }

    TEST_ASSERT_EQUAL(CAT_STATE_SEARCH_COMMAND, test_parser.state); // Kiểm tra state
    TEST_ASSERT_EQUAL(CAT_STATUS_BUSY, status);
}

void test_cat_service_idle_to_parse_command_char_found(void){
    setUp();
    cat_status status;
    const char *input = "AT+CMD3\n";
    for (int i = 0; i < strlen(input); i++) {
        status = cat_service_get_fsm(input[i], &test_parser);
        printf("Received: %c, State: %d, lenght: %ld\n", input[i], test_parser.state, test_parser.length);
        // printf get_atcmd_buf(&test_parser) 
    }    
    if(test_parser.state == CAT_STATE_SEARCH_COMMAND)
    {
        cat_service_search_command(&test_parser);
        // char *fsm_receive = test_parser.atc_receive;
        // while (test_parser.index < test_parser.commands_num) {
        //     // Gọi hàm xử lý lệnh
        //     printf("fsm_receive: %s\n", fsm_receive);
        //     printf("cmd_name: %s - ", cmd_name);
        //     // Kiểm tra trạng thái của lệnh hiện tại
        //     if (cmd_name != NULL) {
        //         printf("lenght %ld - length_cmd %ld\n", test_parser.length, strlen(cmd_name));
        //         } else {
        //         printf("cmd_name is NULL or invalid\n");
        //         }
        //     status = update_command(&test_parser);
        //     if (get_cmd_state(&test_parser, test_parser.index) == CAT_CMD_STATE_FULL_MATCH) {
        //         printf("math *** cmd_name: %s", cmd_name);
        //         test_parser.state = CAT_STATE_COMMAND_FOUND;
        //         // return CAT_STATUS_OK; // Đã tìm thấy lệnh khớp
        //     }
        //     printf("Index :%ld - stt_flag_state_match %d\n\n",test_parser.index, stt_flag_state_match);
        //     test_parser.index++;
        // }
    }
        // status = cat_service_search_command(&test_parser);
        // printf("cmd_name: %s", cmd_name);
        // printf("Received: %c, State: %ld, Index: %d\n", test_parser.state,  test_parser.index);
        // printf("test_prase lenght %ld, command_num %ld\n", test_parser.length, test_parser.commands_num);
        // printf("Index :%ld - stt_flag_state_match %d\n\n",test_parser.index, stt_flag_state_match);
    TEST_ASSERT_EQUAL(7, test_parser.index); // Kiểm tra state
    
    TEST_ASSERT_EQUAL(CAT_STATE_COMMAND_FOUND, test_parser.state); // Kiểm tra state
    TEST_ASSERT_EQUAL(CAT_STATUS_BUSY, status); // Hàm phải trả về CAT_STATUS_BUSY
}

// TEST_ASSERT_EQUAL(CAT_STATE_SEARCH_COMMAND, test_parser.state); // Trạng thái phải là "COMMAND_FOUND"

    // printf("cmd_list num %ld",test_parser.desc->cmd_list_num);
    // printf("commands_num %ld",test_parser.commands_num );
    // for(int i = 0; i < test_parser.desc->cmd_list_num ; i++){
    //     if (test_parser.desc->cmd_list[i].name != NULL) {
    //         printf("cmd: %s\n", test_parser.desc->cmd_list[i].name);
    //     } else {
    //         printf("No command found.\n");
    //     }
    // }
    // printf("partial_cntr %ld\n", test_parser.partial_cntr);