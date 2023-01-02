using System;
using huestream_tests;

public class Light
{
    public enum Position
    {
        FrontLeft,
        FrontRight,
        RearLeft,
        RearRight
    }

    private class LightCoordinateImpl : ILightCoordinate
    {
        public LightCoordinateImpl(int lightId, double x, double y)
        {
            _lightId = lightId;
            _x = x;
            _y = y;
        }

        public int GetLightId()
        {
            return _lightId;
        }

        public double GetX()
        {
            return _x;
        }

        public double GetY()
        {
            return _y;
        }

        private int _lightId;
        private double _x;
        private double _y;
    }

    private Position _position;
    private int _id;
    private String _mac;

    public Light(Position position, ILightID lightID)
    {
        _position = position;
        _id = lightID.GetLightId();
        _mac = lightID.GetMac();
    }

    public int GetLightId()
    {
        return _id;
    }

    public String GetMac()
    {
        return _mac;
    }

    public double GetX()
    {
        if ((_position == Position.FrontLeft) || (_position == Position.RearLeft))
        {
            return -1.0;
        }
        else
        {
            return 1.0;
        }
    }

    public double GetY()
    {
        if ((_position == Position.FrontLeft) || (_position == Position.FrontRight))
        {
            return 1.0;
        }
        else
        {
            return -1.0;
        }
    }

    public ILightID AsLightID()
    {
        return new LightIDImpl(this.GetLightId(), this.GetMac());
    }

    public ILightCoordinate AsLightCoordinate()
    {
        return new LightCoordinateImpl(_id, GetX(), GetY());
    }

    internal object getLightId()
    {
        throw new NotImplementedException();
    }
}
