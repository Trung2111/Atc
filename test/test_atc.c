#include "unity.h"
#include "atc.h"

static struct cat_object test_parser;
static char buf[256];
struct cat_command cmds[] = {
    { .name = "AT+TEST?\n"},
    { .name = "AT+CMD2=123\n" },
    { .name = "AT+CMD3=123\n" },
    { .name = "AT+CMD4=123\n" },
    { .name = "AT+CMD5=123\n" },
    { .name = "AT+CMD6=123\n" }
};

static struct cat_descriptor desc = {
    .cmd_list = cmds,
    .cmd_list_num = sizeof(cmds) / sizeof(cmds[0]),
    .buf = buf,
    .buf_size = sizeof(buf),
};

void setUp(void) {
    test_parser.desc = &desc;
    test_parser.state = CAT_STATE_IDLE;
    test_parser.count_char_received = 0;
    memset(test_parser.atc_receive, 0, sizeof(test_parser.atc_receive));
    test_parser.commands_num = 0;

    for(int i = 0; i < test_parser.desc->cmd_list_num; i++){
        if (test_parser.desc->cmd_list[i].name != NULL) {
            test_parser.commands_num++;
        }
    }
}

void test_cat_service_idle_to_parse_prefix(void) {
    // Test: Nhận ký tự 'A' chuyển từ IDLE -> PARSE_PREFIX
    char input = 'A';
    cat_status status = cat_service(input, &test_parser);
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
    cat_status status = cat_service(input, &test_parser);
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
    cat_status status = cat_service(input_A, &test_parser);
    TEST_ASSERT_EQUAL(CAT_STATE_PARSE_PREFIX, test_parser.state); // Kiểm tra state
    printf("After receiving 'A': State: %d, Expected: %d\n", test_parser.state, CAT_STATE_PARSE_PREFIX);
    TEST_ASSERT_EQUAL('A', test_parser.atc_receive[0]);           // Kiểm tra buffer
    printf("Character at buffer[0]: %c, Expected: 'A'\n", test_parser.atc_receive[0]);

    // Nhận ký tự 'T'
    status = cat_service(input_T, &test_parser);
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
        status = cat_service(input[i], &test_parser);
        
        printf("After receiving %c : State: %d\n", input[i],test_parser.state);
   }

    TEST_ASSERT_EQUAL(CAT_STATE_SEARCH_COMMAND, test_parser.state); // Kiểm tra state
    TEST_ASSERT_EQUAL(CAT_STATUS_BUSY, status);
}

void test_cat_service_idle_to_parse_command_char_found(void){
    setUp();
    cat_status status;
    const char *input = "AT+CMD5=123\n";
    for (int i = 0; i < strlen(input); i++) {
        status = cat_service(input[i], &test_parser);
        printf("Received: %c, State: %d, Index: %ld\n", input[i], test_parser.state,  test_parser.index);
        printf("index %ld\n", test_parser.index);
        printf("cmd_state %d\n", cmd_state);
        // printf get_atcmd_buf(&test_parser) 
    }    
    printf("cmd_list num %ld",test_parser.desc->cmd_list_num);
    printf("commands_num %ld",test_parser.commands_num );
    for(int i = 0; i < test_parser.desc->cmd_list_num ; i++){
        if (test_parser.desc->cmd_list[i].name != NULL) {
            printf("cmd: %s\n", test_parser.desc->cmd_list[i].name);
        } else {
            printf("No command found.\n");
        }
    }
    printf("partial_cntr %ld\n", test_parser.partial_cntr);

    TEST_ASSERT_EQUAL(CAT_STATUS_BUSY, status); // Hàm phải trả về CAT_STATUS_BUSY
    TEST_ASSERT_EQUAL(CAT_STATE_COMMAND_FOUND, test_parser.state); // Trạng thái phải là "COMMAND_FOUND"
    TEST_ASSERT_EQUAL(1, test_parser.partial_cntr);  // Số lệnh khớp một phần là 1
}

 // printf("Desc buf address: %hhn, size: %ld\n", test_parser.desc->buf, sizeof(test_parser.desc->buf));

            // printf("index %ld\n", test_parser.index);

// for (int i = 0; i < test_parser.commands_num; i++) {
    //     printf("Buffer state [%d]: %d\n", i, get_cmd_state(&test_parser, i));
    // }
    // printf("Index: %ld, Cmd state: %d\n", test_parser.index, cmd_state);