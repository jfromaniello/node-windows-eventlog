
# Windows EventLog Js

  Native node.js module to log messages to the Windows Event Log.

## Installation

    $ npm install windows-eventlog

## Usage

Initialize somewhere the logger like:

```js
  var eventlog = require('windows-eventlog');
  eventlog.init("MyAppName");
```

Then you can do:

```js
  var eventlog = require('windows-eventlog');
  eventlog.log("This is an information messsage");
  eventlog.log("This is another info message", "Information");
  eventlog.log("This is an error message", "Error");
  eventlog.log("This is a warning", "Warning");
```

And you will see this:

![2012-04-09_1007.png](http://joseoncodecom.ipage.com/wp-content/uploads/images/2012-04-09_1007.png)

## How it works

This module was built on c++/cli (.Net) and uses [System.Diagnostics.EventLog](http://msdn.microsoft.com/en-us/library/system.diagnostics.eventlog.aspx). 

In order to log events you need to run the application with an elevated account: ie administrator or system account. Windows services run with the system account so this project works  well with [WinSer](https://github.com/jfromaniello/winser).


## TODO

- Use uv_queue_work to execute the writelog method in a different thread ( ? )

## About win32 native modules

If you are looking on how to create native modules in windows follow [this great tutorial](https://github.com/saary/node.net/).

## License 

(The MIT License)

Copyright (c) 2012 Jose Romaniello &lt;jfromaniello@gmail.com&gt;

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
'Software'), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.