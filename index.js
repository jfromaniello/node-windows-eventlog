var fs = require('fs');
var path = require('path');

var gypRequirePathname = './build/Release/EventLog.node';
if (fs.existsSync(path.normalize(module.filename + '/../' + gypRequirePathname))) {
  module.exports = require(gypRequirePathname);
} else {
  module.exports = require('./prebuild/' + process.arch + '/EventLog.node');
}
