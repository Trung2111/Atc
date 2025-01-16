#include <string.h>
#include <stdio.h>

/************************************************************************************************************
**************    Include Headers
************************************************************************************************************/
#include "atc.h"
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

/************************************************************************************************************
**************    Private Definitions
************************************************************************************************************/

// none

/************************************************************************************************************
**************    Private Variables
************************************************************************************************************/

// none
uint8_t stt_flag_state_match ;
/************************************************************************************************************
**************    Private Functions
************************************************************************************************************/
static inline char* get_atcmd_buf(struct cat_object *self);
static inline size_t get_atcmd_buf_size(struct cat_object *self);
static cat_status process_idle_state(struct cat_object *self);
// static void ack_error(struct cat_object *self);
// static cat_status error_state(struct cat_object *self);
static cat_status parse_prefix(struct cat_object *self);
static cat_status parse_command(struct cat_object *self);
static void prepare_parse_command(struct cat_object *self);
static cat_status wait_read_acknowledge(struct cat_object *self);
static void set_cmd_state(struct cat_object *self, size_t i, uint8_t state);
static void prepare_search_command(struct cat_object *self);
// static cat_status update_command(struct cat_object *self);
static int is_valid_cmd_name_char(const char ch);
static bool is_command_disable(struct cat_object *self, size_t index);
// static void reset_state(struct cat_object *self);
// static int read_cmd_char(struct cat_object *self);
// static void ack_ok(struct cat_object *self);
static cat_status search_command(struct cat_object *self);
static struct cat_command const* get_command_by_index(struct cat_object *self, size_t index);
// static cat_status command_found(struct cat_object *self);
uint8_t get_cmd_state(struct cat_object *self, size_t i);
static char to_upper(char ch);
/***********************************************************************************************************/

cat_status cat_service_get_fsm(char array_in ,struct cat_object *self)
{
  cat_status s;
  self->current_char = array_in;

  switch(self-> state)
  {
    case CAT_STATE_IDLE:
      s = process_idle_state(self);
      self->atc_receive[self->count_char_received] = self->current_char;
      self->count_char_received++;
      break;
    
    case CAT_STATE_PARSE_PREFIX:
      s = parse_prefix(self);
      self->atc_receive[self->count_char_received] = self->current_char;
      self->count_char_received++;
      break;
    
    case CAT_STATE_PARSE_COMMAND_CHAR:
      s = parse_command(self);
      self->atc_receive[self->count_char_received] = self->current_char;
      self->count_char_received++;
      break;

    case CAT_STATE_WAIT_READ_ACKNOWLEDGE:
      s = wait_read_acknowledge(self);
      self->atc_receive[self->count_char_received] = self->current_char;
      self->count_char_received++;
      break;
  }
  return s;
}

cat_status cat_service_search_command(struct cat_object *self) {
  if (self == NULL || self->atc_receive == NULL) 
  {
    return CAT_STATUS_ERROR; 
  }
  cat_status status = CAT_STATUS_BUSY; 
  char *fsm_receive = self->atc_receive;
  prepare_search_command(self);
  while (self->index < self->commands_num) 
  {
    status = update_command(self);
    status = search_command(self);
    self->index++;
  }
  if (++self->index >= self->commands_num) {
    if (self->cmd == NULL) {
      self->state = (self->current_char == '\n') ? CAT_STATE_COMMAND_NOT_FOUND : CAT_STATE_ERROR;
    } else {
      self->state = (self->partial_cntr == 1) ? CAT_STATE_COMMAND_FOUND : CAT_STATE_COMMAND_NOT_FOUND;
    }
    // self->index = 0;  
    self->cmd_type = CAT_CMD_TYPE_WRITE;
    // prepare_search_command(self);
  }
  return status; // Trả về trạng thái cuối cùng
}

// /************************************** Control Function *****************************************************************/

// static cat_status error_state(struct cat_object *self)
// {
//   // assert(self != NULL);

