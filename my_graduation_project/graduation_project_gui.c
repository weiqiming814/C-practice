#include "../lv_examples.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cjson/cJSON.h>
#include <pcap.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include "../lv_examples.h"
#if LV_BUILD_EXAMPLES && LV_USE_LABEL

#define BUFFER_SIZE 8192
#define MAX_PACKET_SIZE 1518
#define QUEUE_SIZE 100

#define MAX_LINE 4096
typedef struct {
  char data[MAX_PACKET_SIZE];
  int size;
} packet_t;

typedef struct _FuncCall{
  char funcName[64];
  char *params[2];
} FuncCall;

typedef struct _URL {
  char protocol[8];
  char host[64];
  char port[8];
  char path[64];
}URL;

static void perr_exit(const char *s) {
  perror(s);
  exit(1);
}
LV_FONT_DECLARE(chinese_songti);

// 应用字体到样式

lv_obj_t *text_area;
lv_obj_t *main_page;
lv_obj_t *sign_in_page;
lv_obj_t *sign_up_page;
lv_obj_t *calculate1_page;
lv_obj_t *calculate2_page;
lv_obj_t *change_ipaddr_page;
lv_obj_t *ipmac_info_page;
lv_obj_t *port_info_page;
lv_obj_t *capture_page;
lv_style_t style_btn;
lv_style_t style_btn_pressed;
lv_obj_t *cont; // 窗口对象需要是全局的，这样事件处理函数才能访问它

static pthread_t capture_thread;
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
static packet_t packet_queue[QUEUE_SIZE];
static int queue_head = 0, queue_tail = 0;
static bool is_running = false;

void create_page(void);
void create_main_page(void);
void create_sign_up_page(void);
void create_sign_in_page(void);
static void btn_event_cb(lv_event_t *e);
static void btn_sign_in_event_cb(lv_event_t *e);
static void btn_sign_up_event_cb(lv_event_t *e);
static void event_handler(lv_event_t *e);
char *send_params(char *name, const char *params1, const char *params2);
char* func_call_to_json(FuncCall *call);
char *post_web_info(char buf[], URL url, const char *data);
void clear_func_call(FuncCall *call);
void handle_case_a(lv_obj_t *parent);
void handle_case_b(lv_obj_t *parent);
void handle_case_c(lv_obj_t *parent);
void handle_case_d(lv_obj_t *parent);
static void start_button_clicked(lv_event_t *e);
static void stop_button_clicked(lv_event_t *e);
static void btn_change_ip_event_handler(lv_obj_t *btn);
static void update_text_area(lv_timer_t *timer);
static void btn_case_calculator1_event_handler(lv_obj_t *btn);
static void btn_case_calculator2_event_handler(lv_obj_t *btn);
static void btn_case_ipmacinf_event_handler(lv_obj_t *btn);
static void btn_case_portinf_event_handler(lv_obj_t *btn);
char *display_listening_ports(const char* filename, const char* protocol);
char* change_ipaddress(const char *iface, const char *new_ip);
static void btn_change_ip_event_handler(lv_obj_t *btn);

void lv_example_get_started_4(void) {
  create_page();
}

void create_page(void) {
  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, lv_color_hex(0xDDDDDD)); // 浅灰色
  lv_style_set_border_color(&style_btn, lv_color_hex(0x000000)); // 黑色边框
  lv_style_set_border_width(&style_btn, 2); // 2像素边框宽度

  lv_style_init(&style_btn_pressed);
  lv_style_set_bg_color(&style_btn_pressed, lv_color_hex(0xAAAAAA)); // 深灰色
  create_sign_in_page();
}

