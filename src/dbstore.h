#ifndef DBSTORE_H
#define DBSTORE_H

#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>

#include <db.h>

class DbStore : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> target);

  int open(char const *fname, char const *db, DBTYPE type, u_int32_t flags, int mode);
  int close();

  int put(DBT *key, DBT *data, u_int32_t flags);
  int get(DBT *key, DBT *data, u_int32_t flags);
  int del(DBT *key, u_int32_t flags);

  int sync(u_int32_t flags);

 private:
  DbStore();
  ~DbStore();

  static v8::Persistent<v8::Function> constructor;

  DB *_db;

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Open(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Get(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Put(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Del(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Sync(const v8::FunctionCallbackInfo<v8::Value>& args);
};

#endif
