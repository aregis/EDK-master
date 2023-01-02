function SimulatorServer(app, express, streamsrv, io) {
    const USER = "aSimulatedUser"
    const CLIENT_KEY = "01234567890123456789012345678901"
    const CLOSE_SESSION_TIME_MS = 10000;

    var that = this;
    var fs = require('fs');
    var bodyParser = require('body-parser');
    var groupsManager = require('./groups-manager.js');
    var messageReceivedTimeout;
    that.groups;
    that.streamData = [];

    io.on('connection', function(socket){
        console.log('a user connected to the websocket');
    });

    streamsrv.listen(2100, function(msg) {
        io.emit('streamData', msg);
        that.streamMessage(msg);
        that.streamData = msg.data;
    });

    function readServerPackageInfo() {
        return JSON.parse(fs.readFileSync('package.json', 'utf8'));
    }

    function displayPackageInfo(serverInfo) {
        var serverPackageInfo = readServerPackageInfo();

        console.log("\n-----------------------------------------\n");
        console.log(serverInfo.name + " " + serverInfo.version);
        console.log("\n-----------------------------------------\n");
    }

    function streamTimeout() {
        clearTimeout(messageReceivedTimeout);
        if (that.groups.isActive(USER)) {
            console.log("Closing streaming session due to inactivity for " + CLOSE_SESSION_TIME_MS / 1000 + " sec");
            that.groups.clearActive(USER);
        }
    }

    this.streamMessage = function(msg) {
        if (that.groups.isActive(USER)) {
            clearTimeout(messageReceivedTimeout);
            messageReceivedTimeout = setInterval(streamTimeout, CLOSE_SESSION_TIME_MS);
        }
    }

    function init() {
        displayPackageInfo(readServerPackageInfo());
        that.groups = new groupsManager();

        app.use(bodyParser.json());
        app.use("/", express.static('.'));

        app.post('/api', function (req, res) {
            console.log('POST / (create whitelist)');
            res.setHeader('Content-Type', 'application/json');
            res.send("[{\"success\":{\"username\": \"" + USER + "\", \"clientkey\": \"" + CLIENT_KEY + "\"}}]");
        });

        app.get('/api/*/groups', function (req, res) {
            //console.log('GET groups');
            res.setHeader('Content-Type', 'application/json');
            res.send(JSON.stringify(that.groups.getConfig()));
        });

        app.get('/api/*/groups/:groupId', function (req, res) {
            //console.log('GET group');
            var groupId = req.params.groupId;
            res.setHeader('Content-Type', 'application/json');
            res.send(JSON.stringify(that.groups.getConfigSingle(groupId)));
        });

        app.get('/api/*/lights', function (req, res) {
            //console.log('GET lights');
            res.setHeader('Content-Type', 'application/json');
            res.send(JSON.stringify(that.groups.getLights()));
        });

        app.put('/api/*/groups/:groupId', function (req, res) {
            if (req.body.stream === undefined) {
                console.log("PUT groups -- stream: undefined")
                res.sendStatus(500);
                return;
            }
            if (req.body.stream.active === undefined) {
                console.log("PUT groups -- stream.active: undefined")
                res.sendStatus(500);
                return;
            }
            var active = req.body.stream.active;

            var groupId = req.params.groupId;
            if (groupId === undefined) {
                console.log("PUT light -- groupId: undefined")
                res.sendStatus(500);
                return;
            }

            console.log('PUT groups -- groupId: ' + groupId + ' stream.active: ' + active);
            res.setHeader('Content-Type', 'application/json');
            
            if (active) {
                if (!that.groups.getActive(groupId) || that.groups.getOwner(groupId) === USER) {
                    res.send("[{\"success\":{\"/groups/" + groupId + "/stream/active\":" + active + "}}]");
                    that.groups.setActive(groupId, active);
                    that.groups.setOwner(groupId, USER);
                    messageReceivedTimeout = setInterval(streamTimeout, CLOSE_SESSION_TIME_MS);
                } else {
                    res.send("[{\"error\":{\"type\":307,\"address\":\"/groups/" + groupId + "/stream/active\",\"description\":\"Cannot claim stream ownership\"}}]");
                }
            } else {
                res.send("[{\"success\":{\"/groups/" + groupId + "/stream/active\":" + active + "}}]");
                that.groups.setActive(groupId, active);
                that.groups.setOwner(groupId, null);
                clearTimeout(messageReceivedTimeout);
            }
        });

        app.get('/api/*/config', function (req, res) {
            //console.log('simulator: config');
            res.setHeader('Content-Type', 'application/json');
            var serialized = fs.readFileSync('server/config/config.json', 'utf8');
            var config = JSON.parse(serialized);
            res.send(JSON.stringify(config));
        });
        
        app.get('/api/config', function (req, res) {
            //console.log('simulator: small config');
            res.setHeader('Content-Type', 'application/json');
            var serialized = fs.readFileSync('server/config/config.json', 'utf8');
            var config = JSON.parse(serialized);
            var smallconfig = { name: config['name'],
                                bridgeid: config['bridgeid'],
                                modelid: config['modelid'],
                                apiversion: config['apiversion'] }
            res.send(JSON.stringify(smallconfig));
        });

        app.get('/api/*', function (req, res) {
            //console.log('simulator: full config');
            res.setHeader('Content-Type', 'application/json');
            var serialized = fs.readFileSync('server/config/config.json', 'utf8');
            var config = groupConfig = JSON.parse(serialized);
            var fullConfig = {
                config :  config,
                groups :  that.groups.getConfig(),
                lights :  that.groups.getLights()
            };
            res.send(JSON.stringify(fullConfig));
        });

        app.put('/api/*/groups/*/action', function (req, res) {
            //console.log('simulator: group action');
            //console.log(req.body);
            res.setHeader('Content-Type', 'application/json');
            res.send("");
        });

        app.put('/develop/groupconfig/:groupId/light', function (req, res) {
            var groupId = req.params.groupId;
            if (groupId === undefined) {
                console.log("PUT light -- groupId: undefined")
                res.sendStatus(500);
                return;
            }

            var location = req.body.location;
            if (location === undefined) {
                console.log("PUT light -- location: not set")
                res.sendStatus(500);
                return;

            }

            if (that.groups.addLight(groupId, location[0], location[1]) === undefined) {
                console.log("PUT light -- cannot add light location");
                res.sendStatus(500);
                return;
            }
            console.log("PUT light -- location: " + location);
            res.sendStatus(200);
        });

        app.put('/develop/groupconfig/:groupId/state', function (req, res) {
            var groupId = req.params.groupId;
            if (groupId === undefined) {
                console.log("PUT ('/develop/groupconfig/:groupId/state' ERROR groupId: undefined")
                res.sendStatus(500);
                return;
            }

            var active = req.body.active;
            var owner = req.body.owner;

            if (active === undefined && owner === undefined) {
                console.log("PUT ('/develop/groupconfig/" + groupId + "/state' ERROR active: undefined, owner: undefined")
                res.sendStatus(500);
                return;
            }


            console.log("PUT ('/develop/groupconfig/" + groupId + "/state' active: " + active + ", owner: " + owner)
            if (active !== undefined) {
                that.groups.setActive(groupId, active);
                if (active && owner === USER) {
                    messageReceivedTimeout = setInterval(streamTimeout, CLOSE_SESSION_TIME_MS);
                } else {
                    clearTimeout(messageReceivedTimeout);
                }
            }
            if (owner !== undefined) {
                that.groups.setOwner(groupId, owner);
            }
            res.sendStatus(200);
        });


        app.delete('/develop/groupconfig/:groupId/light/:lightId', function (req, res) {
            var groupId = req.params.groupId;
            if (groupId === undefined) {
                console.log("DELETE light FAIL (groupId: undefined)")
                res.sendStatus(500);
                return;
            }

            var lightId = req.params.lightId;
            if (lightId === undefined) {
                console.log("DELETE light FAIL (lightId: undefined)")
                res.sendStatus(500);
                return;
            }

            console.log("DELETE light SUCCESS (groupId: " + groupId + ", lightId: " + lightId + ")");
            that.groups.deleteLight(groupId, lightId);
            res.sendStatus(200);
        });

        app.get('//entertainmentsetup', function (req, res) {
            res.redirect('/');
        });

        app.get('/develop/lights/:lightId', function (req, res) {
            console.log("light color request");
            var i = 0;
            var lightDataSize = 9;
            var lightId = req.params.lightId;

            var location_count = that.streamData.length / lightDataSize;

            for (var c = 0; c < location_count; c++) {

                var addressType = that.streamData[i];

                if (addressType === 0) {
                    var id = (that.streamData[i + 1] << 8) | (that.streamData[i + 2]);
                    if (id == lightId) {
                        var r = Math.round((((that.streamData[i + 3] << 8) | (that.streamData[i + 4])) / 65535.0) * 255);
                        var g = Math.round((((that.streamData[i + 5] << 8) | (that.streamData[i + 6])) / 65535.0) * 255);
                        var b = Math.round((((that.streamData[i + 7] << 8) | (that.streamData[i + 8])) / 65535.0) * 255);

                        //						res.send(JSON.stringify({"r":r + ", g":g }));
                        res.send(JSON.stringify({ "state": { "r": r, "g": g, "b": b } }));
                        return;
                    }
                } else {
                    console.log("warning: unsupported address type used");
                }

                i += lightDataSize;
            }
            res.sendStatus(400);
        });

    }

    init();
}

// export the class
module.exports = SimulatorServer;