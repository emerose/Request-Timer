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

static long long  counter = 0;
static char      *ip;
static int        port;
static char      *cookie;
static char      *path;

void make_new_request(void);

void request_finished(struct evhttp_request *req, void *arg) {
  long long newcounter = cpucycles();

  if (counter) {
    fprintf(stdout, "%lld\n", newcounter - counter);
  }
  counter = newcounter;

  make_new_request();
}

void make_new_request(void) {
  req  = evhttp_request_new(request_finished, NULL);
  assert(req);
  evhttp_add_header(req->output_headers, "Cookie", cookie);

  assert(evhttp_make_request(conn, req, EVHTTP_REQ_GET, path) != -1);
}

int main ( int argc, char *argv[] ) {
  if (argc != 4) {
    fprintf(stderr, "Usage: timer [IP] [port] [path] [cookie]\n");
    exit(1);
  }

  ip     = argv[1];
  port   = atoi(argv[2]);
  path   = argv[3];
  cookie = argv[4];
  fprintf(stderr, "Connecting to:\n\tIP: %s\n\tPort: %d\n\tPath: %s\n\tCookie: %s\n", ip, port, path, cookie);

  event_init();

  conn = evhttp_connection_new(ip, port);
  assert(conn);

  make_new_request();
  
	event_dispatch();

  evhttp_connection_free(conn);
  return 0;
}
