#ifndef DBENV_H
#define DBENV_H

#include <node.h>
#include <node_object_wrap.h>

class DbEnv : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> target);

 private:
  DbEnv();
  ~DbEnv();

  static v8::Handle<v8::Value> New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Handle<v8::Value> PlusOne(const v8::FunctionCallbackInfo<v8::Value>& args);
  double counter_;
};

#endif
