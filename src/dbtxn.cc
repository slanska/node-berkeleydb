#include <node.h>
#include <nan.h>
#include <db.h>

#include "dbtxn.h"
#include "dbenv.h"

#include <cstdlib>
#include <cstring>
#include <uv.h>

using namespace v8;

Nan::Persistent<Function> DbTxn::constructor;

DbTxn::DbTxn() : _dbtxn(0) {};
DbTxn::~DbTxn() {
  // abort();
  // discard(0);
};

void DbTxn::Init(Local<Object> exports) {
  Nan::HandleScope scope;

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("DbTxn").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Nan::SetPrototypeMethod(tpl, "abort", Abort);
  Nan::SetPrototypeMethod(tpl, "commit", Commit);
  Nan::SetPrototypeMethod(tpl, "discard", Discard);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("DbTxn").ToLocalChecked(), tpl->GetFunction());
}

DB_TXN* DbTxn::get_txn() {
  return _dbtxn;
}

int DbTxn::create_txn(DB_ENV* env, DB_TXN* parent) {
  return env->txn_begin(env, parent, &_dbtxn, 0);
  // int ret = dbenv->txn_begin(parent, dbtxn, 0);
}

int DbTxn::abort() {
  return _dbtxn->abort(_dbtxn);
}

int DbTxn::commit(u_int32_t flags) {
  // fprintf(stderr, "DbTxn::commit - _dbtxn = %p\n", _dbtxn);
  return _dbtxn->commit(_dbtxn, flags);
}

int DbTxn::discard(u_int32_t flags) {
  return _dbtxn->discard(_dbtxn, flags);
}

void DbTxn::New(const Nan::FunctionCallbackInfo<Value>& args) {
  DB_TXN* parent = NULL;

  if (args.Length() == 0 || ! args[0]->IsObject()) {
    return Nan::ThrowTypeError("First argument must be a DbEnv object");
  }
  if (args.Length() > 1) {
    if (! args[1]->IsObject()) {
      Nan::ThrowTypeError("First argument must be a DbTxn object");
      return;
    }
    DbTxn* _parent = Nan::ObjectWrap::Unwrap<DbTxn>(args[1]->ToObject());
    parent = _parent->get_txn();
  }

  DbEnv* dbenv = Nan::ObjectWrap::Unwrap<DbEnv>(args[0]->ToObject());
  DbTxn* txn = new DbTxn();
  txn->create_txn(dbenv->get_env(), parent);
  txn->Wrap(args.This());
  // fprintf(stderr, "DbTxn::New - created dbtxn = %p, DbTxn = %p (wrapped %p)\n", dbtxn->mgrp, txn, *args.This());
  
  args.GetReturnValue().Set(args.This());
}

void DbTxn::Abort(const Nan::FunctionCallbackInfo<Value>& args) {
  DbTxn* txn = Nan::ObjectWrap::Unwrap<DbTxn>(args.This());
  int ret = txn->abort();
  args.GetReturnValue().Set(Nan::New(double(ret)));
}

void DbTxn::Commit(const Nan::FunctionCallbackInfo<Value>& args) {
  DbTxn* txn = Nan::ObjectWrap::Unwrap<DbTxn>(args.This());
  int ret = txn->commit(0);
  args.GetReturnValue().Set(Nan::New(double(ret)));
}

void DbTxn::Discard(const Nan::FunctionCallbackInfo<Value>& args) {
  DbTxn* txn = Nan::ObjectWrap::Unwrap<DbTxn>(args.This());
  int ret = txn->discard(0);
  args.GetReturnValue().Set(Nan::New(double(ret)));
}

