function SimulatorServerClipV2(app, express, streamsrv, io) {
    var that = this;
    that.resources;
    that.streamData = [];

    var fs = require('fs');
    var bodyParser = require('body-parser');
    var resourcesManager = require('./resources-manager-clipv2.js');
    var eventEmitter = require('events');
    var stream = new eventEmitter();
    var messageReceivedTimeout;
    var sseClients = [];

    const USER = "aSimulatedUser";
    const CLIENT_KEY = "01234567890123456789012345678901";
    const APPLICATION_ID = "abcdef01-0123-0123-0123-abcdef012345";
    const CLOSE_SESSION_TIME_MS = 10000;

    io.on('connection', function (socket) {
        console.log('a user connected to the websocket');
    });

    streamsrv.listen(2100, function (msg) {
        io.emit('streamData', msg);
        that.streamMessage(msg);
        that.streamData = msg.data;
    });

    function streamTimeout() {
        clearTimeout(messageReceivedTimeout);
        var result = that.resources.isEntertainmentConfigurationActive(USER);
        if (result.active) {
            console.log("Closing streaming session due to inactivity for " + CLOSE_SESSION_TIME_MS / 1000 + " sec");
            that.resources.setEntertainmentConfigurationActive(result.ecId, false, "");
        }
    }

    this.streamMessage = function (msg) {
        var result = that.resources.isEntertainmentConfigurationActive(USER);
        if (result.active) {
            clearTimeout(messageReceivedTimeout);
            messageReceivedTimeout = setInterval(streamTimeout, CLOSE_SESSION_TIME_MS);
        }
    }

    function readServerPackageInfo() {
        return JSON.parse(fs.readFileSync('package.json', 'utf8'));
    }

    function displayPackageInfo(serverInfo) {
        var serverPackageInfo = readServerPackageInfo();

        console.log("\n-----------------------------------------\n");
        console.log(serverInfo.name + " " + serverInfo.version);
        console.log("\n-----------------------------------------\n");
    }

    function validateAPIVersion(config) {
        const found = config.apiversion.match("(\\d+)\\.(\\d+)\\.(\\d+)");
        if (found.length == 4) {
            if (parseInt(found[1]) < 1) {
                config.apiversion = "1.24.0";
            }
            else if (parseInt(found[1]) == 1 && parseInt(found[2]) < 24) {
                config.apiversion = "1.24.0";
            }
        }
    }

    function init() {
        displayPackageInfo(readServerPackageInfo());
        that.resources = new resourcesManager();

        app.use(bodyParser.json());
        app.use("/", express.static('.'));

        app.get('/api/*/config', function (req, res) {
            //console.log('simulator: config');
            res.setHeader('Content-Type', 'application/json');
            var serialized = fs.readFileSync('server/config/config.json', 'utf8');
            var config = JSON.parse(serialized);
            // Force LOCALHOST here so that it matches with the certificate cn field.
            config.bridgeid = "LOCALHOST";
            // Also make sure api is at least 1.24.0, otherwise client might think https is not supported.
            validateAPIVersion(config);
            if (parseInt(config.swversion) < 1944193080) {
                config.swversion = "1944193080";
            }

            res.send(JSON.stringify(config));
        });

        app.get('/api/config', function (req, res) {
            console.log('simulator: small config');
            res.setHeader('Content-Type', 'application/json');
            res.header("Access-Control-Allow-Origin", "*");
            //res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
            var serialized = fs.readFileSync('server/config/config.json', 'utf8');
            var config = JSON.parse(serialized);
            // Force LOCALHOST here so that it matches with the certificate cn field.
            config.bridgeid = "LOCALHOST";
            // Also make sure api is at least 1.24.0, otherwise client might think https is not supported.
            validateAPIVersion(config);

            var smallconfig = {
                name: config['name'],
                bridgeid: config['bridgeid'],
                modelid: config['modelid'],
                apiversion: config['apiversion'],
                swversion: config['swversion']
            }

            if (parseInt(smallconfig.swversion) < 1944193080) {
                smallconfig.swversion = "1944193080";
            }

            res.send(JSON.stringify(smallconfig));
        });

        app.post('/api', function (req, res) {
            console.log('POST / (create whitelist)');
            res.setHeader('Content-Type', 'application/json');
            res.send("[{\"success\":{\"username\": \"" + USER + "\", \"clientkey\": \"" + CLIENT_KEY + "\"}}]");
        });

        // ClipV2 stuff from here
        app.get('/auth/v1', function (req, res) {
            console.log('GET / (auth v1) ' + req.socket.remotePort);
            res.setHeader("hue-application-id", APPLICATION_ID);
            res.send("");
        });

        app.get('/clip/v2/resource/device', function (req, res) {
            console.log('GET / (device list) ' + req.socket.remotePort);
            res.setHeader('Content-Type', 'application/json');
            res.send(JSON.stringify(that.resources.getDeviceConfig()));
        });

        app.get('/clip/v2/resource/zigbee_connectivity', function (req, res) {
            console.log('GET / (zigbee connectivity list) ' + req.socket.remotePort);
            res.setHeader('Content-Type', 'application/json');
            res.send(JSON.stringify(that.resources.getZigbeeConnectivityConfig()));
        });

        app.get('/clip/v2/resource/bridge', function (req, res) {
            console.log('GET / (bridge list) ' + req.socket.remotePort);
            res.setHeader('Content-Type', 'application/json');
            res.send(JSON.stringify(that.resources.getBridgeConfig()));
        });

        app.get('/clip/v2/resource/entertainment', function (req, res) {
            console.log('GET / (entertainment list) ' + req.socket.remotePort);
            res.setHeader('Content-Type', 'application/json');
            res.send(JSON.stringify(that.resources.getEntertainmentConfig()));
        });

        app.get('/clip/v2/resource/light', function (req, res) {
            console.log('GET / (light list) ' + req.socket.remotePort);
            res.setHeader('Content-Type', 'application/json');
            res.send(JSON.stringify(that.resources.getLightConfig()));
        });

        app.get('/clip/v2/resource/zone', function (req, res) {
            console.log('GET / (zone list) ' + req.socket.remotePort);
            res.setHeader('Content-Type', 'application/json');
            res.send(JSON.stringify(that.resources.getZoneConfig()));
        });

        app.get('/clip/v2/resource/entertainment_configuration', function (req, res) {
            console.log('GET / (entertainment configuration list) ' + req.socket.remotePort);
            res.setHeader('Content-Type', 'application/json');
            res.setHeader('Access-Control-Allow-Origin', '*');
            res.send(JSON.stringify(that.resources.getEntertainmentConfigurationConfig()));
        });

        app.get('/clip/v2/resource/scene', function (req, res) {
            console.log('GET / (scene list) ' + req.socket.remotePort);
            res.setHeader('Content-Type', 'application/json');
            res.send(JSON.stringify(that.resources.getSceneConfig()));
        });

        stream.on("push", function (data) {
            for (var i = 0; i < sseClients.length; i++) {
                sseClients[i].res.write(data + "\n\n");
            }
        });

        app.get('/eventstream/clip/v2', function (req, res) {
            console.log('eventstream ' + req.socket.remotePort);
            res.writeHead(200, {
                'Content-Type': 'text/event-stream',
                'Cache-Control': 'no-cache',
                'Connection': 'keep-alive'/*,
                'Access-Control-Allow-Origin': '*'*/
            });

            const clientId = Date.now();
            const newClient = {
                id: clientId,
                res: res
            };

            console.log("New SSE client: " + clientId);

            sseClients.push(newClient);

            // If we ever have trouble with the http timeout option being 0, we could always do the following to keep the default timeout from happening.
            /*
             * iid = setInterval(function() {
                   socket.handshake.session.reload(function() {
                        hs.session.touch().save()
                   })
               }, interval)
             */

            req.on('close', function () {
                console.log("SSE client connection closed: " + clientId);
                // Remove that client from our list
                for (var i = 0; i < sseClients.length; i++) {
                    if (sseClients[i].id === clientId) {
                        sseClients.splice(i, 1);
                        break;
                    }
                }
            });

            res.write(": hi\n\n");
        });

        app.put('/clip/v2/resource/entertainment_configuration/:ecId', function (req, res) {
            if (req.body.action === undefined) {
                console.log("PUT entertainment_configuration -- action: undefined")
                res.sendStatus(500);
                return;
           }

            var ecId = req.params.ecId;

            if (ecId === undefined) {
                console.log("PUT entertainment_configuration -- ecId: undefined")
                res.sendStatus(500);
                return;
            }

            var action = req.body.action;

            if (action !== "start" && action !== "stop") {
                console.log("PUT entertainment_configuration -- action: unknown value " + action);
                res.sendStatus(400);
                return;
            }

            if (that.resources.getEntertainmentConfiguration(ecId) === undefined) {
                console.log("PUT entertainment_configuration -- ecId: " + ecId + " does not exist");
                res.sendStatus(404);
                return;
            }

            console.log('PUT entertainment_configuration -- ecId: ' + ecId + ' action: ' + action);

            res.setHeader('Content-Type', 'application/json');

            var active = action === "start";

            var result = that.resources.setEntertainmentConfigurationActive(ecId, active, active ? APPLICATION_ID : "");

            if (result.success) {
                if (active) {
                    messageReceivedTimeout = setInterval(streamTimeout, CLOSE_SESSION_TIME_MS);
                }
                else {
                    clearTimeout(messageReceivedTimeout);
                }

                res.send("{\"data\": [{ \"rid\": \"" + ecId + "\", \"rtype\": \"entertainment_configuration\" }], \"errors\": [] }");
            }
            else {
                if (active) {
                    res.send("{\"errors\": [{\"description\": \"cannot override stream ownership\"}]}");
                }
                else {
                    res.send("{\"errors\": [{\"description\": \"unknown error\"}]}");
                }
            }

            if (result.update !== undefined) {
                stream.emit("push", result.update);
            }
        });

        app.put('/develop/groupconfig/:ecId/state', function (req, res) {
            var ecId = req.params.ecId;
            if (ecId === undefined) {
                console.log("PUT ('/develop/groupconfig/:ecId/state' ERROR ecId: undefined")
                res.sendStatus(500);
                return;
            }

            var active = req.body.active;
            var owner = req.body.owner;

            if (active === undefined && owner === undefined) {
                console.log("PUT ('/develop/groupconfig/" + ecId + "/state' ERROR active: undefined, owner: undefined")
                res.sendStatus(500);
                return;
            }

            console.log("PUT ('/develop/groupconfig/" + ecId + "/state' active: " + active + ", owner: " + owner)
            if (active !== undefined) {
                var result = that.resources.setEntertainmentConfigurationActive(ecId, active, active ? owner : "");

                if (result.success) {
                    if (active && owner === USER) {
                        messageReceivedTimeout = setInterval(streamTimeout, CLOSE_SESSION_TIME_MS);
                    }
                    else {
                        clearTimeout(messageReceivedTimeout);
                    }

                    res.send("{\"data\": [{ \"rid\": \"" + ecId + "\", \"rtype\": \"entertainment_configuration\" }], \"errors\": [] }");
                }
                else {
                    if (active) {
                        res.send("{\"errors\": [{\"description\": \"cannot override stream ownership\"}]}");
                    }
                    else {
                        res.send("{\"errors\": [{\"description\": \"unknown error\"}]}");
                    }
                }

                if (result.update !== undefined) {
                    stream.emit("push", result.update);
                }
            }
        });

        app.put('/develop/groupconfig/:ecId/light', function (req, res) {
            var ecId = req.params.ecId;
            if (ecId === undefined) {
                console.log("PUT light -- ecId: undefined")
                res.sendStatus(500);
                return;
            }

            var location = req.body.location;
            if (location === undefined) {
                console.log("PUT light -- location: not set")
                res.sendStatus(500);
                return;

            }

            var result = that.resources.addLight(ecId, location);

            if (result.success) {
                console.log("PUT light -- location: " + location);
                res.sendStatus(200);

                if (result.update !== undefined) {
                    stream.emit("push", result.update);
                }
            }
            else {
                console.log("PUT light -- failed");
                res.sendStatus(500);
            }
        });

        app.delete('/develop/groupconfig/:ecId/light/:channelId', function (req, res) {
            var ecId = req.params.ecId;
            if (ecId === undefined) {
                console.log("DELETE light FAIL (ecId: undefined)")
                res.sendStatus(500);
                return;
            }

            var channelId = req.params.channelId;
            if (channelId === undefined) {
                console.log("DELETE light FAIL (channelId: undefined)")
                res.sendStatus(500);
                return;
            }

            result = that.resources.deleteChannel(ecId, parseInt(channelId));

            if (result.success) {
                console.log("DELETE light SUCCESS (ecId: " + ecId + ", lightId: " + channelId + ")");
                res.sendStatus(200);

                if (result.update !== undefined) {
                    stream.emit("push", result.update);
                }
            }
            else {
                console.log("DELETE light -- failed");
                res.sendStatus(500);
            }
        });

        app.get('/develop/lights/:channelId', function (req, res) {
            console.log("light color request");
            var i = 36;
            var lightDataSize = 7;
            var channelId = req.params.channelId;

            var location_count = that.streamData.length / lightDataSize;

            for (var c = 0; c < location_count; c++) {
                // id is only 1 byte on clipv2
                var id = that.streamData[i];

                if (id == channelId) {
                    var r = Math.round((((that.streamData[i + 1] << 8) | (that.streamData[i + 2])) / 65535.0) * 255);
                    var g = Math.round((((that.streamData[i + 3] << 8) | (that.streamData[i + 4])) / 65535.0) * 255);
                    var b = Math.round((((that.streamData[i + 5] << 8) | (that.streamData[i + 6])) / 65535.0) * 255);

                    res.send(JSON.stringify({ "state": { "r": r, "g": g, "b": b } }));
                    return;
                }

                i += lightDataSize;
            }

            res.sendStatus(400);
        });
    }

    init();
}

// export the class
module.exports = SimulatorServerClipV2;