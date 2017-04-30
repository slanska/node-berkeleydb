#include <node.h>
#include <nan.h>
#include <db.h>

#include "dbcursor.h"
#include "db.h"
#include "dbtxn.h"
#include "util.h"

#include <cstdlib>
#include <cstring>
#include <uv.h>

using namespace v8;

DbCursor::DbCursor() : _dbc(0) {};
DbCursor::~DbCursor() {
  close();
};

void DbCursor::Init(Local<Object> exports) {
  Nan::HandleScope scope;

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("DbCursor").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Nan::SetPrototypeMethod(tpl, "count", Count);
  Nan::SetPrototypeMethod(tpl, "close", Close);
  Nan::SetPrototypeMethod(tpl, "_put", Put);
  
  Nan::SetPrototypeMethod(tpl, "_set", Set);
  Nan::SetPrototypeMethod(tpl, "_current", Current);
  Nan::SetPrototypeMethod(tpl, "_next", Next);
  Nan::SetPrototypeMethod(tpl, "_prev", Prev);
  Nan::SetPrototypeMethod(tpl, "_first", First);
  Nan::SetPrototypeMethod(tpl, "_last", Last);

  Nan::SetPrototypeMethod(tpl, "_del", Del);

  exports->Set(Nan::New("DbCursor").ToLocalChecked(), tpl->GetFunction());
}

DBC* DbCursor::get_dbc() {
  return _dbc;
}

int DbCursor::create_cursor(DB* db, DB_TXN* tx, u_int32_t flags) {
  return db->cursor(db, tx, &_dbc, flags);
}

int DbCursor::count(db_recno_t *countp, u_int32_t flags) {
  // fprintf(stderr, "DbCursor::count - countp = %p\n", countp);
  return _dbc->count(_dbc, countp, flags);
}

int DbCursor::close() {
  int ret = 0;
  if (_dbc) {
    //fprintf(stderr, "%p: close %p\n", this, _dbc);
    ret = _dbc->close(_dbc);
    _dbc = NULL;
  }
  return ret;
}

int DbCursor::put(DBT *key, DBT *data, u_int32_t flags) {
  // fprintf(stderr, "DbCursor::put - txn = %p\n", txn);
  return _dbc->put(_dbc, key, data, flags);
}

int DbCursor::get(DBT *key, DBT *data, u_int32_t flags) {
  return _dbc->get(_dbc, key, data, flags);
}

int DbCursor::del(u_int32_t flags) {
  return _dbc->del(_dbc, flags);
}

void DbCursor::New(const Nan::FunctionCallbackInfo<Value>& args) {
  DbCursor* cursor = new DbCursor();
  cursor->Wrap(args.This());

  DB* _db = NULL;
  DB_TXN* _dbtxn = NULL;
  if (args.Length() > 0) {
    if (! args[0]->IsObject()) {
      return Nan::ThrowTypeError("First argument must be a Db object");
    }
    Db* db = Nan::ObjectWrap::Unwrap<Db>(args[0]->ToObject());
    _db = db->get_db();
  }
  if (args.Length() > 1) {
    if (! args[1]->IsObject()) {
      return Nan::ThrowTypeError("Second argument must be a DbTxn object");
    }
    DbTxn* dbtxn = Nan::ObjectWrap::Unwrap<DbTxn>(args[1]->ToObject());
    _dbtxn = dbtxn->get_txn();
  }
  
  int ret = cursor->create_cursor(_db, _dbtxn, 0);

  if (ret) {
    return Nan::ThrowTypeError("Could not create DBC object");
  }

  args.GetReturnValue().Set(args.This());
}

void DbCursor::Count(const Nan::FunctionCallbackInfo<Value>& args) {
  DbCursor* cursor = Nan::ObjectWrap::Unwrap<DbCursor>(args.This());
  db_recno_t *countp = new db_recno_t(0);
  cursor->count(countp, 0);
  // fprintf(stderr, "DbCursor::Count countp = %u\n", *countp);
  args.GetReturnValue().Set(Nan::New(u_int32_t(*countp)));
}

void DbCursor::Close(const Nan::FunctionCallbackInfo<Value>& args) {
  DbCursor* cursor = Nan::ObjectWrap::Unwrap<DbCursor>(args.This());
  int ret = cursor->close();
  args.GetReturnValue().Set(Nan::New(double(ret)));
}

void DbCursor::Put(const Nan::FunctionCallbackInfo<Value>& args) {
  // fprintf(stderr, "DbCursor::Put (start) - args[2] = %p\n", *args[2]);

  if (!(args.Length() > 0) && ! args[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a Buffer (value)");
    return;
  }

  DbCursor* cursor = Nan::ObjectWrap::Unwrap<DbCursor>(args.This());

  Local<Object> buf = args[0]->ToObject();
  
  DBT * data_dbt = new DBT();
  dbt_set(data_dbt,
          node::Buffer::Data(buf),
          node::Buffer::Length(buf));

  int ret = cursor->put(NULL, data_dbt, DB_CURRENT);

  args.GetReturnValue().Set(Nan::New(double(ret)));
}

void DbCursor::Current(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  _Get(args, DB_CURRENT);
}

void DbCursor::Set(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  _Get(args, DB_SET);
}

void DbCursor::Next(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  _Get(args, DB_NEXT);
}

void DbCursor::Prev(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  _Get(args, DB_PREV);
}

void DbCursor::First(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  _Get(args, DB_FIRST);
}

void DbCursor::Last(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  _Get(args, DB_LAST);
}

void DbCursor::_Get(const Nan::FunctionCallbackInfo<Value>& args, u_int32_t flags) {
  DbCursor* cursor = Nan::ObjectWrap::Unwrap<DbCursor>(args.This());

  DBT key_dbt;
  DBT retbuf;
  dbt_set(&retbuf, 0, 0, DB_DBT_MALLOC);

  if (args.Length() > 0) {
    if (! args[0]->IsString()) {
      Nan::ThrowTypeError("First argument must be a String (key)");
      return;
    }
    String::Utf8Value key(args[0]);
    dbt_set(&key_dbt, *key, strlen(*key), DB_DBT_MALLOC);
    flags = DB_SET;
  } else {
    dbt_set(&key_dbt, 0, 0, DB_DBT_MALLOC);
  }

  cursor->get(&key_dbt, &retbuf, flags);

  Local<Object> buf = Nan::NewBuffer((char*)retbuf.data, retbuf.size, free_buf, NULL).ToLocalChecked();
  Local<Object> res = Nan::New<Object>();
  if (key_dbt.data && flags != DB_SET) {
    Local<Object> keyObj = Nan::NewBuffer((char*)key_dbt.data, key_dbt.size, free_buf, NULL).ToLocalChecked();
    res->Set(Nan::New<String>("key").ToLocalChecked(), keyObj);
  } else {
    res->Set(Nan::New<String>("key").ToLocalChecked(), Nan::Null());
  }
  res->Set(Nan::New<String>("value").ToLocalChecked(), buf);
  args.GetReturnValue().Set(res);
}

void DbCursor::Del(const Nan::FunctionCallbackInfo<Value>& args) {
  DbCursor* cursor = Nan::ObjectWrap::Unwrap<DbCursor>(args.This());
  int ret = cursor->del(0);
  args.GetReturnValue().Set(Nan::New(double(ret)));
}
