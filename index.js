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

  return this._put(key, buf);
};

Db.prototype.del = function (key) {
  return this._del(key);
};

Db.prototype.get = function (key, opts={}) {
  if (typeof opts == 'string') {
    opts = { encoding: opts };
  }

  var buf = this._get(key);

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
