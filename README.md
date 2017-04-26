# node-berkeleydb
Nodejs bindings for Berkeley DB 6.x

Based on the initial work [dbstore](https://github.com/leei/node-dbstore) by Lee Iverson.

## Installation

  `npm install berkeleydb`

## Usage

  `var bdb = require("berkeleydb");`

All calls are synchronous, since berkeleydb itself is synchronous.

### DB

Represents a Berkeley DB database object. Provides a simple `put`/`get`/`del` synchronous interface.

```node
var bdb = require("berkeleydb");

var db = new bdb.Db(); // create a new Db object

var key = "foo";
var val = "bar";

// data access
db.put(key, val); // put
var out1 = db.get(key) // get
db.del(key); // del
var out2 = db.get(key); // get deleted key

assert(out1.toString() === val);
assert(out2.toString() === "");

db.close()
```

* `new bdb.Db([dbenv])` - Creates a new Db instance.
  - param: `[dbenv]` - `[bdb.DbEnv]` - Optional, but needed if you wish to use transactions.
  - returns `[bdb.Db]` - A new Db instance.
* `open(filename)` - Opens a local db file. Will create file if it doesn't exist.
  - param: `filename` - `[String]` - The filename of the db to load, relative to the process.cwd.
  - returns `[number]` - 0 if successful, otherwise an error occurred.
* `close()` - Closes a database file. This is neccessary before shutdown to avoid data corruption.
  - returns `[number]` - 0 if successful, otherwise an error occurred.
* `get(key, [opts])` - Gets a value from the db.
  - param: `key` - `[String]` - The key of the value to get.
  - param: `[opts]` - `[Object]` - An [options](#options) object.
  - returns `[Buffer|String|Object]` - The data stored at the key - `""` if the key doesn't exist, a `String` is returned if `opts.encoding` is set, an `Object` if `opts.json` is set to `true`, otherwise as a `Buffer`.
* `put(key, val, [opts])` - Stores or updates a value at the given key.
  - param: `key` - `[String]` - The key to store/update.
  - param: `val` - `[Buffer|String|Object]` - The value to store.
  - param: `[opts]` - `[Object]` - An [options](#options) object.
  - returns `[number]` - 0 if successful, otherwise an error occurred.
* `del(key, [opts])`
  - param: `key` - `[String]` - The key to delete.
  - param: `[opts]` - `[Object]` - An [options](#options) object.
  - returns `[number]` - 0 if successful, otherwise an error occurred.

#### Options object
The following options are available for the `put`/`get`/`del` methods:

* `opts.json` - `[Boolean]` - Store or retrieve value as a json object. Uses JSON.stringify/JSON.parse on value.
* `opts.encoding` - `[String]` - If specified, the buffer will be encoded/decoded as the specified format and stored/returned as a String.
* `opts.txn` - `[bdb.DbTxn]` - Apply the operation to the given transaction. This will not perform the operation until the transaction is commited.

```node
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
```

### DB_ENV

A DbEnv enables logging, transactions and other berkeley db features, and instead of using a single file, it can store multiple files (dbs) in a directory, along with meta data.

```node
var dbenv = new bdb.DbEnv();
console.log("open env", dbenv.open("db"));

// Pass the dbenv into the db constructor
var db = new bdb.Db(dbenv);

...

// make sure to close the db first
db.close();
dbenv.close();
```

* `new bdb.DbEnv()` - Creates a new DbEnv instance.
  - returns `[bdb.DbEnv]` - A new DbEnv instance.
* `open(dirname)` - Opens a local db env directory. The folder must exist before execution, but may be empty uninitialized.
  - param: `dirname` - `[String]` - The path of the db directory to use, relative to the process.cwd.
  - returns `[number]` - 0 if successful, otherwise an error occurred.
* `close()` - Closes a database dir. This is neccessary before shutdown to avoid data corruption, but after `Db.close()`.
  - returns `[number]` - 0 if successful, otherwise an error occurred.

### DB_TXN

Transactions provides ACID-ity to the db operations. See [Berkeley Db Transaction Documentation](http://docs.oracle.com/cd/E17076_05/html/gsg_txn/C/index.html) for full explanation.

```node
  // pass txn into options
  var txn = new bdb.DbTxn(dbenv);
  var txn2 = new bdb.DbTxn(dbenv);
  var opts = { "txn": txn };
  var opts2 = { "txn": txn2 };


  // commit
  db.put("1", "one", opts);
  db.put("2", "two", opts);
  db.put("3", "three", opts);
  txn.commit();
  var out1 = db.get("1");
  var out2 = db.get("2");
  var out3 = db.get("3");

  assert("one" == out1);
  assert("two" == out2);
  assert("three" == out3);


  // abort
  db.put("4", "four", opts2);
  db.put("5", "five", opts2);
  db.put("6", "six", opts2);
  txn2.abort();
  var out4 = db.get("4");
  var out5 = db.get("5");
  var out6 = db.get("6");

  assert("" == out4);
  assert("" == out5);
  assert("" == out6);
```

* `commit()` - Commits the operations associated to the transaction, with appropriate locking.
  - returns `[number]` - 0 if successful, otherwise an error occurred.

* `abort()` - Aborts the transaction.
  - returns `[number]` - 0 if successful, otherwise an error occurred.


## Tests

  `npm test`


## Todo
* Implement DBcursor
* Implement Bulk search operations
* Implement DB_SEQUENCE
* Implement DB_LOGC

### Licence

node-berkeleydb is licensed under an MIT license. All rights not explicitly granted in the MIT license are reserved. See the included LICENSE file for more details.
