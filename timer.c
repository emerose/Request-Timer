/**
 *
 * @file   timer.c
 * @brief  times as precisely as possible the length of time an HTTP request takes
 *
 *
 *
 * @author  Sam Quigley <quigley@emerose.com>
 * Copyright (c) 2009, All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <event.h>
#include <evhttp.h>
#include "cpucycles/cpucycles.h"

/* why, yes. this *is* a pile of global variables. thanks for noticing! */

static struct evhttp_connection *conn;
static struct evhttp_request    *req;

static long long  cycles = 0;
static char      *ip;
static int        port;
static char      *cookie;
static char      *path;
static long       count = 0;

#define SETTLE_TIME 10000                          /* usec to wait between reqs */

void allow_things_to_settle(void);
void make_new_request(int fd, short type, void *args);
void request_finished(struct evhttp_request *req, void *args);

void request_finished(struct evhttp_request *req, void *args) {
  long long newcycles = cpucycles();

  if (cycles) {
    fprintf(stdout, "%lld\n", newcycles - cycles);
  }
  cycles = newcycles;

  count--;

  if (count) {
    allow_things_to_settle();
  } else {
    event_loopbreak();                          /* we're done. */
  }
}

void allow_things_to_settle() {
  struct timeval delay;
  delay.tv_sec = 0;
  delay.tv_usec = SETTLE_TIME;

  event_once(-1, EV_TIMEOUT, make_new_request, NULL, &delay);
}

void make_new_request(int fd, short type, void *args) {
  req  = evhttp_request_new(request_finished, NULL);
  assert(req);
  evhttp_add_header(req->output_headers, "Cookie", cookie);

  assert(evhttp_make_request(conn, req, EVHTTP_REQ_GET, path) != -1);
}

int main ( int argc, char *argv[] ) {
  if (argc != 6) {
    fprintf(stderr, "Usage: timer [IP] [port] [path] [cookie] [count]\n");
    exit(1);
  }

  ip     = argv[1];
  port   = atoi(argv[2]);
  path   = argv[3];
  cookie = argv[4];
  count  = atol(argv[5]);
  fprintf(stderr, "Making %ld requests to:\n\tIP: %s\n\tPort: %d\n\tPath: %s\n\tCookie: %s\n", count, ip, port, path, cookie);

  event_init();

  conn = evhttp_connection_new(ip, port);
  assert(conn);

  allow_things_to_settle();
  
	event_dispatch();

  fprintf(stderr, "Done.");
  evhttp_connection_free(conn);
  return 0;
}
