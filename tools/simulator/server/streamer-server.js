var events = require('events');
var dgram = require('dgram');

function deserialize(data, cb) {
    // Deseralize the data - magic numbers are as per the Hue Stream protocol
    var length = data.length - 16;
    var colormode = data[14];
    var filter = data[15];
    var version = data[9];

    var rawData = [];
    for (var i = 0; i < length; i++) {
        rawData.push(data[i + 16]);
    }

    // Build the associative array to return
    var retData = {colormode: colormode, filter: filter, length: length, version: version, data: rawData};

    // And call the callback passing the deseralized data
    cb(retData);
}

function udpServer(port, cb) {
    // Set up the socket
    var sock = dgram.createSocket("udp4", function (msg) {
        var data = [];
        for (var i = 0; i < msg.length; i++) {
            data.push(msg[i]);
        }
        deserialize(data, cb);
    });
    sock.bind(port);
    console.log('Stream UDP Server bound port %d', port);
}

// Stream server class
exports.listen = function (port, cb) {
    udpServer(port, cb);
};
