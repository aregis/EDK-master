cmake_minimum_required(VERSION 3.6)

list(APPEND BRIDGE_DISCOVERY_SOURCES
        include/bridgediscovery/BridgeDiscovery.h
        include/bridgediscovery/BridgeDiscoveryCheckIpTask.h
        include/bridgediscovery/BridgeDiscoveryClassType.h
        include/bridgediscovery/BridgeDiscoveryConfiguration.h
        include/bridgediscovery/BridgeDiscoveryConst.h
        include/bridgediscovery/BridgeDiscoveryIpCheckResult.h
        include/bridgediscovery/BridgeDiscoveryResult.h
        include/bridgediscovery/BridgeDiscoveryReturnCode.h
        include/bridgediscovery/BridgeInfo.h
        include/bridgediscovery/IBridgeDiscoveryCallback.h
        src/BridgeDiscovery.cpp
        src/BridgeDiscoveryConfiguration.cpp
        src/BridgeDiscoveryResult.cpp
        src/BridgeInfo.cpp
        src/events/BridgeDiscoveryEvents.h
        src/events/BridgeDiscoveryEventNames.h
        src/events/BridgeDiscoveryEventNotifier.h
        src/events/BridgeDiscoveryEventTranslator.h
        src/events/IBridgeDiscoveryEventNotifier.h
        src/method/BridgeDiscoveryMethodBase.h
        src/method/BridgeDiscoveryMethodFactory.h
        src/method/BridgeDiscoveryMethodFactoryDefault.cpp
        src/method/BridgeDiscoveryMethodUtil.cpp
        src/method/BridgeDiscoveryMethodUtil.h
        src/method/IBridgeDiscoveryMethod.h
        src/method/ipscan/BridgeDiscoveryIpscan.cpp
        src/method/ipscan/BridgeDiscoveryIpscan.h
        src/method/ipscan/BridgeDiscoveryIpscanPreCheck.cpp
        src/method/ipscan/BridgeDiscoveryIpscanPreCheck.h
        src/method/ipscan/tasks/BridgeDiscoveryIpscanTask.cpp
        src/method/ipscan/tasks/BridgeDiscoveryIpscanTask.h
        src/method/mdns/BridgeDiscoveryMDNS.cpp
        src/method/mdns/MDNSResponder.cpp
        src/method/mdns/BridgeDiscoveryMDNS.h
        src/method/mdns/IHueMDNSService.h
        src/method/mdns/tasks/BridgeDiscoveryMDNSTask.h
        src/method/nupnp/BridgeDiscoveryNupnp.cpp
        src/method/nupnp/BridgeDiscoveryNupnp.h
        src/method/nupnp/tasks/BridgeDiscoveryNupnpTask.cpp
        src/method/nupnp/tasks/BridgeDiscoveryNupnpTask.h
        src/method/upnp/BridgeDiscoveryUpnp.cpp
        src/method/upnp/BridgeDiscoveryUpnp.h
        src/method/upnp/tasks/BridgeDiscoveryUpnpListenTask.cpp
        src/method/upnp/tasks/BridgeDiscoveryUpnpListenTask.h
        src/method/upnp/tasks/BridgeDiscoveryUpnpSendTask.cpp
        src/method/upnp/tasks/BridgeDiscoveryUpnpSendTask.h
        src/method/upnp/tasks/BridgeDiscoveryUpnpTask.cpp
        src/method/upnp/tasks/BridgeDiscoveryUpnpTask.h
        src/tasks/BridgeDiscoveryCheckIpArrayTask.cpp
        src/tasks/BridgeDiscoveryCheckIpArrayTask.h
        src/tasks/BridgeDiscoveryCheckIpTask.cpp
        src/tasks/BridgeDiscoveryTask.cpp
        src/tasks/BridgeDiscoveryTask.h
        src/method/mdns/HueMDNSService.cpp
        src/method/mdns/HueMDNSService.h
        src/method/mdns/IHueMDNSService.h
        src/method/mdns/MDNSResponder.h
        src/method/mdns/IMDNSResponder.h
        src/method/mdns/MDNSResponderProvider.h
        src/method/mdns/MDNSResponderProvider.cpp
)

if (WIN32)
    list(APPEND BRIDGE_DISCOVERY_SOURCES
            src/method/mdns/MDNSResponderWindowsUtilities.cpp)
else (WIN32)
    list(APPEND BRIDGE_DISCOVERY_SOURCES
            src/method/mdns/MDNSResponderUnixUtilities.cpp)
endif (WIN32)

add_library(bridge_discovery STATIC ${BRIDGE_DISCOVERY_SOURCES})
set_target_properties(bridge_discovery PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(bridge_discovery PUBLIC "./include/")
target_include_directories(bridge_discovery PRIVATE "./src/")
target_link_libraries(bridge_discovery PUBLIC support json mdns_responder)
treat_warning_as_error(bridge_discovery)

