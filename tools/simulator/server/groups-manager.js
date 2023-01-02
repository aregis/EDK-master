function GroupsManager() {
    var fs = require('fs');
    const DEFAULT_CONFIG = './server/config/default_groups.json';
    const CURRENT_CONFIG = './server/config/current_groups.json';
    var groupConfig;
    var that = this;

    function loadGroupConfig(file) {
        var success = false;

        try {
            var serialized = fs.readFileSync(file, 'utf8');
            if (serialized) {
                groupConfig = JSON.parse(serialized);
                success = true;
            }
        }
        catch (err) {
            console.log(err);
        }

        for (var groupId in groupConfig) {
            var group = groupConfig[groupId];
            group["stream"] = {
                owner: null,
                active: false
            };
        }

        return success;
    }

    function loadDefaultConfig() {
        loadGroupConfig(DEFAULT_CONFIG);
    }

    function loadConfig() {
        loadGroupConfig(CURRENT_CONFIG);
    }

    function saveGroupConfig() {
        fs.writeFileSync(CURRENT_CONFIG, JSON.stringify(groupConfig, null, 4), 'utf8');
    }

    function init() {
        if (fs.existsSync(CURRENT_CONFIG)) {
            loadConfig();
        }
        else {
            loadDefaultConfig();
        }
    }

    that.setActive = function(groupId, active) {
        console.log("group: " + groupId + ", active: " + active);
        groupConfig[groupId].stream.active = active;
        saveGroupConfig();
    }

    that.setOwner = function(groupId, owner) {
        console.log("group: " + groupId + ", owner: " + owner);
        groupConfig[groupId].stream.owner = owner;
        saveGroupConfig();
    }
    
    that.getActive = function(groupId) {
        return groupConfig[groupId].stream.active;
    }

    that.getOwner = function(groupId) {
        return groupConfig[groupId].stream.owner;
    }

    that.clearActive = function(owner) {
        for (var groupId in groupConfig) {
            var group = groupConfig[groupId];
            if (group.stream.active && group.stream.owner === owner) {
                that.setActive(groupId, false);
                that.setOwner(groupId, null);
            }
        }
        saveGroupConfig();
    }
    
    that.isActive = function(owner) {
        for (var groupId in groupConfig) {
            var group = groupConfig[groupId];
            if (group.stream.active && group.stream.owner === owner) {
                return true;
            }
        }
        return false;
    }
    
    function getFreeId(groupId) {
        var sorted = groupConfig[groupId].lights.sort(function(a, b){return a-b});
        for (var i = 1; i < 63; i++) {
            if (sorted[i] != i) {
                return i;
            }
        }
        return undefined;
    }

    that.addLight = function (groupId, x, y) {
        var id = getFreeId(groupId);

        if (id !== undefined) {
            groupConfig[groupId].lights.push(id);
            groupConfig[groupId].locations[String(id)] = [x, y];
            saveGroupConfig();
        }

        return id;
    };

    that.deleteLight = function (groupId, lightId) {
        var lightArray = groupConfig[groupId].lights;
        for (var i = lightArray.length - 1; i >= 0; i--) {
            if (lightArray[i] === lightId) {
                lightArray.splice(i, 1);
            }
        }
        delete groupConfig[groupId].locations[String(lightId)];
        saveGroupConfig();
    };

    that.getConfig = function () {
        return groupConfig;
    };

    that.getConfigSingle = function (groupId) {
        return groupConfig[groupId];
    };

    that.getLights = function() {

        var lights = {};
        for (var groupId in groupConfig) {
            var group = groupConfig[groupId];
            var lightsInGroup = group.lights;
            for (var i = 0; i < lightsInGroup.length; i++) {
             var lightId = lightsInGroup[i];
                if (lightId in lights) {
                    continue;
                }

                lights[lightId] = {
                    type: "Extended color light",
                    name: "Hue go " + lightId,
                    modelid: "LLC020",
                    manufacturername: "Philips",
                    uniqueid: "00:17:88:01:01:15:b2:80-0b",
                    swversion: "5.38.1.14378"
                }
            }
        }
        return lights;
    }

    init();
}

// export the class
module.exports = GroupsManager;