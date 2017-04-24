#include <node.h>
#include <node_buffer.h>

#include "dbstore.h"

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
  NODE_SET_PROTOTYPE_METHOD(tpl, "_del", Del);

  target->Set(
    String::NewFromUtf8(isolate, "DbStore"),
    tpl->GetFunction());
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

void DbStore::New(const FunctionCallbackInfo<Value>& args) {
  DbStore* obj = new DbStore();
  obj->Wrap(args.This());
  args.GetReturnValue().Set(args.This());
}

void DbStore::Open(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (! args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First argument must be String")));
    return;
  }

  DbStore* store = ObjectWrap::Unwrap<DbStore>(args.This());
  String::Utf8Value fname(args[0]);

  int ret = store->open(*fname, NULL, DB_BTREE, DB_CREATE|DB_THREAD, 0);

  args.GetReturnValue().Set(Number::New(isolate, double(ret)));
}


void DbStore::Close(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  DbStore* store = ObjectWrap::Unwrap<DbStore>(args.This());

  int ret = store->close();
  args.GetReturnValue().Set(Number::New(isolate, double(ret)));
}


void DbStore::Put(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First argument must be a string")));
    return;
  }

  if (!(args.Length() > 1) && ! node::Buffer::HasInstance(args[1])) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Second argument must be a Buffer")));
    return;
  }
  DbStore* store = ObjectWrap::Unwrap<DbStore>(args.This());

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


void DbStore::Get(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First argument must be a string")));
    return;
  }

  DbStore* store = ObjectWrap::Unwrap<DbStore>(args.This());

  String::Utf8Value key(args[0]);

  DBT key_dbt;
  dbt_set(&key_dbt, *key, strlen(*key));

  DBT retbuf;
  dbt_set(&retbuf, 0, 0, DB_DBT_MALLOC);

  store->get(&key_dbt, &retbuf, 0);

  Local<Object> buf = node::Buffer::New(isolate, (char*)retbuf.data, retbuf.size, free_buf, NULL).ToLocalChecked();

  args.GetReturnValue().Set(buf);
}


void DbStore::Del(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First argument must be a string")));
    return;
  }

  DbStore* store = ObjectWrap::Unwrap<DbStore>(args.This());
  String::Utf8Value key(args[0]);
  DBT key_dbt;
  dbt_set(&key_dbt, *key, strlen(*key));
  int ret = store->del(&key_dbt, 0);

  args.GetReturnValue().Set(Number::New(isolate, double(ret)));
}

