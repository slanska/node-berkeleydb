#ifndef DBTXN_H
#define DBTXN_H

#include <nan.h>

#include <db.h>

class DbTxn : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  // static v8::Local<v8::Object> createLocal(v8::Local<v8::Context> context, DB_TXN *dbtxn);
  
  DB_TXN* get_txn();

  int create_txn(DB_ENV* env, DB_TXN* parent);
  
  int abort();
  int commit(u_int32_t flags);
  int discard(u_int32_t flags);

  // TODO: implement these more methods
  // int get_name(const char **namep);
  // int set_name(const char *name);
  // u_int32_t id();
  // int prepare(u_int8_t gid[DB_XIDDATASIZE]);
  // u_int32_t set_timeout(db_timeout_t timeout, u_int32_t flags);

 private:
  DbTxn();
  ~DbTxn();

  static Nan::Persistent<v8::Function> constructor;
  DB_TXN *_dbtxn;

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Abort(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Commit(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Discard(const Nan::FunctionCallbackInfo<v8::Value>& args);
};

#endif
