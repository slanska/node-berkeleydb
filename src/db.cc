#include <node.h>
#include <nan.h>

#include "db.h"
#include "dbenv.h"
#include "dbtxn.h"

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

Db::Db() : _db(0) {};
Db::~Db() {
  close(0);
};

void Db::Init(Local<Object> exports) {
  Nan::HandleScope scope;

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Db").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Nan::SetPrototypeMethod(tpl, "open", Open);
  Nan::SetPrototypeMethod(tpl, "close", Close);
  Nan::SetPrototypeMethod(tpl, "_put", Put);
  Nan::SetPrototypeMethod(tpl, "_get", Get);
  Nan::SetPrototypeMethod(tpl, "_del", Del);

  exports->Set(Nan::New("Db").ToLocalChecked(), tpl->GetFunction());
}

DB* Db::get_db() {
  return _db;
}

int Db::create(DB_ENV *dbenv, u_int32_t flags) {
  return db_create(&_db, dbenv, flags);
}

int Db::open(DB_TXN *txnid, char const *fname, char const *db, DBTYPE type, u_int32_t flags, int mode) {
  // fprintf(stderr, "Db::open - txnid = %p\n", txnid);
  return _db->open(_db, txnid, fname, db, type, flags, mode);
}

int Db::close(u_int32_t flags) {
  int ret = 0;
  if (_db && _db->pgsize) {
    //fprintf(stderr, "%p: close %p\n", this, _db);
    ret = _db->close(_db, flags);
    _db = NULL;
  }
  return ret;
}

DB_ENV* Db::get_env() {
  return _db->get_env(_db);
}

int Db::put(DB_TXN *txn, DBT *key, DBT *data, u_int32_t flags) {
  // fprintf(stderr, "Db::put - txn = %p\n", txn);
  return _db->put(_db, txn, key, data, flags);
}

int Db::get(DB_TXN *txn, DBT *key, DBT *data, u_int32_t flags) {
  return _db->get(_db, txn, key, data, flags);
}

int Db::del(DB_TXN *txn, DBT *key, u_int32_t flags) {
  return _db->del(_db, txn, key, flags);
}

void Db::New(const Nan::FunctionCallbackInfo<Value>& args) {
  Db* store = new Db();
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

void Db::Open(const Nan::FunctionCallbackInfo<Value>& args) {
  if (! args[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be String");
    return;
  }

  Db* store = Nan::ObjectWrap::Unwrap<Db>(args.This());
  DB_TXN *tid = 0;

  DB_ENV *env = store->get_env();
  if (env) {
    env->txn_begin(env, NULL, &tid, 0);
  }
  String::Utf8Value fname(args[0]);
  int ret = store->open(tid, *fname, NULL, DB_HASH, DB_CREATE|DB_THREAD, 0);
  tid->commit(tid, 0);
  args.GetReturnValue().Set(Nan::New(double(ret)));
}

void Db::Close(const Nan::FunctionCallbackInfo<Value>& args) {
  Db* store = Nan::ObjectWrap::Unwrap<Db>(args.This());
  int ret = store->close(0);
  args.GetReturnValue().Set(Nan::New(double(ret)));
}

void Db::Put(const Nan::FunctionCallbackInfo<Value>& args) {
  // fprintf(stderr, "Db::Put (start) - args[2] = %p\n", *args[2]);
  DB_TXN * dbtxn = NULL;

  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }

  if (!(args.Length() > 1) && ! node::Buffer::HasInstance(args[1])) {
    Nan::ThrowTypeError("Second argument must be a Buffer");
    return;
  }

  if (args.Length() > 2) {
    if (! args[2]->IsObject()) {
      Nan::ThrowTypeError("Third argument must be a Object");
      return;
    }
    Local<Object> wrapped = args[2]->ToObject();
    DbTxn *_dbtxn = Nan::ObjectWrap::Unwrap<DbTxn>(wrapped);
    dbtxn = _dbtxn->get_txn();
    // fprintf(stderr, "Db::Put - dbtxn = %p, _ = %p (wrapped %p)\n", dbtxn, _dbtxn, *wrapped);
  }
  Db* store = Nan::ObjectWrap::Unwrap<Db>(args.This());

  String::Utf8Value key(args[0]);
  Local<Object> buf = args[1]->ToObject();
  
  DBT * key_dbt = new DBT();
  dbt_set(key_dbt, *key, strlen(*key));
  DBT * data_dbt = new DBT();
  dbt_set(data_dbt,
          node::Buffer::Data(buf),
          node::Buffer::Length(buf));

  int ret = store->put(dbtxn, key_dbt, data_dbt, 0);

  args.GetReturnValue().Set(Nan::New(double(ret)));
}

void Db::Get(const Nan::FunctionCallbackInfo<Value>& args) {
  DB_TXN * dbtxn = NULL;

  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }

  if (args.Length() > 1) {
    if (! args[1]->IsObject()) {
      Nan::ThrowTypeError("Second argument must be a Object");
      return;
    }
    DbTxn *_dbtxn = Nan::ObjectWrap::Unwrap<DbTxn>(args[1]->ToObject());
    dbtxn = _dbtxn->get_txn();
  }

  Db* store = Nan::ObjectWrap::Unwrap<Db>(args.This());

  String::Utf8Value key(args[0]);

  DBT key_dbt;
  dbt_set(&key_dbt, *key, strlen(*key));
  DBT retbuf;
  dbt_set(&retbuf, 0, 0, DB_DBT_MALLOC);

  store->get(dbtxn, &key_dbt, &retbuf, 0);

  Local<Object> buf = Nan::NewBuffer((char*)retbuf.data, retbuf.size, free_buf, NULL).ToLocalChecked();

  args.GetReturnValue().Set(buf);
}

void Db::Del(const Nan::FunctionCallbackInfo<Value>& args) {
  DB_TXN * dbtxn = NULL;

  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }

  if (args.Length() > 1) {
    if (! args[1]->IsObject()) {
      Nan::ThrowTypeError("Second argument must be a Object");
      return;
    }
    DbTxn *_dbtxn = Nan::ObjectWrap::Unwrap<DbTxn>(args[1]->ToObject());
    dbtxn = _dbtxn->get_txn();
  }

  Db* store = Nan::ObjectWrap::Unwrap<Db>(args.This());

  String::Utf8Value key(args[0]);
  
  DBT key_dbt;
  dbt_set(&key_dbt, *key, strlen(*key));
  
  int ret = store->del(dbtxn, &key_dbt, 0);

  args.GetReturnValue().Set(Nan::New(double(ret)));
}

