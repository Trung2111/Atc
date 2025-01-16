#include "src/atc.h"
#include "/var/lib/gems/3.0.0/gems/ceedling-0.31.1/vendor/unity/src/unity.h"


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



    char input = 'A';

    cat_status status = cat_service_get_fsm(input, &test_parser);

    UnityAssertEqualNumber((UNITY_INT)((CAT_STATE_PARSE_PREFIX)), (UNITY_INT)((test_parser.state)), (

   ((void *)0)

   ), (UNITY_UINT)(61), UNITY_DISPLAY_STYLE_INT);

    printf("Expected state: %d, Actual state: %d\n", CAT_STATE_PARSE_PREFIX, test_parser.state);



    UnityAssertEqualNumber((UNITY_INT)(('A')), (UNITY_INT)((test_parser.atc_receive[0])), (

   ((void *)0)

   ), (UNITY_UINT)(64), UNITY_DISPLAY_STYLE_INT);

    printf("Expected character: %c, Actual character: %c\n", 'A', test_parser.atc_receive[0]);

    UnityAssertEqualNumber((UNITY_INT)((CAT_STATUS_BUSY)), (UNITY_INT)((status)), (

   ((void *)0)

   ), (UNITY_UINT)(66), UNITY_DISPLAY_STYLE_INT);

}



void test_cat_service_error_idle_to_parse_prefix(void) {

    setUp();



    char input = 'E';

    cat_status status = cat_service_get_fsm(input, &test_parser);

    do {if (((CAT_STATE_PARSE_PREFIX) != (test_parser.state))) {} else {UnityFail( ((" Expected Not-Equal")), (UNITY_UINT)((UNITY_UINT)(74)));}} while(0);

    printf("Expected state: %d, Actual state: %d\n", CAT_STATE_PARSE_PREFIX, test_parser.state);



    do {if ((('A') != (test_parser.atc_receive[0]))) {} else {UnityFail( ((" Expected Not-Equal")), (UNITY_UINT)((UNITY_UINT)(77)));}} while(0);

    printf("Expected character: %c, Actual character: %c\n", 'A', test_parser.atc_receive[0]);

}



void test_cat_service_idle_to_parse_command(void) {

    setUp();



    char input_A = 'A';

    char input_T = 'T';



    cat_status status = cat_service_get_fsm(input_A, &test_parser);

    UnityAssertEqualNumber((UNITY_INT)((CAT_STATE_PARSE_PREFIX)), (UNITY_INT)((test_parser.state)), (

   ((void *)0)

   ), (UNITY_UINT)(88), UNITY_DISPLAY_STYLE_INT);

    printf("After receiving 'A': State: %d, Expected: %d\n", test_parser.state, CAT_STATE_PARSE_PREFIX);

    UnityAssertEqualNumber((UNITY_INT)(('A')), (UNITY_INT)((test_parser.atc_receive[0])), (

   ((void *)0)

   ), (UNITY_UINT)(90), UNITY_DISPLAY_STYLE_INT);

    printf("Character at buffer[0]: %c, Expected: 'A'\n", test_parser.atc_receive[0]);





    status = cat_service_get_fsm(input_T, &test_parser);

    UnityAssertEqualNumber((UNITY_INT)((CAT_STATE_PARSE_COMMAND_CHAR)), (UNITY_INT)((test_parser.state)), (

   ((void *)0)

   ), (UNITY_UINT)(95), UNITY_DISPLAY_STYLE_INT);

    printf("After receiving 'T': State: %d, Expected: %d\n", test_parser.state, CAT_STATE_PARSE_COMMAND_CHAR);

    UnityAssertEqualNumber((UNITY_INT)(('T')), (UNITY_INT)((test_parser.atc_receive[1])), (

   ((void *)0)

   ), (UNITY_UINT)(97), UNITY_DISPLAY_STYLE_INT);

    printf("Character at buffer[1]: %c, Expected: 'T'\n", test_parser.atc_receive[1]);





    UnityAssertEqualNumber((UNITY_INT)((CAT_STATUS_BUSY)), (UNITY_INT)((status)), (

   ((void *)0)

   ), (UNITY_UINT)(101), UNITY_DISPLAY_STYLE_INT);

}



void test_cat_service_idle_to_parse_command_char(void){

    setUp();

    cat_status status;



    const char *input = "AT+TEST?\r\n";

    for (int i = 0; i < strlen(input); i++) {

        status = cat_service_get_fsm(input[i], &test_parser);



        printf("After receiving %c : State: %d\n", input[i],test_parser.state);

    }



    UnityAssertEqualNumber((UNITY_INT)((CAT_STATE_SEARCH_COMMAND)), (UNITY_INT)((test_parser.state)), (

   ((void *)0)

   ), (UNITY_UINT)(115), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((CAT_STATUS_BUSY)), (UNITY_INT)((status)), (

   ((void *)0)

   ), (UNITY_UINT)(116), UNITY_DISPLAY_STYLE_INT);

}



void test_cat_service_idle_to_parse_command_char_found(void){

    setUp();

    cat_status status;

    const char *input = "AT+CMD3\n";

    for (int i = 0; i < strlen(input); i++) {

        status = cat_service_get_fsm(input[i], &test_parser);

        printf("Received: %c, State: %d, lenght: %ld\n", input[i], test_parser.state, test_parser.length);



    }

    if(test_parser.state == CAT_STATE_SEARCH_COMMAND)

    {

        cat_service_search_command(&test_parser);

    }











    UnityAssertEqualNumber((UNITY_INT)((7)), (UNITY_INT)((test_parser.index)), (

   ((void *)0)

   ), (UNITY_UINT)(157), UNITY_DISPLAY_STYLE_INT);



    UnityAssertEqualNumber((UNITY_INT)((CAT_STATE_COMMAND_FOUND)), (UNITY_INT)((test_parser.state)), (

   ((void *)0)

   ), (UNITY_UINT)(159), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((CAT_STATUS_BUSY)), (UNITY_INT)((status)), (

   ((void *)0)

   ), (UNITY_UINT)(160), UNITY_DISPLAY_STYLE_INT);

}
