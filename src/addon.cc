//#define BUILDING_NODE_EXTENSION
#include <node.h>

#include "dbstore.h"

using namespace v8;

void Init(Local<Object> exports) {
  DbStore::Init(exports);
}

NODE_MODULE(addon, Init)
