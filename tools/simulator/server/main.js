var args = process.argv.slice(2);

var streamsrv = require('./streamer-server');
var express = require('express');
var favicon = require('serve-favicon');
var path = require('path');
var app = express();
app.use(favicon(path.join(__dirname, 'content', 'favicon.ico')))

var isClipv2 = args.length >= 1 && args[0] == '-clipv2';

if (!isClipv2) {
	var fs = require('fs');
	var serialized = fs.readFileSync('server/config/config.json', 'utf8');
	var config = JSON.parse(serialized);

	isClipv2 = config.swversion >= 1941088000;
}

var fs = require('fs');

const options = {
	key: fs.readFileSync('./cert/key.pem'),
	cert: fs.readFileSync('./cert/cert.pem')
}

if (isClipv2) {
	var spdy = require('spdy');

	var http = spdy.createServer(options, app);
	http.timeout = 0; // Need that for the sse connection, otherwise it will timeout after 120 secs of inactivity.
	var io = require('socket.io')(http);

	var simulatorServer = require('./simulator-server-clipv2');
	var server = new simulatorServer(app, express, streamsrv, io);

	process.env.PORT = 443;
	process.env.IP = '0.0.0.0';

	http.listen(process.env.PORT, process.env.IP, (error) => {
		if (error) {
			console.error(error)
			return process.exit(1)
		}
		else {
			console.log('webserver listening on *:' + process.env.PORT);
		}
	});
}
else {
	var http = require('https').Server(options, app);

	var io = require('socket.io')(http);

	var simulatorServer = require('./simulator-server');
	var server = new simulatorServer(app, express, streamsrv, io);

	process.env.PORT = 443;
	process.env.IP = '0.0.0.0';

	http.listen(process.env.PORT, process.env.IP, function () {
		console.log('webserver listening on *:' + process.env.PORT);
	});
}

