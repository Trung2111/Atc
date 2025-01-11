#include "src/atc.h"
#include "/var/lib/gems/3.0.0/gems/ceedling-0.31.1/vendor/unity/src/unity.h"


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

        if (test_parser.desc->cmd_list[i].name != 

                                                 ((void *)0)

                                                     ) {

            test_parser.commands_num++;

        }

    }

}



void test_cat_service_idle_to_parse_prefix(void) {



    char input = 'A';

    cat_status status = cat_service(input, &test_parser);

    UnityAssertEqualNumber((UNITY_INT)((CAT_STATE_PARSE_PREFIX)), (UNITY_INT)((test_parser.state)), (

   ((void *)0)

   ), (UNITY_UINT)(40), UNITY_DISPLAY_STYLE_INT);

    printf("Expected state: %d, Actual state: %d\n", CAT_STATE_PARSE_PREFIX, test_parser.state);



    UnityAssertEqualNumber((UNITY_INT)(('A')), (UNITY_INT)((test_parser.atc_receive[0])), (

   ((void *)0)

   ), (UNITY_UINT)(43), UNITY_DISPLAY_STYLE_INT);

    printf("Expected character: %c, Actual character: %c\n", 'A', test_parser.atc_receive[0]);

    UnityAssertEqualNumber((UNITY_INT)((CAT_STATUS_BUSY)), (UNITY_INT)((status)), (

   ((void *)0)

   ), (UNITY_UINT)(45), UNITY_DISPLAY_STYLE_INT);

}



void test_cat_service_error_idle_to_parse_prefix(void) {

    setUp();



    char input = 'E';

    cat_status status = cat_service(input, &test_parser);

    do {if (((CAT_STATE_PARSE_PREFIX) != (test_parser.state))) {} else {UnityFail( ((" Expected Not-Equal")), (UNITY_UINT)((UNITY_UINT)(53)));}} while(0);

    printf("Expected state: %d, Actual state: %d\n", CAT_STATE_PARSE_PREFIX, test_parser.state);



    do {if ((('A') != (test_parser.atc_receive[0]))) {} else {UnityFail( ((" Expected Not-Equal")), (UNITY_UINT)((UNITY_UINT)(56)));}} while(0);

    printf("Expected character: %c, Actual character: %c\n", 'A', test_parser.atc_receive[0]);

}



void test_cat_service_idle_to_parse_command(void) {

    setUp();



    char input_A = 'A';

    char input_T = 'T';



    cat_status status = cat_service(input_A, &test_parser);

    UnityAssertEqualNumber((UNITY_INT)((CAT_STATE_PARSE_PREFIX)), (UNITY_INT)((test_parser.state)), (

   ((void *)0)

   ), (UNITY_UINT)(67), UNITY_DISPLAY_STYLE_INT);

    printf("After receiving 'A': State: %d, Expected: %d\n", test_parser.state, CAT_STATE_PARSE_PREFIX);

    UnityAssertEqualNumber((UNITY_INT)(('A')), (UNITY_INT)((test_parser.atc_receive[0])), (

   ((void *)0)

   ), (UNITY_UINT)(69), UNITY_DISPLAY_STYLE_INT);

    printf("Character at buffer[0]: %c, Expected: 'A'\n", test_parser.atc_receive[0]);





    status = cat_service(input_T, &test_parser);

    UnityAssertEqualNumber((UNITY_INT)((CAT_STATE_PARSE_COMMAND_CHAR)), (UNITY_INT)((test_parser.state)), (

   ((void *)0)

   ), (UNITY_UINT)(74), UNITY_DISPLAY_STYLE_INT);

    printf("After receiving 'T': State: %d, Expected: %d\n", test_parser.state, CAT_STATE_PARSE_COMMAND_CHAR);

    UnityAssertEqualNumber((UNITY_INT)(('T')), (UNITY_INT)((test_parser.atc_receive[1])), (

   ((void *)0)

   ), (UNITY_UINT)(76), UNITY_DISPLAY_STYLE_INT);

    printf("Character at buffer[1]: %c, Expected: 'T'\n", test_parser.atc_receive[1]);





    UnityAssertEqualNumber((UNITY_INT)((CAT_STATUS_BUSY)), (UNITY_INT)((status)), (

   ((void *)0)

   ), (UNITY_UINT)(80), UNITY_DISPLAY_STYLE_INT);

}



void test_cat_service_idle_to_parse_command_char(void){

    setUp();

    cat_status status;



    const char *input = "AT+TEST?\r\n";

    for (int i = 0; i < strlen(input); i++) {

        status = cat_service(input[i], &test_parser);



        printf("After receiving %c : State: %d\n", input[i],test_parser.state);

   }



    UnityAssertEqualNumber((UNITY_INT)((CAT_STATE_SEARCH_COMMAND)), (UNITY_INT)((test_parser.state)), (

   ((void *)0)

   ), (UNITY_UINT)(94), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((CAT_STATUS_BUSY)), (UNITY_INT)((status)), (

   ((void *)0)

   ), (UNITY_UINT)(95), UNITY_DISPLAY_STYLE_INT);

}



void test_cat_service_idle_to_parse_command_char_found(void){

    setUp();

    cat_status status;

    const char *input = "AT+CMD5=123\n";

    for (int i = 0; i < strlen(input); i++) {

        status = cat_service(input[i], &test_parser);

        printf("Received: %c, State: %d, Index: %ld\n", input[i], test_parser.state, test_parser.index);

        printf("index %ld\n", test_parser.index);

        printf("cmd_state %d\n", cmd_state);



    }

    printf("cmd_list num %ld",test_parser.desc->cmd_list_num);

    printf("commands_num %ld",test_parser.commands_num );

    for(int i = 0; i < test_parser.desc->cmd_list_num ; i++){

        if (test_parser.desc->cmd_list[i].name != 

                                                 ((void *)0)

                                                     ) {

            printf("cmd: %s\n", test_parser.desc->cmd_list[i].name);

        } else {

            printf("No command found.\n");

        }

    }

    printf("partial_cntr %ld\n", test_parser.partial_cntr);



    UnityAssertEqualNumber((UNITY_INT)((CAT_STATUS_BUSY)), (UNITY_INT)((status)), (

   ((void *)0)

   ), (UNITY_UINT)(120), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((CAT_STATE_COMMAND_FOUND)), (UNITY_INT)((test_parser.state)), (

   ((void *)0)

   ), (UNITY_UINT)(121), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((1)), (UNITY_INT)((test_parser.partial_cntr)), (

   ((void *)0)

   ), (UNITY_UINT)(122), UNITY_DISPLAY_STYLE_INT);

}
