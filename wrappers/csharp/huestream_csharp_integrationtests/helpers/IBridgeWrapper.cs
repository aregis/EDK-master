using System;
using System.Collections.Generic;

namespace huestream_tests
{
    public interface ILightCoordinate
    {
        int GetLightId();
        double GetX();
        double GetY();
    }

    public interface ILightID
    {
        int GetLightId();
        String GetMac();
    }

    public interface IBridgeWrapper
    {
        void RemoveEntertainmentGroups();

        int GetEntertainmentGroupId();

        String GetLightRGBColor(ILightID lightId);

        List<ILightID> GetLLCLightsIDs();

        void IncludeLightsIntoGroup(List<ILightID> lights, int targetGroupId);

        void SetLightsCoordinates(int groupId, List<ILightCoordinate> lightCoordinates);

        void CleanUpUser();
    }
}
