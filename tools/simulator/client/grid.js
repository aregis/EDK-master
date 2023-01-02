/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

function Grid(width, height){
    this.margin = 100;
    this.length = Math.min(width, height) - this.margin;
    this.step = this.length / 20;
    this.offsetX = (width - this.length) / 2;
    this.offsetY = (height - this.length) / 2;

    var that = this;

    this.getPixelCoordinate = function(x, y){
        var cX = that.offsetX + (that.length / 2 + x * that.length / 2);
        var cY = that.offsetY + (that.length / 2 + y * that.length / 2);
        return {
            x: cX,
            y: cY
        }
    }
}