//   switch (self->current_char) {
//   case '\n':
//     // ack_error(self);
//     break;
//   case '\r':
//     self->cr_flag = true;
//     self->state = CAT_STATE_IDLE;
//     break;
//   default:
//     break;
//   }
//   return CAT_STATUS_BUSY;
// }

/**
 * 
 */
static cat_status process_idle_state(struct cat_object *self)
{
  switch (self->current_char) {
  case 'A':
    self->state = CAT_STATE_PARSE_PREFIX;
    break;
  case '\n':

  case '\r':
    break;
  default:
    self->state = CAT_STATE_ERROR;
    break;
  }
  return CAT_STATUS_BUSY;
}

static cat_status parse_prefix(struct cat_object *self)
{
  switch (self->current_char) {
  case 'T':
    prepare_parse_command(self);
    self->state = CAT_STATE_PARSE_COMMAND_CHAR;
    break;
  case '\n':
    break;
  case '\r':
    self->cr_flag = true;
    break;
  default:
    self->state = CAT_STATE_ERROR;
    break;
  }
  return CAT_STATUS_BUSY;
}

/**
 * brief: hàm này phân tích lệnh AT từ dữ liệu đầu vào
 * para1: self: con tro den doi tuong chua cac lenh AT
 * return: CAT_STATUS_BUSY neu dang xu ly, CAT_STATUS_OK neu da xu ly xong
 */
static cat_status parse_command(struct cat_object *self)
{
  switch (self->current_char) {
  case '\n':
    if (self->length != 0) {
      prepare_search_command(self);
      self->state = CAT_STATE_SEARCH_COMMAND;
      break;
    }
    // strncpy(get_atcmd_buf(self), "OK", get_atcmd_buf_size(self));
    break;

  case '\r':
    self->cr_flag = true;
    break;
  case '?':
      if (self->length == 0) {
      self->state = CAT_STATE_ERROR;
      break;
    }
    // self->length++;
    self->cmd_type = CAT_CMD_TYPE_READ;
    self->state = CAT_STATE_WAIT_READ_ACKNOWLEDGE;
    break;
  case '=':
    if (self->length == 0) {
      self->state = CAT_STATE_ERROR;
      break;
    }
    // self->length++;
    prepare_search_command(self);
    self->cmd_type = CAT_CMD_TYPE_WRITE;
    self->state = CAT_STATE_SEARCH_COMMAND;
    break;
  default:
    if (is_valid_cmd_name_char(self->current_char) != 0) {
      self->length++;
      self->state = CAT_STATE_PARSE_COMMAND_CHAR;
      break;
    }
    self->state = CAT_STATE_ERROR;
    break;
  }
  return CAT_STATUS_BUSY;
}

/**
 * brief: Hàm này thực hiện nhiệm vụ kiểm tra trạng thái khớp của lệnh 
 * (command state) và cập nhật trạng thái lệnh trong FSM dựa trên dữ liệu
 *  nhận được.
 * Kiểm tra lệnh người dùng nhập có khớp với lệnh trong danh sách định nghĩa hay không.
 * Cập nhật trạng thái khớp (CAT_CMD_STATE_NOT_MATCH, CAT_CMD_STATE_FULL_MATCH).
 * 
 */
