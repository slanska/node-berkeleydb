#ifndef DBENV_H
#define DBENV_H

#include <nan.h>

#include <db.h>

class DbEnv : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  DB_ENV* get_env();

  int create(u_int32_t flags);
  int open(char const *db_home, u_int32_t flags, int mode);
  int close(u_int32_t flags);

 private:
  DbEnv();
  ~DbEnv();

  DB_ENV *_dbenv;

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Open(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void Close(const Nan::FunctionCallbackInfo<v8::Value>& args);
};

#endif
