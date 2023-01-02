package com.lighting.huestream.integrationtests.helpers;

import java.util.List;

public interface IBridgeWrapper {
    interface ILightCoordinate {
        Integer getLightId();
        Double getX();
        Double getY();
    }
    interface ILightID {
        Integer getLightId();
        String getMac();
    }

    String getLightRBGColor(ILightID lightId);

    Integer getEntertainmentGroupId();

    void removeEntertainmentGroups();

    List<ILightID> getLLCLightsIDs();

    void includeLightsIntoGroup(final List<ILightID> lights, final Integer targetGroupId);

    void setLightsCoordinates(final Integer groupId, List<ILightCoordinate> lightCoordinates);

    void cleanupUser();
}