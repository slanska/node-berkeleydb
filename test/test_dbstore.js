var Db = require("..").Db;
var DbEnv = require("..").DbEnv;

var dbenv = new DbEnv();
console.log("open env", dbenv.open("db"));
var db = new Db(dbenv);

var assert = require('assert');

var rand = (Math.random() * 1e6).toFixed(0);
var filename = `foo-${rand}.db`;

var openRes = db.open(filename);
console.log("opened", filename, "ret=", openRes);
  
function test_put_get() {
  console.log("-- test_put_get");

  for (var i = 0; i < 5000; ++i) {
    var key = i.toFixed(0);
    var val = (Math.random() * 1e6).toFixed(0);

    db.put(key, val)
    console.log("put", key);
    var str = db.get(key)
    console.log("get", key, "=>", str.toString());
    assert(str.toString() === val);
    console.log("del", key);
    db.del(key);
    var str2 = db.get(key);
    console.log("get", key, "=>", str2.toString());
    assert(str2.toString() === "");
  }
}

function test_json() {
  console.log("-- test_json");
  var opts = { json: true };
  var put_data = { test: "json1", n: 1 };
  db.put("json1", put_data, opts)
  console.log("put json1", put_data);
  var data = db.get("json1", opts);
  console.log("get json1", data);
  assert(typeof data == 'object');
  assert(data.test == put_data.test);
  assert(data.n == put_data.n);
  console.log("del json1");
  db.del("json1");
}

function test_encoding() {
  console.log("-- test_encoding");
  var opts = { encoding: "hex" };
  var enc_str = "4f4ca1";
  db.put("hex", enc_str, opts)
  console.log("put hex", enc_str);
  var out = db.get("hex", opts);
  console.log("get hex", out);
  assert(enc_str == out);
  console.log("del hex");
  db.del("hex");
}

test_put_get();
test_json();
test_encoding();

var closeRes = db.close();
console.log("closed", "ret=", closeRes);
