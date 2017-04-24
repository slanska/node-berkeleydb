#include <node.h>
#include <node_buffer.h>

#include "dbstore.h"

#include "workbaton.h"

#include <cstdlib>
#include <cstring>
#include <uv.h>

using namespace v8;

static void free_buf(char *data, void *hint) {
  //fprintf(stderr, "Free %p\n", data);
  free(data);
}

static void dbt_set(DBT *dbt, void *data, u_int32_t size, u_int32_t flags = DB_DBT_USERMEM) {
  memset(dbt, 0, sizeof(*dbt));
  dbt->data = data;
  dbt->size = size;
  dbt->flags = flags;
}

DbStore::DbStore() : _db(0) {};
DbStore::~DbStore() {
  //fprintf(stderr, "~DbStore %p\n", this);
  close();
};

void DbStore::Init(Local<Object> target) {
  Isolate* isolate = target->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "DbStore"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "open", Open);
  NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);
  NODE_SET_PROTOTYPE_METHOD(tpl, "_put", Put);
  NODE_SET_PROTOTYPE_METHOD(tpl, "_get", Get);
  NODE_SET_PROTOTYPE_METHOD(tpl, "del", Del);

  target->Set(
    String::NewFromUtf8(isolate, "DbStore"),
    tpl->GetFunction());
}

void DbStore::New(const FunctionCallbackInfo<Value>& args) {
  DbStore* obj = new DbStore();
  obj->Wrap(args.This());
  args.GetReturnValue().Set(args.This());
}


int DbStore::open(char const *fname, char const *db, DBTYPE type, u_int32_t flags, int mode) {
  int ret = db_create(&_db, NULL, 0);
  if (ret) return ret;

  //fprintf(stderr, "%p: open %p\n", this, _db);
  return _db->open(_db, NULL, fname, db, type, flags, mode);
}

int DbStore::close() {
  int ret = 0;
  if (_db && _db->pgsize) {
    //fprintf(stderr, "%p: close %p\n", this, _db);
    ret = _db->close(_db, 0);
    _db = NULL;
  }
  return ret;
}

int DbStore::put(DBT *key, DBT *data, u_int32_t flags) {
  return _db->put(_db, 0, key, data, flags);
}

int DbStore::get(DBT *key, DBT *data, u_int32_t flags) {
  return _db->get(_db, 0, key, data, flags);
}

int DbStore::del(DBT *key, u_int32_t flags) {
  return _db->del(_db, 0, key, flags);
}


static void After(WorkBaton *baton, Local<Value> *argv, int argc) {
  Isolate *isolate = baton->isolate;
  HandleScope scope(isolate);

  if (baton->ret) {
    //fprintf(stderr, "%s %s error %d\n", baton->call, baton->str_arg, baton->ret);
    argv[0] = node::UVException(0, baton->call, db_strerror(baton->ret));
  } else {
    argv[0] = { Null(isolate) };
  }

  // surround in a try/catch for safety
  TryCatch try_catch(isolate);

  // execute the callback function
  //fprintf(stderr, "%p.%s Calling cb\n", baton->store, call);
  baton->callback.Get(isolate)->Call(isolate->GetCurrentContext()->Global(), argc, argv);

  if (try_catch.HasCaught())
    node::FatalException(isolate, try_catch);

  delete baton;
}


void OpenWork(uv_work_t *req) {
  WorkBaton *baton = (WorkBaton *) req->data;

  DbStore *store = baton->store;
  baton->call = "open";
  baton->ret = store->open(baton->str_arg, NULL, DB_BTREE, DB_CREATE|DB_THREAD, 0);
}

void OpenAfter(uv_work_t *req, int status) {
  // fetch our data structure
  WorkBaton *baton = (WorkBaton *)req->data;

  // create an arguments array for the callback
  Local<Value> argv[1];

  After(baton, argv, 1);
}

void DbStore::Open(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  DbStore* obj = ObjectWrap::Unwrap<DbStore>(args.This());

  if (! args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First argument must be String")));
    return;
  }

  if (! args[1]->IsFunction()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Second argument must be callback function")));
    return;
  }

  // create an async work token
  uv_work_t *req = new uv_work_t;

  // assign our data structure that will be passed around
  WorkBaton *baton = new WorkBaton(req, obj);
  req->data = baton;

  String::Utf8Value fname(args[0]);
  baton->isolate = isolate;
  baton->str_arg = strdup(*fname);
  baton->callback.Reset(isolate, Local<Function>::Cast(args[1]));

  uv_queue_work(uv_default_loop(), req, OpenWork, (uv_after_work_cb)OpenAfter);

  args.GetReturnValue().Set(args.This());
}


static void CloseWork(uv_work_t *req) {
  WorkBaton *baton = (WorkBaton *) req->data;

  DbStore *store = baton->store;
  baton->call = "close";
  baton->ret = store->close();
}

static void CloseAfter(uv_work_t *req, int status) {
  // fetch our data structure
  WorkBaton *baton = (WorkBaton *)req->data;

  // create an arguments array for the callback
  Local<Value> argv[1];
  After(baton, argv, 1);
}

void DbStore::Close(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  DbStore* obj = ObjectWrap::Unwrap<DbStore>(args.This());

  if (! args[0]->IsFunction()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument must be callback function")));
    return;
  }

  // create an async work token
  uv_work_t *req = new uv_work_t;

  // assign our data structure that will be passed around
  WorkBaton *baton = new WorkBaton(req, obj);
  req->data = baton;

  baton->isolate = isolate;
  baton->callback.Reset(isolate, Local<Function>::Cast(args[0]));

  uv_queue_work(uv_default_loop(), req, CloseWork, (uv_after_work_cb)CloseAfter);

  args.GetReturnValue().Set(args.This());
}