char *cmd_name;
cat_status update_command(struct cat_object *self) {

  struct cat_command const *cmd = get_command_by_index(self, self->index);
  cmd_name = cmd->name;
  size_t cmd_name_len = strlen(cmd->name);
  
  if (get_cmd_state(self, self->index) != CAT_CMD_STATE_NOT_MATCH) {
    if (self->length > cmd_name_len) {
        set_cmd_state(self, self->index, CAT_CMD_STATE_NOT_MATCH);
    } else if (self->length == cmd_name_len) {
      bool is_match = true;
      for (size_t j = 0; j < self->length; j++) {
        if (to_upper(cmd->name[j]) != to_upper(self->atc_receive[j+2])) {
          is_match = false;
          break;
        }
      }
      if (is_match) {
        set_cmd_state(self, self->index, CAT_CMD_STATE_FULL_MATCH);
        stt_flag_state_match = 1;
        self->partial_cntr ++;
      } else {
        set_cmd_state(self, self->index, CAT_CMD_STATE_NOT_MATCH);
      }
    } 
    // Nếu chuỗi nhận ngắn hơn tên lệnh -> kiểm tra tiền tố
    else {
      bool is_partial_match = true;
      for (size_t j = 0; j < self->length; j++) {
        if (to_upper(cmd->name[j]) != to_upper(self->atc_receive[j+2])) {
          is_partial_match = false;
          break;
        }
      }
      if (is_partial_match) {
        set_cmd_state(self, self->index, CAT_CMD_STATE_PARTIAL_MATCH);
        self->state = CAT_STATE_PARSE_COMMAND_CHAR;  // Tiếp tục nhận thêm ký tự
      } else {
        set_cmd_state(self, self->index, CAT_CMD_STATE_NOT_MATCH);
      }
    }
  }
  return CAT_STATUS_BUSY;  // Tiếp tục trạng thái BUSY
}

static cat_status wait_read_acknowledge(struct cat_object *self)
{
  switch (self->current_char) {
  case '\n':
      prepare_search_command(self);
      self->state = CAT_STATE_SEARCH_COMMAND;
  break;
  case '\r':
      self->cr_flag = true;
  break;
  default:
      self->state = CAT_STATE_ERROR;
      break;
  }
  return CAT_STATUS_BUSY;
}
uint8_t cmd_state;
// tìm kiếm lệnh trong danh sách lệnh
static cat_status search_command(struct cat_object *self)
{
  // kiểm tra lệnh có khớp với lệnh trong danh sách không
  cmd_state = get_cmd_state(self, self->index);

  if (cmd_state != CAT_CMD_STATE_NOT_MATCH) {
    if (cmd_state == CAT_CMD_STATE_PARTIAL_MATCH) {
      if ((self->cmd != NULL) && ((self->index + 1) == self->commands_num)) {
        self->state = (self->current_char == '\n') ? CAT_STATE_COMMAND_NOT_FOUND : CAT_STATE_ERROR;
        return CAT_STATUS_BUSY;
      }
      self->cmd = get_command_by_index(self, self->index);
      self->partial_cntr++;
  } else if (cmd_state == CAT_CMD_STATE_FULL_MATCH) {
      self->cmd = get_command_by_index(self, self->index);
      self->state = CAT_STATE_COMMAND_FOUND;
      return CAT_STATUS_OK;
    }
  }
  return CAT_STATUS_BUSY;
}

// /**
//  * brief: xử lý khi tìm thấy lệnh tùy thuộc vào loại lệnh (cmd_type) 
//  * para1: self: con tro den doi tuong chua cac lenh AT
//  * return: CAT_STATUS_BUSY
//  */
// static cat_status command_found(struct cat_object *self)
// {
//   // assert(self != NULL);

//   switch (self->cmd_type) {
//   case CAT_CMD_TYPE_RUN:
//     if (self->cmd->only_test != false) {
//       ack_error(self);
//       break;
//     }
//     // if (self->cmd->run == NULL) {
//     //   ack_error(self);
//     //   break;
//     // }
//     self->state = CAT_STATE_RUN_LOOP;
//     break;
//   case CAT_CMD_TYPE_READ:
//     if (self->cmd->only_test != false) {
//       ack_error(self);
//       break;
//     }
//     // start_processing_format_read_args(self, CAT_FSM_TYPE_ATCMD);
//     break;
//   case CAT_CMD_TYPE_WRITE:
//     self->length = 0; // reset length
//     get_atcmd_buf(self)[0] = 0; // xóa buffer để sẵn sàng ghi dữ liệu mới
//     self->state = CAT_STATE_PARSE_COMMAND_ARGS;
//     break;
//   default:
//     ack_error(self);
//     break;
//   }
//   return CAT_STATUS_BUSY;
// }



