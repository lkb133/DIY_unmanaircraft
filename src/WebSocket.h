#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_mac.h>
#include <esp_netif.h>
#include <sys/socket.h>
#include <esp_http_server.h>
#include <esp_eth.h>

#define null NULL
static const char *TAG = "main";
static esp_err_t get_handler(httpd_req_t *req);
static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req);

// 注册路由
static const httpd_uri_t ws = {
    .uri = "/ws",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = null,
    .is_websocket = true};

// 创建http服务器
static httpd_handle_t start_webserver(void)
{
  httpd_handle_t server = null;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  printf("Starting server on port: '%d'\n", config.server_port);
  if (httpd_start(&server, &config) == ESP_OK)
  {
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &ws);
    return server;
  }
  printf("ERROR starting server\n");
  return null;
}

// 路由处理
// 异步发送
static esp_err_t get_handler(httpd_req_t *req)
{
  // 断开握手
  if (req->method == HTTP_GET)
  {
    printf("Handshake done, the new connection was opened\n");
    return ESP_OK;
  }
  httpd_ws_frame_t ws_pkt;
  uint8_t *buf = null;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;
  // 读取数据
  esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
  if (ret != ESP_OK)
  {
    printf("httpd_ws_recv_frame failed to get frame len with %d\n", ret);
    return ret;
  }
  printf("frame len is %d\n", ws_pkt.len);
  if (ws_pkt.len)
  {
    /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
    buf = (uint8_t *)calloc(1, ws_pkt.len + 1);
    if (buf == null)
    {
      printf("Failed to calloc memory for buf\n");
      return ESP_ERR_NO_MEM;
    }
    ws_pkt.payload = buf;
    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
    if (ret != ESP_OK)
    {
      printf("httpd_ws_recv_frame failed with %d\n", ret);
      free(buf);
      return ret;
    }
    printf("Got packet with message: %s\n", ws_pkt.payload);
  }
  printf("Packet type: %d\n", ws_pkt.type);
  // 消息为Trigger async则异步发送 trigger_async_send
  if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp((char *)ws_pkt.payload, "Trigger async") == 0)
  {
    free(buf);
    return trigger_async_send(req->handle, req);
  }
  // 如果不是特殊指令，直接回传数据
  ret = httpd_ws_send_frame(req, &ws_pkt);
  if (ret != ESP_OK)
  {
    printf("httpd_ws_send_frame failed with %d\n", ret);
  }
  free(buf);
  return ret;
}

/*
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */
struct async_resp_arg
{
  httpd_handle_t hd;
  int fd;
};
/*
 * async send function, which we put into the httpd work queue
 */
static void ws_async_send(void *arg)
{
  // 防止空指针
  if (arg == null)
  {
    return;
  }
  static const char *data = "Async data";
  // struct async_resp_arg *resp_arg = null; 空指针
  struct async_resp_arg *resp_arg = (struct async_resp_arg *)arg;
  httpd_handle_t hd = resp_arg->hd;
  int fd = resp_arg->fd;
  httpd_ws_frame_t ws_pkt;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.payload = (uint8_t *)data;
  ws_pkt.len = strlen(data);
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;

  httpd_ws_send_frame_async(hd, fd, &ws_pkt);
  free(resp_arg);
}

static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req)
{
  struct async_resp_arg *resp_arg = (struct async_resp_arg *)malloc(sizeof(struct async_resp_arg));
  if (resp_arg == NULL)
  {
    return ESP_ERR_NO_MEM;
  }
  resp_arg->hd = req->handle;
  resp_arg->fd = httpd_req_to_sockfd(req);
  esp_err_t ret = httpd_queue_work(handle, ws_async_send, resp_arg);
  if (ret != ESP_OK)
  {
    free(resp_arg);
  }
  return ret;
}
static esp_err_t stop_webserver(httpd_handle_t server)
{
  // Stop the httpd server
  return httpd_stop(server);
}