static void PutWork(uv_work_t *req) {
  WorkBaton *baton = (WorkBaton *) req->data;
  //fprintf(stderr, "DbStore::PutWork baton %p:%p\n", baton, req);

  DbStore *store = baton->store;

  DBT key_dbt;
  dbt_set(&key_dbt, baton->str_arg, strlen(baton->str_arg));
  DBT &data_dbt = baton->inbuf;

  baton->call = "put";
  //fprintf(stderr, "put %s %p[%d]\n", baton->str_arg, data_dbt.data, data_dbt.size);
  baton->ret = store->put(&key_dbt, &data_dbt, 0);
}

static void PutAfter(uv_work_t *req, int status) {
  // fetch our data structure
  WorkBaton *baton = (WorkBaton *)req->data;
  //fprintf(stderr, "DbStore::PutAfter baton %p:%p\n", baton, req);

  // create an arguments array for the callback
  Local<Value> argv[1];
  After(baton, argv, 1);
}

void DbStore::Put(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  DbStore* obj = ObjectWrap::Unwrap<DbStore>(args.This());

  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First argument must be a string")));
    return;
  }
  String::Utf8Value key(args[0]);

  if (!(args.Length() > 1) && ! node::Buffer::HasInstance(args[1])) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Second argument must be a Buffer")));
    return;
  }
  Local<Object> buf = args[1]->ToObject();

  if (!(args.Length() > 2) && ! args[2]->IsFunction()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument must be callback function")));
    return;
  }
  Local<Function> cb = Local<Function>::Cast(args[2]);

  // create an async work token
  uv_work_t *req = new uv_work_t;

  // assign our data structure that will be passed around
  WorkBaton *baton = new WorkBaton(req, obj);
  //fprintf(stderr, "DbStore::Put %p baton %p\n", req, baton);
  req->data = baton;

  baton->isolate = isolate;
  baton->str_arg = strdup(*key);

  dbt_set(&baton->inbuf,
          node::Buffer::Data(buf),
          node::Buffer::Length(buf));

  baton->data.Reset(isolate, buf); // Ensure not GCed until complete
  baton->callback.Reset(isolate, cb);

  uv_queue_work(uv_default_loop(), req, PutWork, (uv_after_work_cb)PutAfter);

  args.GetReturnValue().Set(args.This());
}


static void GetWork(uv_work_t *req) {
  WorkBaton *baton = (WorkBaton *) req->data;

  DbStore *store = baton->store;

  DBT key_dbt;
  dbt_set(&key_dbt, baton->str_arg, strlen(baton->str_arg));

  DBT *retbuf = &baton->retbuf;
  dbt_set(retbuf, 0, 0, DB_DBT_MALLOC);

  baton->call = "get";
  baton->ret = store->get(&key_dbt, retbuf, 0);
  //fprintf(stderr, "get %s => %p[%d]\n", baton->str_arg, key_dbt.data, key_dbt.size);
}

static void GetAfter(uv_work_t *req, int status) {
  // fetch our data structure
  WorkBaton *baton = (WorkBaton *)req->data;
  Isolate *isolate = baton->isolate;
  HandleScope scope(isolate);

  // create an arguments array for the callback
  Local<Value> argv[2];

  DBT *retbuf = &baton->retbuf;
  Local<Object> buf = node::Buffer::New(isolate, (char*)retbuf->data, retbuf->size, free_buf, NULL).ToLocalChecked();
  argv[1] = buf;
  After(baton, argv, 2);
}

void DbStore::Get(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  DbStore* obj = ObjectWrap::Unwrap<DbStore>(args.This());

  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First argument must be a string")));
    return;
  }
  String::Utf8Value key(args[0]);

  if (!(args.Length() > 1) && ! args[1]->IsFunction()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument must be callback function")));
    return;
  }

  // create an async work token
  uv_work_t *req = new uv_work_t;

  // assign our data structure that will be passed around
  WorkBaton *baton = new WorkBaton(req, obj);
  req->data = baton;

  baton->isolate = isolate;
  baton->str_arg = strdup(*key);
  baton->callback.Reset(isolate, Local<Function>::Cast(args[1]));

  uv_queue_work(uv_default_loop(), req, GetWork, (uv_after_work_cb)GetAfter);

  args.GetReturnValue().Set(args.This());
}


static void DelWork(uv_work_t *req) {
  WorkBaton *baton = (WorkBaton *) req->data;

  DbStore *store = baton->store;

  DBT key_dbt;
  dbt_set(&key_dbt, baton->str_arg, strlen(baton->str_arg));

  fprintf(stderr, "%p/%p: del %s\n", baton, req, baton->str_arg);
  baton->call = "del";
  baton->ret = store->del(&key_dbt, 0);
  //fprintf(stderr, "%p/%p: del %s => %d\n", baton, req, baton->str_arg, baton->ret);
}

void DbStore::Del(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  DbStore* obj = ObjectWrap::Unwrap<DbStore>(args.This());

  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First argument must be a string")));
    return;
  }
  String::Utf8Value key(args[0]);

  if (!(args.Length() > 1) && ! args[1]->IsFunction()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument must be callback function")));
    return;
  }

  // create an async work token
  uv_work_t *req = new uv_work_t;

  // assign our data structure that will be passed around
  WorkBaton *baton = new WorkBaton(req, obj);
  req->data = baton;

  baton->isolate = isolate;
  baton->str_arg = strdup(*key);
  baton->callback.Reset(isolate, Local<Function>::Cast(args[1]));

  uv_queue_work(uv_default_loop(), req, DelWork, (uv_after_work_cb)PutAfter); // Yes, use the same.

  args.GetReturnValue().Set(args.This());
}

