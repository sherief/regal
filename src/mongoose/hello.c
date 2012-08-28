#include <stdio.h>
#include <string.h>
#include "mongoose.h"

static void *callback(enum mg_event event,
                      struct mg_connection *conn)
{
  if (event == MG_NEW_REQUEST) {
    char content[1024];
    int content_length = snprintf(content, sizeof(content),
                                  "<html><body><b>Hello from mongoose!</b><br/>Remote port: %d</body></html>",
                                  mg_get_request_info(conn)->remote_port);
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %d\r\n"        // Always set Content-Length
              "\r\n"
              "%s",
              content_length, content);
    // Mark as processed
    return "";
  } else {
    return NULL;
  }
}

int main(void) {
  struct mg_context *ctx;
  const char *options[] = {"listening_ports", "8080", NULL};

  ctx = mg_start(&callback, NULL, options);
  getchar();  // Wait until user hits "enter"
  mg_stop(ctx);

  return 0;
}
