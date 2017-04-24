#ifndef WORKBATON_H
#define WORKBATON_H

#include <node.h>
#include <db.h>
#include <uv.h>

#include "dbstore.h"

struct WorkBaton {
  uv_work_t *req;
  DbStore *store;
  v8::Isolate *isolate;

  char *str_arg;
  v8::Persistent<v8::Value> data;
  v8::Persistent<v8::Function> callback;

  char const *call;
  DBT inbuf;
  DBT retbuf;
  int ret;

  WorkBaton(uv_work_t *_r, DbStore *_s);
  ~WorkBaton();
};

WorkBaton::WorkBaton(uv_work_t *_r, DbStore *_s) : req(_r), store(_s), str_arg(0) {
  fprintf(stderr, "new WorkBaton %p/%p\n", this, req);
}

WorkBaton::~WorkBaton() {
  fprintf(stderr, "~WorkBaton %p/%p %s %s\n", this, req, call, str_arg);
  delete req;

  if (str_arg) free(str_arg);
  data.Reset();
  callback.Reset();
  // Ignore retbuf since it will be freed by Buffer
}

#endif
