/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

function Streamer(updateLightState, notify){
    var that = this;
    var updateFrequency = 50;
    var lightDataSize = 9;
    that.streamData = [];
    that.newData = false;
    that.version = 1;
    var socket = io(); // Replace with io.connect(https://127.0.0.1) if you wanna load the index.html file directly in the browser instead of loading it from the web server.
    var timer;
    var animation = ['/', '-', '\\', '|', '/', '-', '\\', '|'];
    var animantionCount = 0;

    notify("idle")
    that.Start = function(locations) {
        var location_count = 0;

        if (Array.isArray(locations)) {
            location_count = locations.length;
            lightDataSize = 7; // clipv2
        }
        else {
            $.each(locations, function (id, pos) {
                location_count++;
            });
        }

        var bytes_needed = location_count * lightDataSize;
        console.log("start streamer for " + location_count + " locations, expected message size: " + bytes_needed + " bytes");

        console.log("start streamer for " + location_count + " locations");

        timer = setInterval(function () {
            if (!that.streamData) {
                notify("idle")
                return;
            }

            if (that.streamData.length === 0) {
                notify("idle")
                return;
            }

            if ((that.version === 1 ? bytes_needed : bytes_needed + 36) !== that.streamData.length) {
                notify("error (data size mismatch)")
                console.log("Error: not right amount of streaming data received! (needed: " + bytes_needed + ", received: " + (that.version === 1 ? that.streamData.length : that.streamData.length - 36) + ")")
                return;
            }

            if (that.newData) {
                that.newData = false;
                notify("streaming " + animation[animantionCount++ % animation.length]);
                var i = that.version === 1 ? 0 : 36;
                for (var c = 0; c < location_count; c++) {
                    // There's no address type on clipv2
                    var addressType = that.version === 1 ? that.streamData[i] : 0;
                    i = that.version === 1 ? i : i - 1;

                    if (addressType === 0) {
                        var id = 0;
                        if (that.version === 1) {
                            id = (that.streamData[i + 1] << 8) | (that.streamData[i + 2]);
                        }
                        else {
                            // id is only 1 byte on clipv2
                            id = that.streamData[i + 1];
                            i--;
                        }

                        var r = Math.round((((that.streamData[i + 3] << 8) | (that.streamData[i + 4])) / 65535.0) * 255);
                        var g = Math.round((((that.streamData[i + 5] << 8) | (that.streamData[i + 6])) / 65535.0) * 255);
                        var b = Math.round((((that.streamData[i + 7] << 8) | (that.streamData[i + 8])) / 65535.0) * 255);
                        updateLightState(id, r, g, b);
                    } else {
                        console.log("warning: unsupported address type used");
                    }

                    i += 9;
                }
            }
        }, updateFrequency);
    };

    that.Stop = function(){
        clearInterval(timer);
    };

    socket.on('streamData', function (msg) {
        if (msg.colormode === 0) {
            that.streamData = msg.data;
            that.newData = true;
            that.version = msg.version;
        }
        else {
            console.log("warning: unsupported colormode used");
        }
    });
}

