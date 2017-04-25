#ifndef DBENV_H
#define DBENV_H

#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>

#include <db.h>

class DbEnv : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  int open(char const *fname, char const *db, DBTYPE type, u_int32_t flags, int mode);
  int close(u_int32_t flags);

  int put(DBT *key, DBT *data, u_int32_t flags);
  int get(DBT *key, DBT *data, u_int32_t flags);
  int del(DBT *key, u_int32_t flags);


 private:
  DbEnv();
  ~DbEnv();

  DB *_db;

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Open(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Get(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Put(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Del(const v8::FunctionCallbackInfo<v8::Value>& args);
};

#endif