void create_sign_in_page(void) {
  lv_obj_t *sign_in_page_screen = lv_obj_create(NULL); // 创建一个新的屏幕对象代替原来的main_page对象
  lv_scr_load(sign_in_page_screen);  
  sign_in_page = lv_obj_create(sign_in_page_screen);
  lv_obj_set_size(sign_in_page, 1400, 900);

  lv_obj_t *label = lv_label_create(sign_in_page);
  lv_label_set_text(label, "sign in");
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 150);

  lv_obj_t *username_ta = lv_textarea_create(sign_in_page);
  lv_obj_align(username_ta, LV_ALIGN_TOP_MID, 0 ,170);
  lv_textarea_set_one_line(username_ta, true);
  lv_textarea_set_placeholder_text(username_ta, "username");

  lv_obj_t *password_ta = lv_textarea_create(sign_in_page);
  lv_obj_align_to(password_ta, username_ta, LV_ALIGN_OUT_BOTTOM_LEFT, 0 ,10);
  lv_textarea_set_one_line(password_ta, true);
  lv_textarea_set_placeholder_text(password_ta, "password");

  lv_obj_t *sign_in_btn = lv_btn_create(sign_in_page);
  lv_obj_align_to(sign_in_btn, password_ta, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
  lv_obj_set_size(sign_in_btn, lv_obj_get_width(password_ta) / 2 - 10, lv_obj_get_height(password_ta));

  lv_obj_t *sign_up_btn = lv_btn_create(sign_in_page);
  lv_obj_align_to(sign_up_btn, sign_in_btn, LV_ALIGN_OUT_RIGHT_TOP, 20, 0);
  lv_obj_set_size(sign_up_btn, lv_obj_get_width(password_ta) / 2 - 10, lv_obj_get_height(password_ta));

  lv_obj_t *error_msg_label = lv_label_create(sign_in_page);

  lv_label_set_text(error_msg_label, "");
  static lv_style_t style_red;
  lv_style_init(&style_red);
  lv_style_set_text_color(&style_red, lv_color_hex(0xFF0000));
  lv_obj_add_style(error_msg_label, &style_red, 0);
  lv_obj_align_to(error_msg_label, sign_in_btn, LV_ALIGN_OUT_BOTTOM_LEFT, 0 ,10);
  lv_obj_t *sign_up_label = lv_label_create(sign_up_btn);
  lv_label_set_text(sign_up_label, "注册");
  lv_obj_align_to(sign_up_label, sign_up_btn, LV_ALIGN_CENTER, -5, 0);
  lv_obj_set_style_text_font(sign_up_label, &chinese_songti, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *sign_in_label = lv_label_create(sign_in_btn);
  lv_label_set_text(sign_in_label, "登录");
  lv_obj_align_to(sign_in_label, sign_in_btn, LV_ALIGN_CENTER, -5, 0);
  lv_obj_set_style_text_font(sign_in_label, &chinese_songti, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(sign_up_btn, btn_sign_in_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(sign_in_btn, btn_sign_in_event_cb, LV_EVENT_ALL, NULL);
}

void create_sign_up_page(void) {
  sign_up_page = lv_obj_create(lv_scr_act());
  lv_obj_set_size(sign_up_page, 1400, 900);

  lv_obj_t *label = lv_label_create(sign_up_page);
  lv_label_set_text(label, "sign up");
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 150);

  lv_obj_t *username_ta = lv_textarea_create(sign_up_page);
  lv_obj_align(username_ta, LV_ALIGN_TOP_MID, 0 ,170);
  lv_textarea_set_one_line(username_ta, true);
  lv_textarea_set_placeholder_text(username_ta, "username");

  lv_obj_t *password_ta = lv_textarea_create(sign_up_page);
  lv_obj_align_to(password_ta, username_ta, LV_ALIGN_OUT_BOTTOM_LEFT, 0 ,10);
  lv_textarea_set_one_line(password_ta, true);
  lv_textarea_set_placeholder_text(password_ta, "password");

  lv_obj_t *sign_up_btn = lv_btn_create(sign_up_page);
  lv_obj_align_to(sign_up_btn, password_ta, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
  lv_obj_set_size(sign_up_btn, lv_obj_get_width(password_ta) / 2 - 10, lv_obj_get_height(password_ta));

  lv_obj_t *sign_in_btn = lv_btn_create(sign_up_page);
  lv_obj_align_to(sign_in_btn, sign_up_btn, LV_ALIGN_OUT_RIGHT_TOP, 20, 0);
  lv_obj_set_size(sign_in_btn, lv_obj_get_width(password_ta) / 2 - 10, lv_obj_get_height(password_ta));
  lv_obj_t *error_msg_label = lv_label_create(sign_up_page);
  lv_label_set_text(error_msg_label, "");
  static lv_style_t style_red;
  lv_style_init(&style_red);
  lv_style_set_text_color(&style_red, lv_color_hex(0xFF0000));
  lv_obj_add_style(error_msg_label, &style_red, 0);
  lv_obj_align_to(error_msg_label, sign_up_btn, LV_ALIGN_OUT_BOTTOM_LEFT, 0 ,10);

  lv_obj_t *sign_up_label = lv_label_create(sign_up_btn);
  lv_label_set_text(sign_up_label, "注册");
  lv_obj_align_to(sign_up_label, sign_up_btn, LV_ALIGN_CENTER, -5, 0);
  lv_obj_set_style_text_font(sign_up_label, &chinese_songti, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *sign_in_label = lv_label_create(sign_in_btn);
  lv_label_set_text(sign_in_label, "登录");
  lv_obj_align_to(sign_in_label, sign_in_btn, LV_ALIGN_CENTER, -5, 0);
  lv_obj_set_style_text_font(sign_in_label, &chinese_songti, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(sign_up_btn, btn_sign_up_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(sign_in_btn, btn_sign_up_event_cb, LV_EVENT_ALL, NULL);
}

void create_main_page(void) {
  lv_obj_t *main_page_screen = lv_obj_create(NULL); // 创建一个新的屏幕对象代替原来的main_page对象
  lv_scr_load(main_page_screen);
  main_page = lv_obj_create(main_page_screen);
  lv_obj_align(main_page,LV_ALIGN_CENTER,0,0);
  lv_obj_set_size(main_page, 1400, 900);
  lv_obj_set_style_bg_color(main_page, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

  // 创建一个横向的列表
  lv_obj_t *list = lv_list_create(main_page);
  lv_obj_set_size(list, lv_pct(100), lv_pct(5)); // 宽度占满父对象，高度由内容决定
  lv_obj_set_flex_flow(list, LV_FLEX_FLOW_ROW);
  lv_obj_set_align(list, LV_ALIGN_TOP_MID); // 顶部居中对齐

  // 创建按钮并应用默认样式
  const char *btn_labels[] = {"子网掩码计算器", "IPMAC信息", "IP绑定", "抓包", "登出"};
  for (int i = 0; i < sizeof(btn_labels) / sizeof(btn_labels[0]); ++i) {
    lv_obj_t *list_btn = lv_list_add_btn(list, NULL, btn_labels[i]);
  lv_obj_set_style_text_font(list_btn, &chinese_songti, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(list_btn, &style_btn, 0);
    lv_obj_set_width(list_btn, lv_pct(20));
    lv_obj_add_event_cb(list_btn, event_handler, LV_EVENT_CLICKED, NULL);
  }
}

static void btn_sign_in_event_cb(lv_event_t *e){
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  const char *btn_text = lv_label_get_text(lv_obj_get_child(btn, 0));
  if (code == LV_EVENT_CLICKED)  {
    if (strcmp(btn_text, "登录") == 0) {
      char *result = send_params("Sign_in", lv_textarea_get_text(lv_obj_get_child(sign_in_page, 1)),lv_textarea_get_text(lv_obj_get_child(sign_in_page, 2)));
      if (strcmp(result, "successful\n") != 0) {
        // 检查是否需要创建新的错误消息标签或更新现有的
        lv_label_set_text(lv_obj_get_child(sign_in_page, 5), result ? result : "Unknown error");
      } else {
        create_main_page(); // 登录成功，加载主页面
      }
    } else if (strcmp(btn_text, "注册") == 0) {
      create_sign_up_page();
    }
  }
}

static void btn_sign_up_event_cb(lv_event_t *e){
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  const char *btn_text = lv_label_get_text(lv_obj_get_child(btn, 0));
  if (code == LV_EVENT_CLICKED) {
    if (strcmp(btn_text, "注册") == 0) {
      // 假设send_params是发送注册信息，并返回服务器的响应
      char *result = send_params("Sign_up", lv_textarea_get_text(lv_obj_get_child(sign_up_page, 1)), lv_textarea_get_text(lv_obj_get_child(sign_up_page, 2)));
      // 假设返回的字符串 "successful\n" 表示注册成功
      lv_label_set_text(lv_obj_get_child(sign_up_page, 5), result ? result : "Unknown error");
    } else if (strcmp(btn_text, "登录") == 0) {
      // 如果有一个返回登录页面的按钮
      create_sign_in_page(); // 创建登录页面
    }
  }
}

char *send_params(char *name, const char *params1, const char *params2) {
  char buf[MAX_LINE];
  URL url;
  strcpy(url.protocol, "http"); // 假设是HTTP协议
  strcpy(url.host, "localhost");
  strcpy(url.port, "8080");
  strcpy(url.path, "/"); // 或者设置为你需要的特定路径
  FuncCall call;
  strcpy(call.funcName, name);
  //call.params[0] = strdup("24");
  call.params[0] = strdup(params1);
  call.params[1] = strdup(params2);
  char *json_data = func_call_to_json(&call);
  char *buffer = post_web_info(buf, url, json_data);
  free(json_data);
  clear_func_call(&call);
  //get_web_info(url);

  return buffer;
}

char* func_call_to_json(FuncCall *call) {
  cJSON *root = cJSON_CreateObject();
  cJSON_AddItemToObject(root, "funcName", cJSON_CreateString(call->funcName));

  cJSON *params = cJSON_CreateArray();
  for (int i = 0; i < 2; ++i) {
    if (call->params[i] != NULL) {
      cJSON_AddItemToArray(params, cJSON_CreateString(call->params[i]));
    }
  }
  cJSON_AddItemToObject(root, "params", params);

  char* jsonString = cJSON_Print(root);
  cJSON_Delete(root);
  return jsonString; // 注意：调用者需要释放这个字符串的内存
}

char *post_web_info(char buf[], URL url, const char *data) {
  int sockfd, num;
  struct hostent *he;
  struct sockaddr_in servaddr;

  // 获取主机地址信息
  if ((he = gethostbyname(url.host)) == NULL) {
    perr_exit("gethostbyname error");
  }

  // 创建套接字
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perr_exit("socket creation error");
  }

  // 设置服务器地址
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(url.port));
  servaddr.sin_addr = *((struct in_addr *)he->h_addr);

  // 连接服务器
  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
    perr_exit("connect error");
  }

  // 计算整个HTTP请求的长度
  int header_len = snprintf(NULL, 0,
      "POST %s HTTP/1.1\r\n"
      "Host: %s\r\n"
      "Content-Type: application/json\r\n"
      "Content-Length: %ld\r\n\r\n",
      url.path, url.host, strlen(data));
  int total_len = header_len + strlen(data);

  // 为整个HTTP请求分配内存
  char *request = (char *)malloc(total_len + 1); // +1 for null terminator
  if (request == NULL) {
    perr_exit("malloc failed");
  }

  // 构造HTTP请求
  sprintf(request,
      "POST %s HTTP/1.1\r\n"
      "Host: %s\r\n"
      "Content-Type: application/json\r\n"
      "Content-Length: %ld\r\n\r\n%s",
      url.path, url.host, strlen(data), data);

  // 发送HTTP请求
  if (send(sockfd, request, total_len, 0) == -1) {
    perr_exit("send request error");
  }

  // 释放请求内存
  free(request);

  // 读取响应
  while ((num = recv(sockfd, buf, MAX_LINE, 0)) > 0) {
    buf[num] = '\0';
    printf("%s", buf);
  }

  close(sockfd);
  return buf;
}

void clear_func_call(FuncCall *call) {
  memset(call->funcName, 0, sizeof(call->funcName));
  for (int i = 0; i < 2; ++i) {
    if (call->params[i] != NULL) {
      free(call->params[i]);
      call->params[i] = NULL;
    }
  }
}

static void event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    // 重置所有按钮的样式
    lv_obj_t *list = lv_obj_get_parent(btn);
    uint32_t i;
    for (i = 0; i < lv_obj_get_child_cnt(list); i++) {
      lv_obj_t *child = lv_obj_get_child(list, i);
      lv_obj_remove_style(child, &style_btn_pressed, 0);
      lv_obj_add_style(child, &style_btn, 0);
    }
    // 应用被按下的样式到当前按钮
    lv_obj_remove_style(btn, &style_btn, 0);
    lv_obj_add_style(btn, &style_btn_pressed, 0);

    // 更新窗口标题为当前按钮文本
    char *current_btn_text;
if (strcmp(lv_list_get_btn_text(list, btn), "子网掩码计算器") == 0) {
  current_btn_text = "A";
} else if (strcmp(lv_list_get_btn_text(list, btn), "IPMAC信息") == 0) {
  current_btn_text = "B";
} else if (strcmp(lv_list_get_btn_text(list, btn), "IP绑定") == 0) {
  current_btn_text = "C";
} else if (strcmp(lv_list_get_btn_text(list, btn), "抓包") == 0) {
  current_btn_text = "D";
} else if (strcmp(lv_list_get_btn_text(list, btn), "登出") == 0) {
  current_btn_text = "E";
}

// 创建窗口对象
    if (cont) {
      lv_obj_del(cont);
    }

    cont = lv_obj_create(main_page);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(95)); // 宽度占满父对象，高度由内容决定
    lv_obj_align_to(cont, list, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0); // 将窗口对齐到列表下方
  lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    switch (current_btn_text[0]) {
      case 'A':
        handle_case_a(cont);
        break;
      case 'B':
        handle_case_b(cont);
        break;
      case 'C':
        handle_case_c(cont);
        break;
      case 'D':
        handle_case_d(cont);
        break;
      case 'E':
        create_sign_in_page();
        break;
    }
  }
}

void handle_case_a(lv_obj_t *parent) {
  lv_obj_t *main_cont = lv_obj_create(parent);
  lv_obj_set_scrollbar_mode(main_cont, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_size(main_cont, 650, 750); // 根据屏幕大小调整
  lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

  /* 创建标题 */
  lv_obj_t *title = lv_label_create(main_cont);
  lv_label_set_text(title, "Subnet Mask Calculator");
  lv_obj_set_width(title, lv_pct(100));
  lv_obj_set_height(title, lv_pct(10));
  lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);

  /* 创建IP和掩码位标签及输入框所在的容器 */
  calculate1_page = lv_obj_create(main_cont);
  lv_obj_set_scrollbar_mode(calculate1_page, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_width(calculate1_page, lv_pct(100));
  lv_obj_set_height(calculate1_page, lv_pct(20));
  lv_obj_set_flex_flow(calculate1_page, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(calculate1_page, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  /* 创建IP标签和输入框 */
  lv_obj_t *ip_label = lv_label_create(calculate1_page);
  lv_label_set_text(ip_label, "IP:");
  lv_obj_t *ip_ta = lv_textarea_create(calculate1_page);
  lv_textarea_set_one_line(ip_ta, true);
  lv_obj_set_width(ip_ta, lv_pct(20)); // 根据需要调整宽度

  /* 创建掩码位标签和输入框 */
  lv_obj_t *mask_label = lv_label_create(calculate1_page);
  lv_label_set_text(mask_label, "mark:");
  lv_obj_t *mask_ta = lv_textarea_create(calculate1_page);
  lv_textarea_set_one_line(mask_ta, true);
  lv_obj_set_width(mask_ta, lv_pct(20)); // 根据需要调整宽度
  lv_obj_t* btn = lv_btn_create(calculate1_page);
  lv_obj_t* label = lv_label_create(btn);
  lv_label_set_text(label, "calculate");
  lv_obj_align_to(btn, mask_ta, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
  /* 创建底部容器 */
  lv_obj_t *bottom_cont = lv_obj_create(main_cont);
  lv_obj_set_width(bottom_cont, lv_pct(100));
  lv_obj_set_height(bottom_cont, lv_pct(70)); // 占据剩余的空间

  lv_obj_t *main2_cont = lv_obj_create(parent);
  lv_obj_align_to(main2_cont, main_cont, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
  lv_obj_set_scrollbar_mode(main2_cont, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_size(main2_cont, 650, 750); // 根据屏幕大小调整
  lv_obj_set_flex_flow(main2_cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(main2_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_t *title_2 = lv_label_create(main2_cont);
  lv_label_set_text(title_2, "Mask decimal calculator");
  lv_obj_set_width(title_2, lv_pct(100));
  lv_obj_set_height(title_2, lv_pct(10));
  lv_obj_set_style_text_align(title_2, LV_TEXT_ALIGN_CENTER, 0);

  calculate2_page = lv_obj_create(main2_cont);
  lv_obj_set_scrollbar_mode(calculate2_page, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_width(calculate2_page, lv_pct(100));
  lv_obj_set_height(calculate2_page, lv_pct(20));
  lv_obj_set_flex_flow(calculate2_page, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(calculate2_page, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  /* 创建IP标签和输入框 */
  lv_obj_t *mark = lv_label_create(calculate2_page);
  lv_label_set_text(mark, "MARK:");
  lv_obj_t *mark_ta = lv_textarea_create(calculate2_page);
  lv_textarea_set_one_line(mark_ta, true);
  lv_obj_set_width(mark_ta, lv_pct(20)); // 根据需要调整宽度
  lv_obj_t* btn2 = lv_btn_create(calculate2_page);
  lv_obj_t* label2 = lv_label_create(btn2);
  lv_label_set_text(label2, "calculate");
  lv_obj_align_to(btn2, mark_ta, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
  lv_obj_add_event_cb(btn2, btn_event_cb, LV_EVENT_CLICKED, NULL);
  /* 创建底部容器 */
  lv_obj_t *bottom2_cont = lv_obj_create(main2_cont);
  lv_obj_set_width(bottom2_cont, lv_pct(100));
  lv_obj_set_height(bottom2_cont, lv_pct(70)); // 占据剩余的空间
}

void handle_case_b(lv_obj_t *parent) {
  ipmac_info_page = lv_obj_create(parent);
  lv_obj_set_size(ipmac_info_page, 700, 800);
  lv_obj_set_scrollbar_mode(ipmac_info_page, LV_SCROLLBAR_MODE_OFF);
  lv_obj_t *cont3 = lv_obj_create(ipmac_info_page);
  lv_obj_set_size(cont3, 650, 600);
  lv_obj_align(cont3, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_t *btn3 = lv_btn_create(ipmac_info_page);
  lv_obj_t *label3 = lv_label_create(btn3);
  lv_label_set_text(label3, "IP/MAC information");
  lv_obj_align_to(btn3, ipmac_info_page, LV_ALIGN_BOTTOM_MID, 0, -40);
  lv_obj_add_event_cb(btn3, btn_event_cb, LV_EVENT_CLICKED, NULL);

  port_info_page = lv_obj_create(parent);
  lv_obj_align_to(port_info_page, ipmac_info_page, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
  lv_obj_set_size(port_info_page, 700, 800);
  lv_obj_set_scrollbar_mode(port_info_page, LV_SCROLLBAR_MODE_OFF);
  lv_obj_t *cont4 = lv_obj_create(port_info_page);
  lv_obj_set_size(cont4, 650, 600);
  lv_obj_t *btn4 = lv_btn_create(port_info_page);
  lv_obj_t *label4 = lv_label_create(btn4);
  lv_label_set_text(label4, "PORT information");
  lv_obj_align_to(btn4, port_info_page, LV_ALIGN_BOTTOM_MID, 0, -40);
  lv_obj_add_event_cb(btn4, btn_event_cb, LV_EVENT_CLICKED, NULL);
}

void handle_case_c(lv_obj_t *parent) {
  lv_obj_t *main_cont = lv_obj_create(parent);
  lv_obj_set_scrollbar_mode(main_cont, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_size(main_cont, 1000, 700); // 根据屏幕大小调整
  lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
      lv_obj_align(main_cont, LV_ALIGN_CENTER, 0, 0);

  change_ipaddr_page = lv_obj_create(main_cont);
  lv_obj_set_scrollbar_mode(change_ipaddr_page, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_width(change_ipaddr_page, lv_pct(100));
  lv_obj_set_height(change_ipaddr_page, lv_pct(20));
  lv_obj_set_flex_flow(change_ipaddr_page, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(change_ipaddr_page, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  /* 创建IP标签和输入框 */
  lv_obj_t *ip_label = lv_label_create(change_ipaddr_page);
  lv_label_set_text(ip_label, "iface:");
  lv_obj_t *ip_ta = lv_textarea_create(change_ipaddr_page);
  lv_textarea_set_one_line(ip_ta, true);
  lv_obj_set_width(ip_ta, lv_pct(20)); // 根据需要调整宽度

  /* 创建掩码位标签和输入框 */
  lv_obj_t *mask_label = lv_label_create(change_ipaddr_page);
  lv_label_set_text(mask_label, "new_ip:");
  lv_obj_t *mask_ta = lv_textarea_create(change_ipaddr_page);
  lv_textarea_set_one_line(mask_ta, true);
  lv_obj_set_width(mask_ta, lv_pct(20)); // 根据需要调整宽度
  lv_obj_t* btn = lv_btn_create(change_ipaddr_page);
  lv_obj_t* label = lv_label_create(btn);
  lv_label_set_text(label, "change");
  lv_obj_align_to(btn, mask_ta, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
  /* 创建底部容器 */
  lv_obj_t *bottom_cont = lv_obj_create(main_cont);
  lv_obj_set_width(bottom_cont, lv_pct(100));
  lv_obj_set_height(bottom_cont, lv_pct(70)); // 占据剩余的空间
}

void handle_case_d(lv_obj_t *parent) {
  capture_page = lv_obj_create(parent);
  lv_obj_set_size(capture_page, 1000, 700);
      lv_obj_align(capture_page, LV_ALIGN_CENTER, 0, 0);
  lv_obj_t *start_button = lv_btn_create(capture_page);
  lv_obj_align(start_button, LV_ALIGN_TOP_LEFT, 300, 10);
  lv_obj_add_event_cb(start_button, start_button_clicked, LV_EVENT_CLICKED, NULL);
  lv_obj_t *start_label = lv_label_create(start_button);
  lv_label_set_text(start_label, "Start");

  lv_obj_t *stop_button = lv_btn_create(capture_page);
  lv_obj_align(stop_button, LV_ALIGN_TOP_RIGHT, -300, 10);
  lv_obj_add_event_cb(stop_button, stop_button_clicked, LV_EVENT_CLICKED, NULL);
  lv_obj_t *stop_label = lv_label_create(stop_button);
  lv_label_set_text(stop_label, "Stop");

  text_area = lv_textarea_create(capture_page);
  lv_obj_set_size(text_area,900, 600);
  lv_obj_align(text_area, LV_ALIGN_CENTER, 0, 20);


  lv_timer_create(update_text_area, 100, NULL);

}

static void btn_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  lv_obj_t *cont = lv_obj_get_parent(btn);
  if (code == LV_EVENT_CLICKED)  {
    if (cont == calculate1_page) {
      btn_case_calculator1_event_handler(btn);
    } else if (cont == calculate2_page) {
      btn_case_calculator2_event_handler(btn);
    } else if (cont == change_ipaddr_page) {
      btn_change_ip_event_handler(btn);
    } else if (cont == ipmac_info_page) {
      btn_case_ipmacinf_event_handler(btn);
    } else if (cont == port_info_page) {
      btn_case_portinf_event_handler(btn);
    }
  }
}

char* calculate_subnet_details(const char *ip_str, int mask_bits) {
  static char result[1024]; // 静态分配足够大的空间来存储结果
  uint32_t ip, mask, network, broadcast, first, last, available_ips;
  inet_pton(AF_INET, ip_str, &ip);
  ip = ntohl(ip);

  mask = ~(UINT32_MAX >> mask_bits);
  network = ip & mask;
  broadcast = network | ~mask;
  first = network + 1;
  last = broadcast - 1;
  available_ips = last - first + 1;

  struct in_addr addr;
  char addr_str[INET_ADDRSTRLEN];
  result[0] = '\0'; // 清空结果字符串

  // 将结果拼接成一个大的字符串
  addr.s_addr = htonl(mask);
  inet_ntop(AF_INET, &addr, addr_str, INET_ADDRSTRLEN);
  snprintf(result, sizeof(result), "Mask: %s\n", addr_str);

  addr.s_addr = htonl(network);
  inet_ntop(AF_INET, &addr, addr_str, INET_ADDRSTRLEN);
  snprintf(result + strlen(result), sizeof(result) - strlen(result), "Network: %s\n", addr_str);

  addr.s_addr = htonl(first);
  inet_ntop(AF_INET, &addr, addr_str, INET_ADDRSTRLEN);
  snprintf(result + strlen(result), sizeof(result) - strlen(result), "First Usable: %s\n", addr_str);

  addr.s_addr = htonl(last);
  inet_ntop(AF_INET, &addr, addr_str, INET_ADDRSTRLEN);
  snprintf(result + strlen(result), sizeof(result) - strlen(result), "Last Usable: %s\n", addr_str);

  addr.s_addr = htonl(broadcast);
  inet_ntop(AF_INET, &addr, addr_str, INET_ADDRSTRLEN);
  snprintf(result + strlen(result), sizeof(result) - strlen(result), "Broadcast: %s\nAvailable IPs: %u\n", addr_str, available_ips);

  return result;
}

static void btn_case_calculator1_event_handler(lv_obj_t *btn) {
  lv_obj_t *input_cont = lv_obj_get_parent(btn);
  lv_obj_t *ip_ta = lv_obj_get_child(input_cont, 1); // 根据创建顺序获取
  lv_obj_t *mask_ta = lv_obj_get_child(input_cont, 3); // 同上

  const char *ip = lv_textarea_get_text(ip_ta);
  const char *mask_str = lv_textarea_get_text(mask_ta);
  int mask_bits = atoi(mask_str); // 将字符串转换为整数
  char *output = calculate_subnet_details(ip, mask_bits);

  // 找到底部容器并在其中显示结果
  lv_obj_t *bottom_cont = lv_obj_get_child(lv_obj_get_parent(input_cont), 2); // 根据你的布局调整索引
  if (bottom_cont) { // 确认 bottom_cont 存在
    lv_obj_clean(bottom_cont); // 清除 bottom_cont 中的所有子对象

    lv_obj_t *output_label = lv_label_create(bottom_cont);
    lv_label_set_text(output_label, output);
    lv_obj_align(output_label, LV_ALIGN_TOP_LEFT, 0, 0); // 根据需要调整对齐方式
  } else {
    printf("Error: bottom_cont is NULL.\n");
  }
}

char* calculate_subnet_details2(const int mask_bits) {

  static char result[1024]; // 静态分配足够大的空间来存储结果
  uint32_t mask = ~(UINT32_MAX >> mask_bits);

  // Output in decimal
  struct in_addr mask_addr;
  mask_addr.s_addr = htonl(mask); // Convert to network byte order
  char mask_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &mask_addr, mask_str, INET_ADDRSTRLEN);
  result[0] = '\0';
  snprintf(result, sizeof(result), "Decimal: %s\nHex: %#X\n" , mask_str, mask);
  return result;
}

static void btn_case_calculator2_event_handler(lv_obj_t *btn) {
  lv_obj_t *input_cont = lv_obj_get_parent(btn);
  lv_obj_t *mark_ta = lv_obj_get_child(input_cont, 1); // 根据创建顺序获取
  const char *mark_str = lv_textarea_get_text(mark_ta);
  int mask_bits = atoi(mark_str); // 将字符串转换为整数
  char *output = calculate_subnet_details2(mask_bits);
  lv_obj_t *bottom_cont = lv_obj_get_child(lv_obj_get_parent(input_cont), 2); // 根据你的布局调整索引
  if (bottom_cont) { // 确认 bottom_cont 存在
    lv_obj_clean(bottom_cont); // 清除 bottom_cont 中的所有子对象

    lv_obj_t *output_label = lv_label_create(bottom_cont);
    lv_label_set_text(output_label, output);
    lv_obj_align(output_label, LV_ALIGN_TOP_LEFT, 0, 0); // 根据需要调整对齐方式
  } else {
    printf("Error: bottom_cont is NULL.\n");
  }
}

static void btn_case_ipmacinf_event_handler(lv_obj_t *btn) {
  lv_obj_t *cont1 = lv_obj_get_child(lv_obj_get_parent(btn), 0);
  struct ifaddrs *ifaddr, *ifa;
  int family, s;
  char host[NI_MAXHOST];
  unsigned char *ptr;
  static char result[8192] = ""; // 假设这个大小足够；实际情况可能需要更大或者动态分配

  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    exit(EXIT_FAILURE);
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;

    family = ifa->ifa_addr->sa_family;

    // 使用sprintf和strcat来构建字符串
    char line[1024]; // 临时字符串，用于格式化当前行
    snprintf(line, sizeof(line), "%-8s %s (%d)\n",
        ifa->ifa_name,
        (family == AF_PACKET) ? "AF_PACKET" :
        (family == AF_INET) ? "AF_INET" :
        (family == AF_INET6) ? "AF_INET6" : "???",
        family);
    strcat(result, line); // 注意: result 也需要检查溢出
    if (family == AF_INET || family == AF_INET6) {
      s = getnameinfo(ifa->ifa_addr,
          (family == AF_INET) ? sizeof(struct sockaddr_in) :
          sizeof(struct sockaddr_in6),
          host, NI_MAXHOST,
          NULL, 0, NI_NUMERICHOST);
      if (s != 0) {
        printf("getnameinfo() failed: %s\n", gai_strerror(s));
        freeifaddrs(ifaddr);
        exit(EXIT_FAILURE);
      }

      sprintf(line, "\t\taddress: <%s>\n", host);
      strcat(result, line);
    } else if (family == AF_PACKET) {
      struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
      ptr = (unsigned char*)s->sll_addr;
      sprintf(line, "\t\tMAC: ");
      strcat(result, line);
      for (int i = 0; i < s->sll_halen; i++) {
        snprintf(line, sizeof(line), "%02x", *ptr++);
        strcat(result, line); // 注意: 这里可能需要更复杂的逻辑以避免result溢出
        if (i != (s->sll_halen - 1))
          strcat(result, ":");
      }
      strcat(result, "\n");
    }
  }
  if (cont1) { // 确认 bottom_cont 存在
    lv_obj_clean(cont1); // 清除 bottom_cont 中的所有子对象

    lv_obj_t *output_label = lv_label_create(cont1);
    lv_label_set_text(output_label, result);
    lv_obj_align_to(output_label, cont1, LV_ALIGN_TOP_LEFT, 0, 0); // 根据需要调整对齐方式
    result[0] = '\0';
  } else {
    printf("Error: bottom_cont is NULL.\n");
  }
  freeifaddrs(ifaddr);
}

static void btn_case_portinf_event_handler(lv_obj_t *btn) {
  lv_obj_t *cont1 = lv_obj_get_child(lv_obj_get_parent(btn), 0);
  lv_obj_clean(cont1); // 清除之前的显示结果
  char *tcpResult = display_listening_ports("/proc/net/tcp", "TCP");
  char *udpResult = display_listening_ports("/proc/net/udp", "UDP");
  // 创建并显示结果
  lv_obj_t *output_label_tcp = lv_label_create(cont1);
  lv_label_set_text(output_label_tcp, tcpResult);
  lv_obj_align(output_label_tcp, LV_ALIGN_TOP_LEFT, 0, 0);
}

char *display_listening_ports(const char* filename, const char* protocol) {
  static char result[8192];  
  if(protocol == "TCP") {
    result[0] = '\0'; // 初始化为空字符串
  }
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    perror("Error opening file");
    return 0;
  }

  char line[BUFFER_SIZE];
  char ip[128];
  int port;
  int count = 0;

  // Skip the first line, which is the header
  if (fgets(line, BUFFER_SIZE, file) != NULL) {
    snprintf(result + strlen(result), sizeof(result) - strlen(result), "Listening %s ports:\n", protocol);  
    while (fgets(line, BUFFER_SIZE, file) != NULL) {
      sscanf(line, "%*d: %64[0-9A-Fa-f]:%X %*64[0-9A-Fa-f]:%*X %*X", ip, &port);
      if (strcmp(ip, "00000000") == 0 || strcmp(ip, "0100007F") == 0) { // INADDR_ANY or localhost
        snprintf(result + strlen(result), sizeof(result) - strlen(result), "%s:%d\n", protocol, port);
        ++count;
      }
    }
    if (count == 0) {
      snprintf(result + strlen(result), sizeof(result) - strlen(result), "No %s ports are currently listening.\n", protocol);
    }
  }

  fclose(file);

  return result;
}

char* change_ipaddress(const char *iface, const char *new_ip) {
  static char result[256]; // 存储操作结果
  int fd;
  struct ifreq ifr;

  // 创建socket
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    snprintf(result, sizeof(result), "Failed to open socket\n");
    return result;
  }

  // 设置ifr结构体
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);

  // 设置IP地址
  struct sockaddr_in *ipaddr = (struct sockaddr_in *)&ifr.ifr_addr;
  ipaddr->sin_family = AF_INET;
  inet_pton(AF_INET, new_ip, &ipaddr->sin_addr);

  // 调用ioctl设置IP地址
  if (ioctl(fd, SIOCSIFADDR, &ifr) < 0) {
    close(fd);
    snprintf(result, sizeof(result), "Failed to set IP address\n");
    return result;
  }

  close(fd);
  snprintf(result, sizeof(result), "IP address changed to %s on %s\n", new_ip, iface);
  return result;
}

static void btn_change_ip_event_handler(lv_obj_t *btn) {
  lv_obj_t *input_cont = lv_obj_get_parent(btn);
  lv_obj_t *iface = lv_obj_get_child(input_cont, 1); // 根据创建顺序获取
  lv_obj_t *new_ip = lv_obj_get_child(input_cont, 3); // 同上

  const char *iface_str = lv_textarea_get_text(iface);
  const char *new_ip_str = lv_textarea_get_text(new_ip);

  char *output = change_ipaddress(iface_str, new_ip_str);

  // 找到底部容器并在其中显示结果
  lv_obj_t *bottom_cont = lv_obj_get_child(lv_obj_get_parent(input_cont), 1); // 根据你的布局调整索引
  if (bottom_cont) { // 确认 bottom_cont 存在
    lv_obj_clean(bottom_cont); // 清除 bottom_cont 中的所有子对象

    lv_obj_t *output_label = lv_label_create(bottom_cont);
    lv_label_set_text(output_label, output);
    lv_obj_align(output_label, LV_ALIGN_TOP_LEFT, 0, 0); // 根据需要调整对齐方式
  } else {
    printf("Error: bottom_cont is NULL.\n");
  }
}

static void *capture_packets(void *arg) {
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t *handle = pcap_open_live("lo", MAX_PACKET_SIZE, 1, 1000, errbuf);
  pcap_dumper_t *dumper = pcap_dump_open(handle, "packets_dump.pcap");

  if (handle == NULL) {
    fprintf(stderr, "Couldn't open device lo: %s\n", errbuf);
    return NULL;
  }

  if (dumper == NULL) {
    fprintf(stderr, "Couldn't open dump file: %s\n", pcap_geterr(handle));
    pcap_close(handle);
    return NULL;
  }

  while (is_running) {
    struct pcap_pkthdr *header;
    const u_char *packet;
    int result = pcap_next_ex(handle, &header, &packet);
    if (result == 1) {
      pthread_mutex_lock(&queue_mutex);
      while ((queue_tail + 1) % QUEUE_SIZE == queue_head) {
        pthread_cond_wait(&queue_cond, &queue_mutex);
      }
      packet_queue[queue_tail].size = header->caplen;
      memcpy(packet_queue[queue_tail].data, packet, header->caplen);
      queue_tail = (queue_tail + 1) % QUEUE_SIZE;
      pthread_cond_signal(&queue_cond);
      pthread_mutex_unlock(&queue_mutex);
      pcap_dump((u_char *)dumper, header, packet);
      sleep(1);
    }
  }

  pcap_dump_close(dumper);
  pcap_close(handle);
  return NULL;
}

static void update_text_area(lv_timer_t *timer) {
  pthread_mutex_lock(&queue_mutex);
  while (queue_head != queue_tail) {
    packet_t packet = packet_queue[queue_head];
    queue_head = (queue_head + 1) % QUEUE_SIZE;
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

    char buffer[1024];
    sprintf(buffer, "Packet size: %d\n", packet.size);
    lv_textarea_add_text(text_area, buffer);

    pthread_mutex_lock(&queue_mutex);
  }
  pthread_mutex_unlock(&queue_mutex);
}

static void start_button_clicked(lv_event_t *e) {
  is_running = true;
  pthread_create(&capture_thread, NULL, capture_packets, NULL);
}

static void stop_button_clicked(lv_event_t *e) {
  is_running = false;
  pthread_join(capture_thread, NULL);
}

#endif // LV_BUILD_EXAMPLES && LV_USE_LABEL

