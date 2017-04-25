#include <node.h>
#include <nan.h>

#include "dbstore.h"
#include "dbenv.h"

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
  close(0);
};

void DbStore::Init(Local<Object> exports) {
  Nan::HandleScope scope;

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("DbStore").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Nan::SetPrototypeMethod(tpl, "open", Open);
  Nan::SetPrototypeMethod(tpl, "close", Close);
  Nan::SetPrototypeMethod(tpl, "_put", Put);
  Nan::SetPrototypeMethod(tpl, "_get", Get);
  Nan::SetPrototypeMethod(tpl, "_del", Del);

  exports->Set(Nan::New("DbStore").ToLocalChecked(), tpl->GetFunction());
}

DB* DbStore::get_db() {
  return _db;
}

int DbStore::create(DB_ENV *dbenv, u_int32_t flags) {
  return db_create(&_db, dbenv, flags);
}

int DbStore::open(char const *fname, char const *db, DBTYPE type, u_int32_t flags, int mode) {
  //fprintf(stderr, "%p: open %p\n", this, _db);
  return _db->open(_db, NULL, fname, db, type, flags, mode);
}

int DbStore::close(u_int32_t flags) {
  int ret = 0;
  if (_db && _db->pgsize) {
    //fprintf(stderr, "%p: close %p\n", this, _db);
    ret = _db->close(_db, flags);
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

void DbStore::New(const Nan::FunctionCallbackInfo<Value>& args) {
  DbStore* store = new DbStore();
  store->Wrap(args.This());

  DB_ENV* _dbenv = NULL;
  if (args.Length() > 0) {
    if (! args[0]->IsObject()) {
      return Nan::ThrowTypeError("First argument must be an object");
    }
    DbEnv* dbenv = Nan::ObjectWrap::Unwrap<DbEnv>(args[0]->ToObject());
    _dbenv = dbenv->get_env();
  }
  
  int ret = store->create(_dbenv, 0);

  if (ret) {
    return Nan::ThrowTypeError("Could not create DB object");
  }

  args.GetReturnValue().Set(args.This());
}

void DbStore::Open(const Nan::FunctionCallbackInfo<Value>& args) {
  if (! args[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be String");
    return;
  }

  DbStore* store = Nan::ObjectWrap::Unwrap<DbStore>(args.This());
  String::Utf8Value fname(args[0]);
  int ret = store->open(*fname, NULL, DB_HASH, DB_CREATE|DB_THREAD, 0);
  args.GetReturnValue().Set(Nan::New(double(ret)));
}

void DbStore::Close(const Nan::FunctionCallbackInfo<Value>& args) {
  DbStore* store = Nan::ObjectWrap::Unwrap<DbStore>(args.This());
  int ret = store->close(0);
  args.GetReturnValue().Set(Nan::New(double(ret)));
}

void DbStore::Put(const Nan::FunctionCallbackInfo<Value>& args) {
  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }

  if (!(args.Length() > 1) && ! node::Buffer::HasInstance(args[1])) {
    Nan::ThrowTypeError("Second argument must be a Buffer");
    return;
  }
  DbStore* store = Nan::ObjectWrap::Unwrap<DbStore>(args.This());

  String::Utf8Value key(args[0]);
  Local<Object> buf = args[1]->ToObject();

  DBT key_dbt;
  dbt_set(&key_dbt, *key, strlen(*key));
  DBT data_dbt;
  dbt_set(&data_dbt,
          node::Buffer::Data(buf),
          node::Buffer::Length(buf));

  int ret = store->put(&key_dbt, &data_dbt, 0);

  args.GetReturnValue().Set(Nan::New(double(ret)));
}

void DbStore::Get(const Nan::FunctionCallbackInfo<Value>& args) {
  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }

  DbStore* store = Nan::ObjectWrap::Unwrap<DbStore>(args.This());

  String::Utf8Value key(args[0]);

  DBT key_dbt;
  dbt_set(&key_dbt, *key, strlen(*key));
  DBT retbuf;
  dbt_set(&retbuf, 0, 0, DB_DBT_MALLOC);

  store->get(&key_dbt, &retbuf, 0);

  Local<Object> buf = Nan::NewBuffer((char*)retbuf.data, retbuf.size, free_buf, NULL).ToLocalChecked();

  args.GetReturnValue().Set(buf);
}

void DbStore::Del(const Nan::FunctionCallbackInfo<Value>& args) {
  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }

  DbStore* store = Nan::ObjectWrap::Unwrap<DbStore>(args.This());

  String::Utf8Value key(args[0]);
  
  DBT key_dbt;
  dbt_set(&key_dbt, *key, strlen(*key));
  
  int ret = store->del(&key_dbt, 0);

  args.GetReturnValue().Set(Nan::New(double(ret)));
}

