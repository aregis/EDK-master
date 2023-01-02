function ResourcesManager() {
    var fs = require('fs');

    var deviceConfig;
    var zigbeeConnectivityConfig;
    var bridgeConfig;
    var entertainmentConfig;
    var lightConfig;
    var zoneConfig;
    var entertainmentConfigurationConfig;
    var sceneConfg;

    var nextMessageId = 0;
    var date = new Date();
    const { v4: uuidv4 } = require('uuid');

    var that = this;

    const DEFAULT_DEVICE = './server/config/default_device.json';
    const CURRENT_DEVICE = './server/config/current_device.json';
    const DEFAULT_ZIGBEE_CONNECTIVITY = './server/config/default_zigbee_connectivity.json';
    const CURRENT_ZIGBEE_CONNECTIVITY = './server/config/current_zigbee_connectivity.json';
    const DEFAULT_LIGHT = './server/config/default_light.json';
    const CURRENT_LIGHT = './server/config/current_light.json';
    const DEFAULT_ENTERTAINMENT = './server/config/default_entertainment.json';
    const CURRENT_ENTERTAINMENT = './server/config/current_entertainment.json';
    const DEFAULT_ENTERTAINMENT_CONFIGURATION = './server/config/default_entertainment_configuration.json';
    const CURRENT_ENTERTAINMENT_CONFIGURATION = './server/config/current_entertainment_configuration.json';

	function init() {
        try {
            var path = DEFAULT_DEVICE;
            if (fs.existsSync(CURRENT_DEVICE)) {
                path = CURRENT_DEVICE;
            }

            var serialized = fs.readFileSync(path, 'utf8');
            if (serialized) {
                deviceConfig = JSON.parse(serialized);
            }
        }
        catch (err) {
            console.log(err);
        }

        try {
            var path = DEFAULT_ZIGBEE_CONNECTIVITY;
            if (fs.existsSync(CURRENT_ZIGBEE_CONNECTIVITY)) {
                path = CURRENT_ZIGBEE_CONNECTIVITY;
            }

            var serialized = fs.readFileSync(path, 'utf8');
            if (serialized) {
                zigbeeConnectivityConfig = JSON.parse(serialized);
            }
        }
        catch (err) {
            console.log(err);
        }

        try {
            var serialized = fs.readFileSync('./server/config/default_bridge.json', 'utf8');
            if (serialized) {
                bridgeConfig = JSON.parse(serialized);
            }
        }
        catch (err) {
            console.log(err);
        }

        try {
            var path = DEFAULT_ENTERTAINMENT;
            if (fs.existsSync(CURRENT_ENTERTAINMENT)) {
                path = CURRENT_ENTERTAINMENT;
            }

            var serialized = fs.readFileSync(path, 'utf8');
            if (serialized) {
                entertainmentConfig = JSON.parse(serialized);
            }
        }
        catch (err) {
            console.log(err);
        }

        try {
            var path = DEFAULT_LIGHT;
            if (fs.existsSync(CURRENT_LIGHT)) {
                path = CURRENT_LIGHT;
            }

            var serialized = fs.readFileSync(path, 'utf8');
            if (serialized) {
                lightConfig = JSON.parse(serialized);
            }
        }
        catch (err) {
            console.log(err);
        }

        try {
            var serialized = fs.readFileSync('./server/config/default_zone.json', 'utf8');
            if (serialized) {
                zoneConfig = JSON.parse(serialized);
            }
        }
        catch (err) {
            console.log(err);
        }

        try {
            var path = DEFAULT_ENTERTAINMENT_CONFIGURATION;
            if (fs.existsSync(CURRENT_ENTERTAINMENT_CONFIGURATION)) {
                path = CURRENT_ENTERTAINMENT_CONFIGURATION;
            }

            var serialized = fs.readFileSync(path, 'utf8');
            if (serialized) {
                entertainmentConfigurationConfig = JSON.parse(serialized);
            }
        }
        catch (err) {
            console.log(err);
        }

        try {
            var serialized = fs.readFileSync('./server/config/default_scene.json', 'utf8');
            if (serialized) {
                sceneConfg = JSON.parse(serialized);
            }
        }
        catch (err) {
            console.log(err);
        }
    }

    function saveConfigs() {
        fs.writeFileSync(CURRENT_DEVICE, JSON.stringify(deviceConfig, null, 4), 'utf8');
        fs.writeFileSync(CURRENT_LIGHT, JSON.stringify(lightConfig, null, 4), 'utf8');
        fs.writeFileSync(CURRENT_ENTERTAINMENT, JSON.stringify(entertainmentConfig, null, 4), 'utf8');
        fs.writeFileSync(CURRENT_ZIGBEE_CONNECTIVITY, JSON.stringify(zigbeeConnectivityConfig, null, 4), 'utf8');

        // Make sure w don't save the stauts as active, not the active_streamer
        var status = "inactive";
        var activeStreamer = undefined;
        var ec = undefined;

        for (let entertainmentConfig of entertainmentConfigurationConfig.data) {
            if (entertainmentConfig.status === "active") {
                ec = entertainmentConfig;
                status = "active";
                ec.status = "inactive";
                activeStreamer = ec.active_streamer;
                delete ec.active_streamer;
                break;
            }
        }

        fs.writeFileSync(CURRENT_ENTERTAINMENT_CONFIGURATION, JSON.stringify(entertainmentConfigurationConfig, null, 4), 'utf8');

        if (ec !== undefined) {
            ec.status = status;
            ec.active_streamer = activeStreamer;
        }
    }

    function createMessageProto() {
        var msg = "id: " + nextMessageId++;
        msg += ":0\ndata: [";

        return msg;
    }

    function createMessageDataProto(type) {
        var msg = "{\"creationtime\":\"";
        msg += date.getFullYear();
        msg += "-";
        msg += ("0" + (date.getMonth() + 1)).slice(-2);
        msg += "-";
        msg += ("0" + date.getDate()).slice(-2);
        msg += "T";
        msg += ("0" + date.getHours()).slice(-2);
        msg += ":";
        msg += ("0" + date.getMinutes()).slice(-2);
        msg += ":";
        msg += ("0" + date.getSeconds()).slice(-2);
        msg += "Z\",\"id\":\"";
        msg += uuidv4();
        msg += "\",\"type\":\"" + type + "\",\"data\":[";

        return msg;
    }

    that.getDeviceConfig = function () {
        return deviceConfig;
    };

    that.getZigbeeConnectivityConfig = function () {
        return zigbeeConnectivityConfig;
    }

    that.getBridgeConfig = function () {
        return bridgeConfig;
    }

    that.getEntertainmentConfig = function () {
        return entertainmentConfig;
    }

    that.getLightConfig = function () {
        return lightConfig;
    }

    that.getZoneConfig = function () {
        return zoneConfig;
    }

    that.getEntertainmentConfigurationConfig = function () {
        return entertainmentConfigurationConfig;
    }

    that.getSceneConfig = function () {
        return sceneConfg;
    }

    that.getEntertainmentConfiguration = function (ecId) {
        for (var i = 0; i < entertainmentConfigurationConfig.data.length; i++) {
            if (entertainmentConfigurationConfig.data[i].id === ecId) {
                return entertainmentConfigurationConfig.data[i];
            }
        }

        return undefined;
    }

    that.isEntertainmentConfigurationActive = function (owner) {
        for (var i = 0; i < entertainmentConfigurationConfig.data.length; i++) {
            if (entertainmentConfigurationConfig.data[i].active_streamer === owner) {
                return { active: true, ecId: entertainmentConfigurationConfig.data[i].id };
            }
        }

        return { active: false, ecId: undefined };
    }

    that.setEntertainmentConfigurationActive = function (ecId, active, auth) {
        var ec = this.getEntertainmentConfiguration(ecId);

        if (ec === undefined) {
            return { success: false, update: undefined };
        }

        if (active && !(ec.status === "inactive" || ec.active_streamer.rid === auth)) {
            return { success: false, update: undefined };
        }

        var msg = createMessageProto();
        var updateMessage = createMessageDataProto("update");

        ec.status = active ? "active" : "inactive";

        if (active) {
            ec.active_streamer = { rid: auth };

            updateMessage += "{\"active_streamer\":{\"rid\":\"" + auth;
            updateMessage += "\",\"rtype\":\"auth_v1\"},\"id\":\"" + ecId;
            updateMessage += "\",\"id_v1\":\"" + ec.id_v1;
            updateMessage += "\",\"type\":\"entertainment_configuration\"}]},";

            msg += updateMessage;

            updateMessage = createMessageDataProto("update");
        }
        else {
            delete ec.active_streamer;
        }

        updateMessage += "{\"id\":\"" + ecId;
        updateMessage += "\", \"id_v1\":\"" + ec.id_v1;
        updateMessage += "\", \"status\":\"" + ec.status;
        updateMessage += "\", \"type\":\"entertainment_configuration\"}]}";

        msg += updateMessage;

        // Update light status too
        for (var i = 0; i < ec.light_services.length; i++) {
            var rid = ec.light_services[i].rid;

            for (var j = 0; j < lightConfig.data.length; j++) {
                var light = lightConfig.data[i];
                if (light.id === rid) {
                    light.mode = active ? "streaming" : "normal";

                    // TODO update brightness?

                    updateMessage = createMessageDataProto("update");
                    updateMessage += "{\"id\":\"" + rid;
                    updateMessage += "\",\"id_v1\":\"" + light.id_v1;
                    updateMessage += "\",\"mode\":\"" + light.mode;
                    updateMessage += "\",\"type\":\"light\"}]}";

                    msg += ",";
                    msg += updateMessage;
                    break;
                }
            }
        }

        msg += "]";

        return { success: true, update: msg };
    }

    that.addLight = function (ecId, location) {
        var ec = this.getEntertainmentConfiguration(ecId);

        if (ec === undefined) {
            return { success: false, update: undefined };
        }

        // Make sure there's enough free channel first
        var channelIdList = [];
        channelIdList.length = 255;
        for (let channel of ec.channels) {
            channelIdList[channel.channel_id] = true;
        }

        var channelId = undefined;
        for (var i = 0; i < channelIdList.length; i++) {
            if (channelIdList[i] === undefined) {
                channelId = i;
                break;
            }
        }

        if (channelId === undefined) {
            return { success: false, update: undefined };
        }

        var light = {};
        light.id = uuidv4();
        light.id_v1 = "/lights/99";
        light.dynamics = {};
        light.metadata = { archetype: "hue_bulb", name: "Hue Bulb 99" };
        light.mode = "normal";
        light.on = { on: true };
        light.dimming = { brightness: 100.0 };
        light.color_temperature = { mirek: null };
        light.color = { gamut: { blue: { x: 0.1532, y: 0.0475 }, green: { x: 0.17, y: 0.7 }, red: { x: 0.6915, y: 0.3083 } }, gamut_type: "C", xy: { x: 0.5, y: 0.5 }};
        light.type = "light";

        lightConfig.data.push(light);

        // add corresponding entertainemnt
        var entertainment = {};
        entertainment.id = uuidv4();
        entertainment.id_v1 = light.id_v1;
        entertainment.proxy = true;
        entertainment.renderer = true;
        entertainment.segments = { configurable: false, max_segments: 1, segments: [{ length: 1, start: 0 }] };
        entertainment.type = "entertainment";

        entertainmentConfig.data.push(entertainment);

        // Add corresponding zigbee connectivity
        var zigbeeConnectivity = {};
        zigbeeConnectivity.id = uuidv4();
        zigbeeConnectivity.id_v1 = light.id_v1;
        zigbeeConnectivity.mac_address = "00:00:00:00:00:00:00:00";
        zigbeeConnectivity.status = "connected";
        zigbeeConnectivity.type = "zigbee_connectivity";

        zigbeeConnectivityConfig.data.push(zigbeeConnectivity);

        // Add corresponding device
        var device = {};
        device.id = uuidv4();
        device.id_v1 = light.id_v1;
        device.metadata = light.metadata;
        device.product_data = { certified: true, manufacturer_name: "Signify Netherlands B.V.", model_id: "LLC020", product_archetype: light.metadata.archetype, product_name: "Hue bulb", software_version: "130.1.30000" };
        device.services = [{ rid: light.id, rtype: "light" }, { rid: zigbeeConnectivity.id, rtype: "zigbee_connectivity" }, { rid: entertainment.id, rtype: "entertainment" }];
        device.type = "device";

        deviceConfig.data.push(device);

        // Add corresponding channel, light service and location into entertainment configuration.
        var channel = {};
        channel.channel_id = channelId;
        channel.members = [{ index: 0, service: { rid: entertainment.id, rtype: "entertainment" } }];
        channel.position = { x: location[0], y: location[1], z: 0.0 };

        ec.channels.push(channel);

        var lightService = {};
        lightService.rid = light.id;
        lightService.rtype = "light";
        ec.light_services.push(lightService);

        var serviceLocation = {};
        serviceLocation.position = { x: location[0], y: location[1], z: 0.0 };
        serviceLocation.service = { rid: entertainment.id, rtype: "entertainment" };
        ec.locations.service_locations.push(serviceLocation);

        // Create add messages
        var msg = createMessageProto();
        var addMessage = createMessageDataProto("add");
        addMessage += JSON.stringify(light);

        msg += addMessage;
        msg += "]},";

        addMessage = createMessageDataProto("add");
        addMessage += JSON.stringify(zigbeeConnectivity);

        msg += addMessage;
        msg += "]},";

        addMessage = createMessageDataProto("add");
        addMessage += JSON.stringify(device);

        msg += addMessage;
        msg += "]},";

        addMessage = createMessageDataProto("update");
        addMessage += JSON.stringify(ec);
        msg += addMessage;
        msg += "]}]";

        // Save config
        saveConfigs();

        return { success: true, update: msg };
    }

    function deleteResources(resources, idToDelete, idToKeep) {
        for (var i = 0; i < resources.length; i++) {
            var keep = false;

            for (let id of idToKeep) {
                if (id === resources[i].id) {
                    keep = true;
                    break;
                }
            }

            if (keep) {
                continue;
            }

            keep = true;
            for (let id of idToDelete) {
                if (id === resources[i].id) {
                    keep = false;
                    break;
                }
            }

            if (!keep) {
                resources.splice(i, 1);
                i--;
            }
        }
    }

    that.deleteChannel = function (ecId, channelId) {
        var ec = this.getEntertainmentConfiguration(ecId);

        if (ec === undefined) {
            return { success: false, update: undefined };
        }

        var entertainmentId = [];
        var entertainmentToKeepId = [];
        var deviceId = [];
        var deviceToKeepId = [];
        var zigbeeConnectivityId = [];
        var zigbeeConnectivityToKeepId = [];
        var lightId = [];
        var lightToKeepId = [];

        // Update entertainment_configuration
        for (var i = 0; i < ec.channels.length; i++) {
            if (ec.channels[i].channel_id === channelId) {
                for (var j = 0; j < ec.channels[i].members.length; j++) {
                    entertainmentId.push(ec.channels[i].members[j].service.rid);
                }

                ec.channels.splice(i, 1);
                break;
            }
        }

        // In case of multi light channels, we wanna remove the whole light so check for the other channels
        for (var i = 0; i < ec.channels.length; i++) {
            for (var j = 0; j < ec.channels[i].members.length; j++) {

                for (var k = 0; k < entertainmentId.length; k++) {
                    if (ec.channels[i].members[j].service.rid === entertainmentId[k]) {
                        ec.channels[i].members.splice(j, 1);
                        j--;
                        break;
                    }
                }

                if (ec.channels[i].members.length === 0) {
                    ec.channels.splice(i, 1);
                    i--;
                    break;
                }
            }
        }

        for (var i = 0; i < ec.locations.service_locations.length; i++) {
            var found = false;

            for (var j = 0; j < entertainmentId.length; j++) {
                if (ec.locations.service_locations[i].service.rid === entertainmentId[j]) {
                    found = true;
                    break;
                }
            }

            if (found) {
                ec.locations.service_locations.splice(i, 1);
                i--;
            }
        }

        // Make sure this entertainment id is not used by other entertainment configuration, otherwise skip it
        for (let otherEC of entertainmentConfigurationConfig.data) {
            for (let channel of otherEC.channels) {
                for (let member of channel.members) {
                    for (var i = 0; i < entertainmentId.length; i++) {
                        if (member.service.rid === entertainmentId[i]) {
                            entertainmentToKeepId.push(member.service.rid);
                            break;
                        }
                    }
                }
            }
        }

        // Look for device, light and zigbee_connectivity ids
        for (var i = 0; i < deviceConfig.data.length; i++) {
            deviceServices = deviceConfig.data[i].services;
            var found = false;
            var keep = false;

            for (var j = 0; j < deviceServices.length; j++) {
                for (var k = 0; k < entertainmentId.length; k++) {
                    if (deviceServices[j].rtype === "entertainment" && deviceServices[j].rid === entertainmentId[k]) {

                        for (let entertainmentId of entertainmentToKeepId) {
                            if (deviceServices[j].rid === entertainmentId) {
                                keep = true;
                                break;
                            }
                        }

                        for (var l = 0; l < deviceServices.length; l++) {
                            if (deviceServices[l].rtype === "light") {
                                lightId.push(deviceServices[l].rid);
                                if (keep) {
                                    lightToKeepId.push(deviceServices[l].rid);
                                }
                            }
                            else if (deviceServices[l].rtype === "zigbee_connectivity") {
                                zigbeeConnectivityId.push(deviceServices[l].rid);

                                if (keep) {
                                    zigbeeConnectivityToKeepId.push(deviceServices[l].rid);
                                }
                            }
                        }

                        found = true;
                        break;
                    }
                }

                if (found) {
                    break;
                }
            }

            if (found) {
                deviceId.push(deviceConfig.data[i].id);

                if (keep) {
                    deviceToKeepId.push(deviceConfig.data[i].id);
                }
            }
        }

        // Update entertainment configuration light services
        for (var i = 0; i < ec.light_services.length; i++) {
            var found = false;

            for (var j = 0; j < lightId.length; j++) {
                if (ec.light_services[i].rid === lightId[j]) {
                    found = true;
                    break;
                }
            }

            if (found) {
                ec.light_services.splice(i, 1);
                i--;
            }
        }

        // Delete light
        deleteResources(lightConfig.data, lightId, lightToKeepId);
        deleteResources(zigbeeConnectivityConfig.data, zigbeeConnectivityId, zigbeeConnectivityToKeepId);
        deleteResources(entertainmentConfig.data, entertainmentId, entertainmentToKeepId);
        deleteResources(deviceConfig.data, deviceId, deviceToKeepId);

        // Create delete message
        var msg = createMessageProto();
        for (let light of lightId) {
            var keep = false;
            for (let lightToKeep of lightToKeepId) {
                if (light === lightToKeep) {
                    keep = true;
                    break;
                }
            }

            if (keep) {
                continue;
            }

            var deleteMessage = createMessageDataProto("delete");
            deleteMessage += "{\"type\": \"light\", \"id\":\"" + light + "\"}";

            msg += deleteMessage;
            msg += "]},";
        }

        for (let entertainment of entertainmentId) {
            var keep = false;
            for (let entertainmentToKeep of entertainmentToKeepId) {
                if (entertainment === entertainmentToKeep) {
                    keep = true;
                    break;
                }
            }

            if (keep) {
                continue;
            }

            var deleteMessage = createMessageDataProto("delete");
            deleteMessage += "{\"type\":\"entertainment\", \"id\":\"" + entertainment + "\"}";

            msg += deleteMessage;
            msg += "]},";
        }

        for (let zigbeeConnectivity of zigbeeConnectivityId) {
            var keep = false;
            for (let zigbeeConnectivityToKeep of zigbeeConnectivityToKeepId) {
                if (zigbeeConnectivity === zigbeeConnectivityToKeep) {
                    keep = true;
                    break;
                }
            }

            if (keep) {
                continue;
            }

            var deleteMessage = createMessageDataProto("delete");
            deleteMessage += "{\"type\":\"zigbee_connectivity\", \"id\":\"" + zigbeeConnectivity + "\"}";

            msg += deleteMessage;
            msg += "]},";
        }

        addMessage = createMessageDataProto("update");
        addMessage += JSON.stringify(ec);
        msg += addMessage;
        msg += "]}]";

        saveConfigs();

        return { success: true, update: msg };
    }

	init();
}

// export the class
module.exports = ResourcesManager;