// /***********************************************************************************************************/

// static int read_cmd_char(struct cat_object *self)
// {
//   assert(self != NULL);

//   if (self->io->read(&self->current_char) == 0)
//     return 0;

//   if (self->state != CAT_STATE_PARSE_COMMAND_ARGS)
//     self->current_char = to_upper(self->current_char);

//   return 1;
// }

// // static int read_cmd_char(struct cat_object *self) {
// //   assert(self != NULL);

// //   // Nếu `atc_receive` chưa được khởi tạo, trả về lỗi
// //   if (self->atc_receive == NULL) {
// //       return 0;
// //   }

// //   // Nếu `read_ptr` chưa được gán, khởi tạo nó bằng đầu của `atc_receive`
// //   if (read_ptr == NULL) {
// //       read_ptr = self->atc_receive;
// //   }

// //   // Nếu đã tới ký tự kết thúc chuỗi, trả về 0 để báo hiệu hết dữ liệu
// //   if (*read_ptr == '\0') {
// //       return 0;
// //   }

// //   // Lấy ký tự hiện tại và tăng con trỏ lên ký tự tiếp theo
// //   self->current_char = *read_ptr;
// //   read_ptr++;

// //   // Trả về ký tự vừa đọc dưới dạng số nguyên
// //   return 1;
// // }

/**
 * brief: kiem tra xem lenh co bi vo hieu hoa khong 
 * mỗi lệnh được biểu diễn bằng 2 bit trong mảng atcmd_buf
 * para1: self: con tro den doi tuong chua cac lenh AT
 * para2: i: chi so lenh trong danh sach
 * return: true neu lenh bi vo hieu hoa, nguoc lai tra ve false
 */
uint8_t get_cmd_state(struct cat_object *self, size_t i)
{
  uint8_t s;  
  if (self == NULL || i >= self->commands_num) {
    return CAT_CMD_STATE_NOT_MATCH;
  }
  if (is_command_disable(self, i) != false)
    return CAT_CMD_STATE_NOT_MATCH;
  /* get_atcmd_buf(self)trả về một mảng buffer trạng thái lưu trữ trạng thái của tất cả các lệnh.
    Mỗi lệnh chiếm 2 bit trong buffer. Do đó, 1 byte trong buffer có thể lưu trạng thái của 4 lệnh
    Chỉ mục i được chia 4 (i >> 2) để tìm byte tương ứng trong buffer.
  */
  s = get_atcmd_buf(self)[i >> 2];
  s >>= (i % 4) << 1;
  s &= 0x03;
  return s;
}

/**
 * brief: cập nhật trạng thái của lệnh AT
 * mỗi lệnh được biểu diễn bằng 2 bit trong mảng atcmd_buf
 * para1: self: con tro den doi tuong chua cac lenh AT
 * para2: i: chi so lenh trong danh sach
 * para3: state: trang thai cua lenh
 * return: khong co
 */
static void set_cmd_state(struct cat_object *self, size_t i, uint8_t state)
{
  uint8_t s;
  size_t n;
  uint8_t k;

  n = i >> 2;
  k = ((i % 4) << 1);

  s = get_atcmd_buf(self)[n];
  s &= ~(0x03 << k);
  s |= (state & 0x03) << k;
  get_atcmd_buf(self)[n] = s;
}


static bool is_command_disable(struct cat_object *self, size_t index)
{
  size_t i, j;
  struct cat_command_group const *cmd_group;

  assert(self != NULL);
  assert(index < self->commands_num);

  j = 0;
  for (i = 0; i < self->desc->cmd_group_num; i++) {
    cmd_group = self->desc->cmd_group[i];

    if (index >= j + cmd_group->cmd_num) {
      j += cmd_group->cmd_num;
      continue;
    }

    if (cmd_group->disable != false)
      return true;

    if (cmd_group->cmd[index - j].disable != false)
      return true;
    break;
  }

  return false;
}

