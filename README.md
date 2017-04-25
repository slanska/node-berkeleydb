# node-berkeleydb
Nodejs bindings for Berkeley DB 6.x
Based on the initial work [dbstore](https://github.com/leei/node-dbstore) by Lee Iverson.

# Installation 

  `npm install berkeleydb`

# Usage

All calls are synchronous, since berkeleydb itself is synchronous.
Currently the implemented methods are:
  
  * DB
    - open
    - close
    - get
    - put
    - del
  * DBENV
    - open
    - close

```node
var Db = require("berkeleydb").Db;
var DbEnv = require("berkeleydb").DbEnv;

var dbenv = new DbEnv(); // create a new db env (optional, but neccessary for transactions)
dbenv.open("db"); // open the './db/' folder

var db = new Db(dbenv); // create a new Db object

var key = "foo";
var val = "bar";

// data access
db.put(key, val); // put
var out1 = db.get(key) // get
db.del(key); // del
var out2 = db.get(key); // get deleted key

assert(out1.toString() === val);
assert(out2.toString() === "");


// get/put support 'json' as an option
var opts = { json: true };
var put_data = { test: "json1", n: 1 };
db.put("json", put_data, opts)
var data = db.get("json", opts);

assert(typeof data == 'object');
assert(data.test == put_data.test);
assert(data.n == put_data.n);


// get/put support 'json' as an option
var opts = { encoding: 'hex' };
var enc_str = "4f4ca1";
db.put("hex", enc_str, opts)
var out = db.get("hex", opts);

assert(enc_str == out);

db.close();
dbenv.close();

```

# Todo

* Full Documentation
* Implement DB_TXN
* Implement DBcursor
* Implement Bulk search operations
* Implement DB_SEQUENCE
* Implement DB_LOGC

