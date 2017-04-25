#include <node.h>
#include <node_buffer.h>

#include "dbenv.h"

#include <cstdlib>
#include <cstring>
#include <uv.h>

using namespace v8;

DbEnv::DbEnv() : _db(0) {};
DbEnv::~DbEnv() {
  close();
};

void DbEnv::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "DbEnv"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "open", Open);
  NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);
  NODE_SET_PROTOTYPE_METHOD(tpl, "_put", Put);
  NODE_SET_PROTOTYPE_METHOD(tpl, "_get", Get);
  NODE_SET_PROTOTYPE_METHOD(tpl, "_del", Del);

  exports->Set(
    String::NewFromUtf8(isolate, "DbEnv"),
    tpl->GetFunction());
}

int DbEnv::open(char const *fname, char const *db, DBTYPE type, u_int32_t flags, int mode) {
  int ret = db_create(&_db, NULL, 0);
  if (ret) return ret;

  //fprintf(stderr, "%p: open %p\n", this, _db);
  return _db->open(_db, NULL, fname, db, type, flags, mode);
}

int DbEnv::close() {
  int ret = 0;
  if (_db && _db->pgsize) {
    //fprintf(stderr, "%p: close %p\n", this, _db);
    ret = _db->close(_db, 0);
    _db = NULL;
  }
  return ret;
}

int DbEnv::put(DBT *key, DBT *data, u_int32_t flags) {
  return _db->put(_db, 0, key, data, flags);
}

int DbEnv::get(DBT *key, DBT *data, u_int32_t flags) {
  return _db->get(_db, 0, key, data, flags);
}

int DbEnv::del(DBT *key, u_int32_t flags) {
  return _db->del(_db, 0, key, flags);
}

void DbEnv::New(const FunctionCallbackInfo<Value>& args) {
  DbEnv* store = new DbEnv();
  store->Wrap(args.This());
  args.GetReturnValue().Set(args.This());
}

void DbEnv::Open(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (! args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First argument must be String")));
    return;
  }

  DbEnv* store = ObjectWrap::Unwrap<DbEnv>(args.This());
  String::Utf8Value fname(args[0]);

  int ret = store->open(*fname, NULL, DB_HASH, DB_CREATE|DB_THREAD, 0);

  args.GetReturnValue().Set(Number::New(isolate, double(ret)));
}


void DbEnv::Close(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  DbEnv* store = ObjectWrap::Unwrap<DbEnv>(args.This());

  int ret = store->close();
  args.GetReturnValue().Set(Number::New(isolate, double(ret)));
}


void DbEnv::Put(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First argument must be a string")));
    return;
  }

  if (!(args.Length() > 1) && ! node::Buffer::HasInstance(args[1])) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Second argument must be a Buffer")));
    return;
  }
  DbEnv* store = ObjectWrap::Unwrap<DbEnv>(args.This());

  String::Utf8Value key(args[0]);
  Local<Object> buf = args[1]->ToObject();

  DBT key_dbt;
  dbt_set(&key_dbt, *key, strlen(*key));
  DBT data_dbt;
  dbt_set(&data_dbt,
          node::Buffer::Data(buf),
          node::Buffer::Length(buf));
  int ret = store->put(&key_dbt, &data_dbt, 0);

  args.GetReturnValue().Set(Number::New(isolate, double(ret)));
}


void DbEnv::Get(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First argument must be a string")));
    return;
  }

  DbEnv* store = ObjectWrap::Unwrap<DbEnv>(args.This());

  String::Utf8Value key(args[0]);

  DBT key_dbt;
  dbt_set(&key_dbt, *key, strlen(*key));

  DBT retbuf;
  dbt_set(&retbuf, 0, 0, DB_DBT_MALLOC);

  store->get(&key_dbt, &retbuf, 0);

  Local<Object> buf = node::Buffer::New(isolate, (char*)retbuf.data, retbuf.size, free_buf, NULL).ToLocalChecked();

  args.GetReturnValue().Set(buf);
}


void DbEnv::Del(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First argument must be a string")));
    return;
  }

  DbEnv* store = ObjectWrap::Unwrap<DbEnv>(args.This());
  String::Utf8Value key(args[0]);
  DBT key_dbt;
  dbt_set(&key_dbt, *key, strlen(*key));
  int ret = store->del(&key_dbt, 0);

  args.GetReturnValue().Set(Number::New(isolate, double(ret)));
}