// /***********************************************************************************************************/

static void ack_error(struct cat_object *self)
{
  strncpy(get_atcmd_buf(self), "ERROR", get_atcmd_buf_size(self));
  // start_flush_io_buffer(self, CAT_STATE_AFTER_FLUSH_RESET);
}

static void ack_ok(struct cat_object *self)
{
  strncpy(get_atcmd_buf(self), "OK", get_atcmd_buf_size(self));
  // start_flush_io_buffer(self, CAT_STATE_AFTER_FLUSH_RESET);
}

static void prepare_search_command(struct cat_object *self)
{
  self->index = 0;
  self->partial_cntr = 0;
  self->cmd = NULL;
}

/**
 * Hàm này truy cập vào cấu trúc dữ liệu tổ chức các lệnh AT theo nhóm lệnh (command groups).
 * Mục tiêu: Lấy lệnh (command) tại vị trí chỉ định dựa trên chỉ số index.
 * Đầu vào para1: self: Con trỏ đến đối tượng chứa các nhóm lệnh.
 * Đầu vào para2: index: Chỉ số lệnh trong danh sách.
 * Đầu ra: Con trỏ đến lệnh (struct cat_command).
 */
static struct cat_command const* get_command_by_index(struct cat_object *self, size_t index)
{
  size_t i, j;
  struct cat_command_group const *cmd_group;

  j = 0;
  for (i = 0; i < self->desc->cmd_group_num; i++) {
    cmd_group = self->desc->cmd_group[i];

    if (index >= j + cmd_group->cmd_num) {
      j += cmd_group->cmd_num;
      continue;
    }

    return &cmd_group->cmd[index - j];
  }
  return NULL;
}

/**
 * prepare_parse_command
 * Hàm này chuẩn bị bộ đệm lệnh AT cho việc phân tích lệnh AT.
 * Mục tiêu: Đặt tất cả các lệnh AT trong trạng thái khớp một phần (CAT_CMD_STATE_PARTIAL_MATCH).
 * Đầu vào para1: self: Con trỏ đến đối tượng chứa các lệnh AT.
 * Đầu ra: Không có.
 */
static void prepare_parse_command(struct cat_object *self)
{
  uint8_t val = (CAT_CMD_STATE_PARTIAL_MATCH << 0) | (CAT_CMD_STATE_PARTIAL_MATCH << 2) | (CAT_CMD_STATE_PARTIAL_MATCH << 4) |
                (CAT_CMD_STATE_PARTIAL_MATCH << 6);
  memset(get_atcmd_buf(self), val, get_atcmd_buf_size(self));

  self->index = 0;
  self->length = 0;
  self->cmd_type = CAT_CMD_TYPE_RUN;
}

static inline char* get_atcmd_buf(struct cat_object *self)
{
  return (char*)self->desc->buf;
}

static inline size_t get_atcmd_buf_size(struct cat_object *self)
{
  return (self->desc->unsolicited_buf != NULL) ? self->desc->buf_size : self->desc->buf_size >> 1;
}


static int is_valid_cmd_name_char(const char ch)
{
  return (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || (ch == '+') || (ch == '#') || (ch == '$') || (ch == '@') || (ch == '_') || (ch == '%') || (ch == '&');
}

// static void reset_state(struct cat_object *self)
// {
//   assert(self != NULL);

//   // if (self->hold_state_flag == false) {
//   //         self->state = CAT_STATE_IDLE;
//   //         self->cr_flag = false;
//   // } else {
//   //         self->state = CAT_STATE_HOLD;
//   // }
//   self->state = CAT_STATE_HOLD;
//   self->cmd = NULL;
//   self->cmd_type = CAT_CMD_TYPE_NONE;
// }

static char to_upper(char ch)
{
  return (ch >= 'a' && ch <= 'z') ? ch - ('a' - 'A') : ch;
}
