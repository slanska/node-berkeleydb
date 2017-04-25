#ifndef DBSTORE_H
#define DBSTORE_H

#include <nan.h>

#include <db.h>

class DbStore : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  DB* get_db();

  int create(DB_ENV *dbenv, u_int32_t flags);
  int open(char const *fname, char const *db, DBTYPE type, u_int32_t flags, int mode);
  int close(u_int32_t flags);

  int get(DBT *key, DBT *data, u_int32_t flags);
  int put(DBT *key, DBT *data, u_int32_t flags);
  int del(DBT *key, u_int32_t flags);


 private:
  DbStore();
  ~DbStore();

  DB *_db;

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Open(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Close(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Get(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Put(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Del(const Nan::FunctionCallbackInfo<v8::Value>& args);
};

#endif
