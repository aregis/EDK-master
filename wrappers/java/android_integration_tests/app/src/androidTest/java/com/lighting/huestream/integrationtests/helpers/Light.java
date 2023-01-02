package com.lighting.huestream.integrationtests.helpers;

public class Light {
    public enum Position {
        FrontLeft,
        FrontRight,
        RearLeft,
        RearRight
    }

    public Light(Position position, IBridgeWrapper.ILightID lightID) {
        _position = position;
        _id = lightID.getLightId();
        _mac = lightID.getMac();
    }

    public Integer getLightId() {
        return _id;
    }

    public String getMac() {
        return _mac;
    }

    public double getX() {
        if ((_position == Position.FrontLeft) || (_position == Position.RearLeft)) {
            return -1.0;
        } else {
            return 1.0;
        }
    }

    public double getY() {
        if ((_position == Position.FrontLeft) || (_position == Position.FrontRight)) {
            return 1.0;
        } else {
            return -1.0;
        }
    }

    public IBridgeWrapper.ILightID asLightID() {
        final Light that = this;

        return new IBridgeWrapper.ILightID() {
            @Override
            public Integer getLightId() {
                return that.getLightId();
            }

            @Override
            public String getMac() {
                return that.getMac();
            }
        };
    }

    public IBridgeWrapper.ILightCoordinate asLightCoordinate() {
        final Light that = this;

        return new IBridgeWrapper.ILightCoordinate() {
            @Override
            public Integer getLightId() {
                return that.getLightId();
            }

            @Override
            public Double getX() {
                return that.getX();
            }

            @Override
            public Double getY() {
                return that.getY();
            }
        };
    }

    final private Position _position;
    final private Integer _id;
    final private String _mac;
}
