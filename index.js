var addon = require("bindings")("addon.node");

var DbStore = addon.DbStore;

DbStore.prototype.put = function (key, val, opts={}) {
  if (opts.json) {
    val = JSON.stringify(val);
  }

  var buf = val;
  if (typeof buf == 'string') {
    buf = new Buffer(val, 'utf8');
  }

  return this._put(key, buf);
};

DbStore.prototype.del = function (key) {
  return this._del(key);
};

DbStore.prototype.get = function (key, opts={}) {
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

module.exports.DbStore = addon.DbStore;
module.exports.DbEnv = addon.DbEnv;
