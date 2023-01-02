/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

function SimGui() {
    const INVALID_GROUP_ID = -1;
    const MARKER_COLOR = 0x00CCFF;
    const MARGIN = 10;
    const LOGO_POS_X = MARGIN;
    const LOGO_POS_Y = MARGIN;
    const ITEM_SPACING = 15;
    const HEADING_COLOR = 0xFFFF00;
    const HEADING_FONT = "Trebuchet MS";
    const PROPERTY_LABEL_COLOR = 0xFFFFFF;
    const PROPERTY_VALUE_COLOR = 0x999999;
    const PROPERTY_LABEL_MAX_SIZE = 100;
    const PROPERTY_LABEL_POS_X = MARGIN;
    const PROPERTY_LABEL_POS_Y = 100;
    const PROPERTY_VALUE_POS_X = PROPERTY_LABEL_POS_X + PROPERTY_LABEL_MAX_SIZE;
    const PROPERTY_VALUE_POS_Y = PROPERTY_LABEL_POS_Y;
    const PROPERTY_LABEL_FONT = "Trebuchet MS";
    const PROPERTY_VALUE_FONT = "Courier New";
    const PROPERTY_ACTIVE_GROUP = "active group";
    const PROPERTY_OWNER = "owner";
    const ACTION_COLOR = 0xFFFFFF;
    const ACTION_COLOR_OVER = 0xFF00FF;
    const ACTION_POS_X = MARGIN;
    const ACTION_POS_Y = PROPERTY_LABEL_POS_Y + 2 * (ITEM_SPACING + 20);
    const ACTION_FONT = "Trebuchet MS";
    const PROPERTY_STREAMING_STATE = "streaming state";
    const ACTION_CURRENT_GROUP_ACTIVE_FALSE = "set current group active to false";
    const ACTION_CURRENT_GROUP_OTHER_OWNER = "set current group other owner";
    const CLIP_V2_MIN_SWVERSION = 1941088000;

    var targetHtmlElement = $('#simview');
    var screenWidth = targetHtmlElement.width();
    var screenHeight = targetHtmlElement.height();
    var renderer;
    var stage;
    var graphics;
    var grid;
    var lights = [];
    var currentGroups;
    var currentGroup;
    var currentGroupId = INVALID_GROUP_ID;
    var timer;
    var properties = {};
    var propertiesCount = 0;
    var actions = {};
    var actionsCount = 0;
    var streamer;
    var isClipv2 = false;

    function drawProperty(text) {
        if (propertiesCount === 0) {
            var header = new PIXI.Text("Properties", { fontSize: 12, fontFamily: HEADING_FONT, fill: HEADING_COLOR});
            header.x = PROPERTY_LABEL_POS_X;
            header.y = PROPERTY_LABEL_POS_Y;
            stage.addChild(header);
        }

        var yOffsetStep =  (propertiesCount + 1) * ITEM_SPACING;
        var label = new PIXI.Text(text, { fontSize: 12, fontFamily: PROPERTY_LABEL_FONT, fill: PROPERTY_LABEL_COLOR});
        label.x = PROPERTY_LABEL_POS_X;
        label.y = PROPERTY_LABEL_POS_Y + yOffsetStep;
        stage.addChild(label);

        var value = new PIXI.Text(":", { fontSize: 12, fontFamily: PROPERTY_VALUE_FONT, fill: PROPERTY_VALUE_COLOR});
        value.x = PROPERTY_VALUE_POS_X;
        value.y = PROPERTY_VALUE_POS_Y + yOffsetStep;
        stage.addChild(value);

        properties[text] = value;
        propertiesCount++;
    }

    function drawAction(text, click) {
        if (actionsCount === 0) {
            var header = new PIXI.Text("Actions", { fontSize: 12, fontFamily: HEADING_FONT, fill: HEADING_COLOR});
            header.x = ACTION_POS_X;
            header.y = ACTION_POS_Y;
            stage.addChild(header);
        }

        var yOffsetStep =  (actionsCount + 1) * ITEM_SPACING;
        var action = new PIXI.Text(" > " + text, { fontSize: 12, fontFamily: ACTION_FONT, fill: ACTION_COLOR});
        action.x = ACTION_POS_X;
        action.y = ACTION_POS_Y + yOffsetStep;
        action.interactive = true;
        action.buttonMode = true;
        action
            .on('pointerup',  function(){ click();})
            .on('pointerover', function(){ this.style.fill  = ACTION_COLOR_OVER;})
            .on('pointerout', function(){ this.style.fill  = ACTION_COLOR;});


        stage.addChild(action);

        actions[text] = action;
        actionsCount++;
    }

    function updateField(text, value) {
        properties[text].text = ": " + value;
    }

    function isStreaming() {
        return currentGroupId !== INVALID_GROUP_ID;
    }

    function executeActionCurrentGroupActiveToFalse() {
        if (!isStreaming())
            return;
        console.log("TEXT_ACTION_ACTIVE_CURRENT_GROUP");
        restcallState(currentGroupId, false, null, function () {});
    }

    function executeActionCurrentGroupOtherOwner() {
        if (!isStreaming())
            return;
        console.log("TEXT_ACTION_ACTIVE_CURRENT_GROUP");
        if (isClipv2) {
            // On clipv2 we need to stop streaming first
            executeActionCurrentGroupActiveToFalse();
        }

        restcallState(currentGroupId, true, "otherowner", function () {});
    }

    function drawLogo() {
        var logo = PIXI.Sprite.fromImage('client/hue.png');
        logo.x = LOGO_POS_X;
        logo.y = LOGO_POS_Y;
        stage.addChild(logo);
    }

    function findLightId(x, y) {
        if (isClipv2) {
            for (let channel of currentGroup.channels) {
                if (channel.position.x > (x - 0.01) && channel.position.x < (x + 0.01) && channel.position.y > (y - 0.01) && channel.position.y < (y + 0.01)) {
                    return channel.channel_id;
                }
            }
        }
        else {
            $.each(currentGroup.locations, function (id, pos) {
                if (pos[0] > (x - 0.01) && pos[0] < (x + 0.01) && pos[1] > (y - 0.01) && pos[1] < (y + 0.01)) {
                    return id;
                }
            });
        }

        return -1;
    }

    function drawGrid() {
        for (var x = -1; x < 1; x+=.1) {
            for (var y = -1; y < 1; y+=.1) {
                var graphics = new PIXI.Graphics();
                var start = grid.getPixelCoordinate(x, y);
                var rectangle = new PIXI.Rectangle(start.x, start.y, grid.step, grid.step);

                graphics.lineStyle(1, 0x333333);
                graphics.drawRect(rectangle.x, rectangle.y, rectangle.width, rectangle.height);
                stage.addChild(graphics);
            }
        }
    }

    function restcallPutLight(groupdId, x, y, done) {
        var data = {
            location: [x, y]
        };
        $.ajax({
            type: 'PUT',
            contentType: 'application/json',
            url: "/develop/groupconfig/" + groupdId + "/light",
            headers: {"X-HTTP-Method-Override": "PUT"},
            data: JSON.stringify(data)
        }).done(function () {
            done();
        });
    }

    function restcallDeleteLight(groupId, lightId, done) {
        $.ajax({
            type: 'DELETE',
            contentType: 'application/json',
            url: "/develop/groupconfig/" + groupId +"/light/" + lightId,
            headers: {"X-HTTP-Method-Override": "DELETE"},
        }).done(function () {
            done();
        });
    }

    function restcallState(groupdId, active, owner, done) {
        var data = {
            active: active,
            owner: owner
        };

        $.ajax({
            type: 'PUT',
            contentType: 'application/json',
            url: "/develop/groupconfig/" + groupdId + "/state",
            headers: { "X-HTTP-Method-Override": "PUT" },
            data: JSON.stringify(data)
        }).done(function () {
            done();
        });
    }

    function drawHitGrid() {4
        for (var x = -1; x < 1; x+=.1) {
            for (var y = -1; y < 1; y+=.1) {
                var graphics = new PIXI.Graphics();
                var start = grid.getPixelCoordinate(x, -y);
                var rectangle = new PIXI.Rectangle(start.x, start.y, grid.step, grid.step);
                graphics.beginFill(0xFFFFFF);
                graphics.alpha = 0;
                graphics.drawRect(rectangle.x, rectangle.y, rectangle.width, rectangle.height);
                graphics.interactive = true;
                graphics.buttonMode = true;
                graphics.hitArea = new PIXI.Rectangle(start.x, start.y, grid.step, grid.step);

                var click = function (x, y) {
                    graphics.click = function (e) {
                        if (!isStreaming()) {
                            return;
                        }

                        var id = findLightId(x, y);
                        if (id !== -1) {
                            //remove
                            restcallDeleteLight(currentGroupId, id, function () {
                                if (!isClipv2) {
                                    restcallGetGroups(function () {
                                        currentGroup = currentGroups[currentGroupId];
                                        removeAllLightFromStage();
                                        drawLights();
                                        streamer.Stop();
                                        streamer.Start(currentGroup.locations);
                                    });
                                }
                            });
                        }
                        else {
                            if (!isStreaming()) {
                                return;
                            }
                            //add

                            restcallPutLight(currentGroupId, x, y, function () {
                                if (!isClipv2) {
                                    restcallGetGroups(function () {
                                        currentGroup = currentGroups[currentGroupId];
                                        removeAllLightFromStage();
                                        drawLights();
                                        streamer.Stop();
                                        streamer.Start(currentGroup.locations);
                                    });
                                }
                            });
                        }
                    };
                }(x, y);

                graphics.mouseover = function (e) {
                    this.alpha = 0.2;
                };
                graphics.mouseout = function (e) {
                    this.alpha = 0;
                };
                stage.addChild(graphics);
            }
        }
    }

    function componentToHex(c) {
        var hex = c.toString(16);
        return hex.length == 1 ? "0" + hex : hex;
    }

    function rgbToHex(r, g, b) {
        return "0x" + componentToHex(r) + componentToHex(g) + componentToHex(b);
    }

    function changeColor(id, r, g, b) {
        lights[id].radiationSprite.tint = rgbToHex(r, g, b);
        changeColorLabel(id, r, g, b);
    }

    function changeColorLabel(id, r, g, b) {
        lights[id].colorText.text = (r + " " + g + " " + b);
    }

    function generateRadiationSprite(x, y) {
        var sprite = PIXI.Sprite.fromImage("client/content/glow.png");
        sprite.anchor.set(0.5, 0.5);
        sprite.height = grid.step * 5;
        sprite.width = grid.step * 5;
        sprite.position.x = x;
        sprite.position.y = y;
        //sprite.scale.set(0.3);
        sprite.blendMode = PIXI.BLEND_MODES.SCREEN;
        return sprite;
    }

    function generateLightMarker(x, y) {
        var circle = new PIXI.Graphics();
        var radius = grid.step / 2;
        circle.lineStyle(1, MARKER_COLOR);  //(thickness, color)
        circle.drawCircle(x, y, radius);   //(x,y,radius)
        circle.endFill();

        return circle;
    }

    function generateLabel(x, y, id) {
        var label = new PIXI.Text(String(id), {font: "13px Trebuchet MS", fill: MARKER_COLOR});
        //label.width = grid.step * .3;
        //label.height = grid.step * .3;
        label.anchor.set(0.5, 1);
        label.position.x = x;
        label.position.y = y;
        return label;
    }

    function updateFieldActiveGroup() {
        var name = "-";
        var id = "-";
        if (isStreaming()) {
            name = currentGroup.name;
            id = currentGroupId;
        }
        updateField(PROPERTY_ACTIVE_GROUP, name + " (" + id + ")");
    }

    function updateFieldState(text) {
        updateField(PROPERTY_STREAMING_STATE, text);
    }

    function updateFieldOwner() {
        var owner = "-";
        if (isStreaming()) {
            owner = isClipv2 ? currentGroup.active_streamer.rid : currentGroup.stream.owner;
        }
        updateField(PROPERTY_OWNER, owner );
    }

    function generateColorText(x, y, id) {
        var label = new PIXI.Text("0 0 0", {font: "8px Courier New", fill: MARKER_COLOR});
        label.anchor.set(0.5, 0.0);
        //label.width = grid.step * .6;
        //label.height = grid.step * .4;
        label.position.x = x;
        label.position.y = y;
        return label;
    }

    function drawLight(id, x, y) {
        var pos = grid.getPixelCoordinate(x, -y);

        // not on the line but on the position in between:
        pos.x += grid.step / 2;
        pos.y += grid.step / 2;

        lights[id] = {
            radiationSprite: generateRadiationSprite(pos.x, pos.y),
            lightMarker: generateLightMarker(pos.x, pos.y),
            label: generateLabel(pos.x, pos.y, id),
            colorText: generateColorText(pos.x, pos.y)
        };
        changeColor(id, 0, 0, 0);
        addLightToStage(lights[id]);
    }

    function drawLights() {
        if (isClipv2) {
            for (let channel of currentGroup.channels) {
                drawLight(channel.channel_id, channel.position.x, channel.position.y);
            }
        }
        else {
            $.each(currentGroup.locations, function (id, pos) {
                drawLight(id, pos[0], pos[1]);
            });
        }
    }

    function addLightToStage(light) {
        stage.addChild(light.radiationSprite);
        stage.addChild(light.label);
        stage.addChild(light.colorText);
        stage.addChild(light.lightMarker);
    }

    function removeAllLightFromStage() {
        for (var key in lights) {
            var light = lights[key];
            stage.removeChild(light.radiationSprite);
            stage.removeChild(light.label);
            stage.removeChild(light.colorText);
            stage.removeChild(light.lightMarker);
        }
    }

    function animate() {
        requestAnimationFrame(animate);
        renderer.render(stage);
    }

    function restcallGetGroups(done) {
        $.getJSON("/api/simUser/groups", function (groups) {
            currentGroups = groups;
            done();
        });
    }

    function getActiveGroup() {
        for (var groupId in currentGroups) {
            var activeGroup = currentGroups[groupId];
            if (activeGroup.stream.active === true) {
                return groupId;
            }
        }
        return undefined;
    }

    function UpdateGroups() {
        restcallGetGroups(function () {
            OnUpdateGroups();
        });
    }

    function OnUpdateGroups() {
        var newActiveGroup = isClipv2 ? getActiveEntertainmentConfiguration() : getActiveGroup();

        if (newActiveGroup === undefined) {
            currentGroupId = INVALID_GROUP_ID;

            streamer.Stop();
            removeAllLightFromStage();
            updateFieldActiveGroup();
            updateFieldOwner();
            updateField(PROPERTY_STREAMING_STATE, 'idle');
            return;
        }

        if (!isClipv2 && currentGroupId === newActiveGroup) {
            return;
        }

        currentGroupId = isClipv2 ? newActiveGroup.id : newActiveGroup;
        currentGroup = isClipv2 ? newActiveGroup : currentGroups[currentGroupId];

        streamer.Stop();
        removeAllLightFromStage();
        drawLights();
        updateFieldActiveGroup();
        updateFieldOwner();
        streamer.Start(isClipv2 ? currentGroup.channels : currentGroup.locations);
    }

    function restcallGetEntertainmentConfiguration(done) {
        $.getJSON("/clip/v2/resource/entertainment_configuration", function (data) {
            currentGroups = data.data;
            done();
        });
    }

    function getActiveEntertainmentConfiguration() {
        for (let ec of currentGroups) {
            if (ec.status === "active") {
                return ec;
            }
        }

        return undefined;
    }

    function setEntertainmentConfigurationStatus(id, status) {
        for (let ec of currentGroups) {
            if (ec.id === id) {
                ec.status = status;

                if (status === 'inactive') {
                    delete ec.active_streamer;
                }

                break;
            }
        }
    }

    function setEntertainmentCongigurationStreamer(id, streamer) {
        for (let ec of currentGroups) {
            if (ec.id === id) {
                ec.active_streamer = streamer;
                break;
            }
        }
    }

    function setEntertainmentConfigurationChannles(id, channels) {
        for (let ec of currentGroups) {
            if (ec.id === id) {
                ec.channels = channels;
                break;
            }
        }
    }

    function OnClipV2Event(event) {
        console.log(event.data);

        root = JSON.parse(event.data);

        for (var i = 0; i < root.length; i++) {
            var update = root[i];
            if (update.type !== "update") {
                continue;
            }

            var updateData = update.data[0];

            if (updateData.type !== "entertainment_configuration") {
                continue;
            }

            if (updateData.status !== undefined) {
                setEntertainmentConfigurationStatus(updateData.id, updateData.status);
            }

            if (updateData.active_streamer !== undefined) {
                setEntertainmentCongigurationStreamer(updateData.id, updateData.active_streamer);
            }

            if (updateData.channels !== undefined) {
                setEntertainmentConfigurationChannles(updateData.id, updateData.channels);
            }
        }

        OnUpdateGroups();
    }

    function initGui() {
        renderer = new PIXI.WebGLRenderer(screenWidth, screenHeight, {antialias: true, resolution: 1});
        targetHtmlElement.append(renderer.view);
        stage = new PIXI.Container();
        graphics = new PIXI.Graphics();
        stage.addChild(graphics);

        drawLogo();
        drawProperty(PROPERTY_ACTIVE_GROUP)
        drawProperty(PROPERTY_OWNER)
        drawProperty(PROPERTY_STREAMING_STATE)
        drawAction(ACTION_CURRENT_GROUP_ACTIVE_FALSE, executeActionCurrentGroupActiveToFalse);
        drawAction(ACTION_CURRENT_GROUP_OTHER_OWNER, executeActionCurrentGroupOtherOwner);

        grid = new Grid(screenWidth, screenHeight);
        drawGrid();
        drawHitGrid();

        var loader = PIXI.loader;
        loader.add('glow', "client/content/glow.png");
        loader.once('complete', animate);
        loader.load();
    }

    function init() {
        $.ajax({
            method: 'GET',
            url: "/api/config",
            type: 'json',
            error: function (querry, err, exc) {
                console.log(err);

                // Suppose we're on clipv1 here
                initGui();
                streamer = new Streamer(changeColor, updateFieldState);
                timer = setInterval(UpdateGroups, 1000);
            },
            success: function (config) {
                isClipv2 = config.swversion >= CLIP_V2_MIN_SWVERSION;
                console.log("Using clipv2: " + isClipv2);

                initGui();
                streamer = new Streamer(changeColor, updateFieldState);

                if (isClipv2) {
                    restcallGetEntertainmentConfiguration(function () {
                        OnUpdateGroups();

                        // Clipv2 eventing connection
                        var source = new EventSource("/eventstream/clip/v2");
                        source.onmessage = OnClipV2Event;
                        source.onerror = function () {
                            console.log("Eventing connection failed!");
                        }
                    });
                }
                else {
                    timer = setInterval(UpdateGroups, 1000);
                }
            }
        });
    }

    init();
}

new SimGui();
