#include "../fd_ballet.h"
#include "fd_http_server.h"
#include <malloc.h>
#include <signal.h>

static volatile int stopflag = 0;
static void sighandler(int sig) {
  (void)sig;
  stopflag = 1;
}

static void response_free( uchar const * orig_body, void * free_ctx ) {
  (void)free_ctx;
  free((void*)orig_body);
}

static fd_http_server_response_t
request_get( ulong connection_id, char const * path, void * ctx ) {
  FD_LOG_NOTICE(( "GET id=%lu path=\"%s\" ctx=%lx", connection_id, path, (ulong)ctx ));
  static const char* TEXT = "{\"jsonrpc\": \"2.0\", \"result\": {\"absoluteSlot\": 166598, \"blockHeight\": 166500, \"epoch\": 27, \"slotIndex\": 2790, \"slotsInEpoch\": 8192, \"transactionCount\": 22661093}, \"id\": 1}";
  fd_http_server_response_t response = {
    .status = 200,
    .upgrade_websocket = 0,
    .content_type = "application/json",
    .body = (const uchar*)strdup(TEXT),
    .body_len = strlen(TEXT),
    .body_free = response_free
  };
  return response;
}

static fd_http_server_response_t
request_post( ulong connection_id, char const * path, uchar const * data, ulong data_len, void * ctx ) {
  FD_LOG_NOTICE(( "POST id=%lu path=\"%s\" ctx=%lx", connection_id, path, (ulong)ctx ));
  fwrite(">>>", 1, 3, stdout);
  fwrite(data, 1, data_len, stdout);
  printf("<<<\n");
  static const char* TEXT = "{\"jsonrpc\": \"2.0\", \"result\": {\"absoluteSlot\": 166598, \"blockHeight\": 166500, \"epoch\": 27, \"slotIndex\": 2790, \"slotsInEpoch\": 8192, \"transactionCount\": 22661093}, \"id\": 1}";
  fd_http_server_response_t response = {
    .status = 200,
    .upgrade_websocket = 0,
    .content_type = "application/json",
    .body = (const uchar*)strdup(TEXT),
    .body_len = strlen(TEXT),
    .body_free = response_free
  };
  return response;
}

int
main( int     argc,
      char ** argv ) {
  fd_boot( &argc, &argv );

  fd_http_server_params_t params = {
    .max_connection_cnt = 5,
    .max_ws_connection_cnt = 2,
    .max_request_len = 1<<16,
    .max_ws_recv_frame_len = 2048,
    .max_ws_send_frame_cnt = 100
  };
  fd_http_server_callbacks_t callbacks = {
    .request_get = request_get,
    .request_post = request_post,
    .close = NULL,
    .ws_open = NULL,
    .ws_close = NULL,
    .ws_message = NULL
  };
  void* server_mem = aligned_alloc( fd_http_server_align(), fd_http_server_footprint( params ) );
  fd_http_server_t * server = fd_http_server_join( fd_http_server_new( server_mem, params, callbacks, NULL ) );

  FD_TEST( fd_http_server_listen( server, 4321U ) != NULL );

  FD_LOG_NOTICE(( "try running\ncurl http://localhost:4321/hello/from/the/magic/tavern\ncurl http://localhost:4321 -X POST -H \"Content-Type: application/json\" -d '{ \"jsonrpc\": \"2.0\", \"id\": 1, \"method\": \"getAccountInfo\", \"params\": [ \"vines1vzrYbzLMRdu58ou5XTby4qAqVRLmqo36NKPTg\", { \"encoding\": \"base58\" } ] }'" ));

  signal( SIGINT, sighandler );
  while( !stopflag ) {
    fd_http_server_poll( server );
  }

  free( fd_http_server_delete( fd_http_server_leave( server ) ) );

  FD_LOG_NOTICE(( "pass" ));
  fd_halt();
  return 0;
}
