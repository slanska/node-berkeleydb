#ifndef DBCURSOR_H
#define DBCURSOR_H

#include <nan.h>

#include <db.h>

class DbCursor : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  DBC* get_dbc();

  int create_cursor(DB* db, DB_TXN* tx, u_int32_t flags);

  int close();
  int count(db_recno_t *countp, u_int32_t flags);
  int del(u_int32_t flags);
  int get(DBT *key, DBT *data, u_int32_t flags);
  int put(DBT *key, DBT *data, u_int32_t flags);
  // int cmp()
  // int dup()

 private:
  DbCursor();
  ~DbCursor();

  DBC *_dbc;

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Close(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Count(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Del(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Put(const Nan::FunctionCallbackInfo<v8::Value>& args);

  static void Current(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Set(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Next(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Prev(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void First(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Last(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void _Get(const Nan::FunctionCallbackInfo<v8::Value>& args, u_int32_t flags);
};

#endif
