//#define BUILDING_NODE_EXTENSION
#include <nan.h>

#include "dbstore.h"
#include "dbenv.h"

using namespace v8;

void Init(Local<Object> exports) {
  DbStore::Init(exports);
  DbEnv::Init(exports);
}

NODE_MODULE(addon, Init)
