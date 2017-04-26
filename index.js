var addon = require("bindings")("addon.node");

var Db = addon.Db;

Db.prototype.put = function (key, val, opts={}) {
  if (opts.json) {
    val = JSON.stringify(val);
  }

  var buf = val;
  if (typeof buf == 'string') {
    buf = new Buffer(val, opts.encoding || 'utf8');
  }

  var args = opts.txn ? [key, buf, opts.txn] : [key, buf];
  return this._put.apply(this, args);
};

Db.prototype.del = function (key, opts={}) {
  var args = opts.txn ? [key, opts.txn] : [key];
  return this._del.apply(this, args);
};

Db.prototype.get = function (key, opts={}) {
  if (typeof opts == 'string') {
    opts = { encoding: opts };
  }

  var args = opts.txn ? [key, opts.txn] : [key];
  var buf = this._get.apply(this, args);

  if (opts.encoding || opts.json) {
    buf = buf.toString(opts.encoding || 'utf8');
  }
  if (opts.json) {
    buf = JSON.parse(buf);
  }
  return buf;
};

module.exports.Db = addon.Db;
module.exports.DbEnv = addon.DbEnv;
module.exports.DbTxn = addon.DbTxn;
