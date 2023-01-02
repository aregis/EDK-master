/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <algorithm>

#include <boost/optional.hpp>

#include "support/network/_test/Network.h"  // NOLINT(build/include)

#if defined(ENABLE_JNI)
#include "support/jni/core/Core.h"
#include "support/jni/core/HueJNIEnv.h"
#endif

#if defined(CONSOLE_SUPPORT)
#elif defined(_WIN32)
#   define WINAPI_FAMILY WINAPI_FAMILY_DESKTOP_APP
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   include <iphlpapi.h>
#   include <netioapi.h>
#   include <mstcpip.h>
#   include <string>
#elif defined(ANDROID)
#   include <arpa/inet.h>
#   include <netinet/in.h>
#   include <netinet/in6.h>
#   include <net/if.h>
#   include <linux/wireless.h>
#   include <sys/socket.h>
#   include <sys/types.h>
#   include <sys/ioctl.h>
#   include <linux/sockios.h>
#   include <netdb.h>
#   include <unistd.h>
#   include <jni.h>
#   include <string>
#   include "support/jni/SDKSupportJNI.h"
#   include "support/jni/JNIToNative.h"
#   include "support/jni/JNIConstants.h"
#else
#   include <arpa/inet.h>
#   include <ifaddrs.h>
#   include <net/if.h>
#   include <sys/ioctl.h>
#   include <sys/socket.h>
#   include <sys/types.h>
#   include <netdb.h>
#   include <unistd.h>
#   if defined(__linux__)
#       include <linux/wireless.h>
#   endif
#   include <string>
#endif

#include "support/logging/Log.h"

using std::string;

#ifdef _WIN32
using std::runtime_error;
#endif

namespace support {
    bool Network::_default_network_interface_set = false;
    NetworkInterface Network::_default_network_interface;

    void Network::set_default_network_interface(std::string ip, std::string name, NetworkInetType type) {
        _default_network_interface.set_ip(ip);
        _default_network_interface.set_name(name);
        _default_network_interface.set_inet_type(type);
        _default_network_interface.set_up(true);
        _default_network_interface.set_loopback(false);
        _default_network_interface.set_adapter_type(NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRELESS);
        _default_network_interface.set_is_connected(true);
        _default_network_interface_set = true;
    }

#if defined(_WIN32)

    NetworkAdapterType Network::get_adapter_type(const NetworkInterface& network_interface) {
        NetworkAdapterType adapter_type = NetworkAdapterType::NETWORK_ADAPTER_TYPE_UNKNOWN;
        std::string ifname = network_interface.get_name();

        ULONG buflen = sizeof(IP_ADAPTER_INFO);
        IP_ADAPTER_INFO *pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(new char[buflen]);

        if (GetAdaptersInfo(pAdapterInfo, &buflen) == ERROR_BUFFER_OVERFLOW) {
            delete [] reinterpret_cast<char*>(pAdapterInfo);
            pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(new char[buflen]);
        }

        if (GetAdaptersInfo(pAdapterInfo, &buflen) == NO_ERROR) {
            for (IP_ADAPTER_INFO *pAdapter = pAdapterInfo; pAdapter; pAdapter = pAdapter->Next) {
                if (string(pAdapter->GatewayList.IpAddress.String) != "0.0.0.0" && string(pAdapter->AdapterName) == ifname) {
                    switch (pAdapter->Type) {
                        case MIB_IF_TYPE_ETHERNET:
                        case IF_TYPE_ISO88025_TOKENRING:
                        case MIB_IF_TYPE_PPP:
                        case MIB_IF_TYPE_LOOPBACK:
                        case MIB_IF_TYPE_SLIP:
                            adapter_type = NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRED;
                            break;
                        case IF_TYPE_IEEE80211:
                            adapter_type = NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRELESS;
                            break;
                        case MIB_IF_TYPE_OTHER:
                        default:
                            adapter_type = NetworkAdapterType::NETWORK_ADAPTER_TYPE_UNKNOWN;
                    }  // switch (pAdapter->Type)

                    break;
                }  // if (targeted adapter)
            }  // for (pAdapter)
        }  // if (GetAdaptersInfo)

        if (pAdapterInfo) {
            delete [] reinterpret_cast<char*>(pAdapterInfo);
        }

        return adapter_type;
    }


    // #if defined(_WIN32)
#elif defined(__APPLE__)

    NetworkAdapterType Network::get_adapter_type(const NetworkInterface& network_interface) {
        NetworkAdapterType adapter_type = NetworkAdapterType::NETWORK_ADAPTER_TYPE_UNKNOWN;
        std::string ifname = network_interface.get_name();
        bool detection_done = false;

        for (int family = AF_INET; family != -1 && !detection_done; family = (family == AF_INET ? AF_INET6 : -1)) {
            struct ifreq ifreq;
            memset(&ifreq, 0, sizeof ifreq);
            strncpy(ifreq.ifr_name, ifname.c_str(), IFNAMSIZ);

            int socket_fd = socket(family, SOCK_DGRAM, 0);
            if (socket_fd >= 0) {
#   if defined(SIOCGIFFUNCTIONALTYPE)
                if (ioctl(socket_fd, SIOCGIFFUNCTIONALTYPE, &ifreq) == 0) {
                    detection_done = true;

                    switch (ifreq.ifr_ifru.ifru_functional_type) {
                        case IFRTYPE_FUNCTIONAL_WIRED:
                            adapter_type = NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRED;
                            break;
                        case IFRTYPE_FUNCTIONAL_WIFI_INFRA:
                        case IFRTYPE_FUNCTIONAL_WIFI_AWDL:
                            adapter_type = NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRELESS;
                            break;
                        case IFRTYPE_FUNCTIONAL_CELLULAR:
                            adapter_type = NetworkAdapterType::NETWORK_ADAPTER_TYPE_CELLULAR;
                            break;
                        case IFRTYPE_FUNCTIONAL_UNKNOWN:
                        default:
                            break;
                    }
                }
#   endif  // defined(SIOCGIFFUNCTIONALTYPE)

#   if defined(SIOCGIFPHYS)
                if (adapter_type == NetworkAdapterType::NETWORK_ADAPTER_TYPE_UNKNOWN) {
                    if (ioctl(socket_fd, SIOCGIFPHYS, &ifreq) == 0) {
                        detection_done = true;

                        if (ifreq.ifr_ifru.ifru_phys == 0) {
                            adapter_type = NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRELESS;
                        } else {
                            adapter_type = NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRED;
                        }
                    }
                }
#   endif  // defined(SIOCGIFPHYS)

                close(socket_fd);
            }  // if (valid-socket)
        }  // for (family && type-still-unknown)

        return adapter_type;
    }

    // #elif defined(__APPLE__)
#elif defined(__linux__)  // || defined(ANDROID)

    NetworkAdapterType Network::get_adapter_type(const NetworkInterface& network_interface) {
        NetworkAdapterType adapter_type = NetworkAdapterType::NETWORK_ADAPTER_TYPE_UNKNOWN;
        std::string ifname = network_interface.get_name();
        (void)ifname;

#   if defined(SIOCGIWNAME)
        for (int family = AF_INET; family != -1; family = (family == AF_INET ? AF_INET6 : -1)) {
            int socket_fd = socket(family, SOCK_DGRAM, 0);
            if (socket_fd >= 0) {
                struct iwreq pwrq;
                memset(&pwrq, 0, sizeof(pwrq));
                strncpy(pwrq.ifr_name, ifname.c_str(), IFNAMSIZ);
                if (ioctl(socket_fd, SIOCGIWNAME, &pwrq) != -1) {
                    adapter_type = NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRELESS;

                } else {
                    adapter_type = NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRED;
                }

                close(socket_fd);

                break;
            }  // if (valid-socket)
        }  // for (family && type-still-unknown)
#   endif  // defined(SIOCGIWNAME)

        return adapter_type;
    }

    // #elif defined(__linux__)  // || defined(ANDROID)
#else

    NetworkAdapterType Network::get_adapter_type(const NetworkInterface& network_interface) {
        return adapter_type;
    }

#endif

    bool Network::is_wifi_connected() {
        auto ifs = get_network_interfaces();
        for (auto& network_interface : ifs) {
            if (network_interface.get_adapter_type() == NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRELESS) {
                if (network_interface.get_is_connected()) {
                    return true;
                }
            }
        }

        return false;
    }  // Network::is_wifi_connected()

    std::string Network::get_local_interface_name(uint32_t ip_address) {
        auto ifs = get_network_interfaces();
        string interface_name;

        for (auto& network_interface : ifs) {
            if (network_interface.get_adapter_type() == NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRELESS ||
                network_interface.get_adapter_type() == NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRED) {
                if (network_interface.get_is_connected()) {
                    if (network_interface.is_private()) {
                        if (network_interface.is_in_subnet(ip_address)) {
                            interface_name = network_interface.get_name();
                            break;
                        }
                    }
                }
            }
        }

        return interface_name;
    }

    namespace detail {
#if defined(_WIN32)
        static std::pair<bool, SOCKET> create_socket(int family, int protocol) {
            SOCKET socket_fd = socket(family, protocol, 0);
            return {socket_fd != INVALID_SOCKET, socket_fd};
        }

        static void close_socket(SOCKET socket_fd) {
            closesocket(socket_fd);
        }

        // returns > 0 on success
        static unsigned int if_nametoindex(const std::string if_name) {
            NET_LUID luid;
            unsigned int result = 0;
            if (ConvertInterfaceNameToLuidA(if_name.c_str(), &luid) == NO_ERROR) {
                NET_IFINDEX index;
                if (ConvertInterfaceLuidToIndex(&luid, &index) == NO_ERROR) {
                    result = (unsigned int)index;  // NOLINT
                }
            }
            return result;
        }

        static int inet_pton(const std::string& ip, sockaddr_in* addr) {
            WORD wVersionRequested = MAKEWORD(2, 2);
            WSADATA wsaData;

            if (WSAStartup(wVersionRequested, &wsaData) != 0) {
                return -1;
            }

            addr->sin_family = AF_INET;
            int addr_size = sizeof(*addr);
            char* str = new char[ip.size() + 1];
            strcpy(str, ip.c_str());  // NOLINT
            int err = WSAStringToAddress(str, AF_INET, nullptr, reinterpret_cast<LPSOCKADDR>(addr), &addr_size);
            delete[] str;

            WSACleanup();

            return err == 0 ? 1 : -1;
        }

        static int inet_pton(const std::string& ip, sockaddr_in6* addr) {
            WORD wVersionRequested = MAKEWORD(2, 2);
            WSADATA wsaData;

            if (WSAStartup(wVersionRequested, &wsaData) != 0) {
                return -1;
            }

            addr->sin6_family = AF_INET6;
            int addr_size = sizeof(*addr);
            char* str = new char[ip.size() + 1];
            strcpy(str, ip.c_str());  // NOLINT
            int err = WSAStringToAddress(str, AF_INET6, nullptr, reinterpret_cast<LPSOCKADDR>(addr), &addr_size);
            delete[] str;

            WSACleanup();

            return err == 0 ? 1 : -1;
        }
#else
        static std::pair<bool, int> create_socket(int family, int protocol) {
            int socket_fd = socket(family, protocol, 0);
            return {socket_fd >= 0, socket_fd};
        }

        static void close_socket(int socket_fd) {
            close(socket_fd);
        }

        // returns > 0 on success
        static unsigned int if_nametoindex(const std::string if_name) {
            return ::if_nametoindex(if_name.c_str());
        }

        static int inet_pton(const std::string& ip, sockaddr_in* addr) {
            return ::inet_pton(AF_INET, ip.c_str(), (void*)&addr->sin_addr.s_addr);  // NOLINT
        }

        static int inet_pton(const std::string& ip, sockaddr_in6* addr) {
            return ::inet_pton(AF_INET6, ip.c_str(), (void*)&addr->sin6_addr.s6_addr);  // NOLINT
        }
#endif
    }  // namespace detail

    bool Network::is_network_interface_connected(const NetworkInterface& network_interface) {
        if (!network_interface.is_up() || network_interface.get_ip() == "" || network_interface.get_ip() == "0.0.0.0"
#ifdef __APPLE__
            ||  network_interface.get_name().find("en") == string::npos
#endif
        ) {
            return false;
        }

        bool result = false;

        if (network_interface.get_inet_type() == NetworkInetType::INET_IPV4) {
            auto socket_creation_result = detail::create_socket(AF_INET, SOCK_STREAM);
            auto socket_fd = socket_creation_result.second;
            if (socket_creation_result.first) {
                struct sockaddr_in socket_struct;
                if (0 < detail::inet_pton(network_interface.get_ip(), &socket_struct)) {
                    socket_struct.sin_family = AF_INET;
                    socket_struct.sin_port = 0;
                    if (0 <= bind(socket_fd, (struct sockaddr *) &socket_struct, sizeof(socket_struct))) {
                        result = true;
                    }
                }
                detail::close_socket(socket_fd);
            }
        } else {
            auto socket_creation_result = detail::create_socket(AF_INET6, SOCK_STREAM);
            auto socket_fd = socket_creation_result.second;
            if (socket_creation_result.first) {
                struct sockaddr_in6 socket_struct;
                if (0 < detail::inet_pton(network_interface.get_ip(), &socket_struct)) {
                    socket_struct.sin6_family = AF_INET6;
                    socket_struct.sin6_port = 0;
                    socket_struct.sin6_scope_id = detail::if_nametoindex(network_interface.get_name());
                    if (0 < socket_struct.sin6_scope_id) {
                        if (0 <= bind(socket_fd, (struct sockaddr *) &socket_struct, sizeof(socket_struct))) {
                            result = true;
                        }
                    }
                }
                detail::close_socket(socket_fd);
            }
        }

        return result;
    }


#if defined(CONSOLE_SUPPORT)
#   include "support/network/console/NetworkConsole.h"
#elif defined(_WIN32)
    const std::vector<NetworkInterface> Network::get_network_interfaces() {
        std::vector<NetworkInterface> network_interfaces;
        if (_default_network_interface_set) {
            network_interfaces.push_back(_default_network_interface);
            return network_interfaces;
        }
        ULONG buflen = sizeof(IP_ADAPTER_INFO);
        IP_ADAPTER_INFO *pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(buflen));

        if (GetAdaptersInfo(pAdapterInfo, &buflen) == ERROR_BUFFER_OVERFLOW) {
            free(pAdapterInfo);
            pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(buflen));
        }

        if (GetAdaptersInfo(pAdapterInfo, &buflen) == NO_ERROR) {
            for (IP_ADAPTER_INFO *pAdapter = pAdapterInfo; pAdapter; pAdapter = pAdapter->Next) {
                string test1 = pAdapter->GatewayList.IpAddress.String;
                if (test1 != "0.0.0.0") {
                    NetworkInterface network_interface;
                    network_interface.set_up(true);
                    network_interface.set_loopback(false);

                    network_interface.set_ip(pAdapter->IpAddressList.IpAddress.String);
                    network_interface.set_inet_type(INET_IPV4);
                    network_interface.set_name(pAdapter->Description);

                    switch (pAdapter->Type) {
                        case MIB_IF_TYPE_ETHERNET:
                        case IF_TYPE_ISO88025_TOKENRING:
                        case MIB_IF_TYPE_PPP:
                        case MIB_IF_TYPE_LOOPBACK:
                        case MIB_IF_TYPE_SLIP:
                            network_interface.set_adapter_type(NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRED);
                            break;
                        case IF_TYPE_IEEE80211:
                            network_interface.set_adapter_type(NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRELESS);
                            break;
                        case MIB_IF_TYPE_OTHER:
                        default:
                            network_interface.set_adapter_type(NetworkAdapterType::NETWORK_ADAPTER_TYPE_UNKNOWN);
                    }  // switch (pAdapter->Type)

                    network_interface.set_is_connected(is_network_interface_connected(network_interface));

                    network_interfaces.push_back(network_interface);

                    // printf("%s (%s)\n", pAdapter->IpAddressList.IpAddress.String, pAdapter->Description);
                }
            }
        }

        if (pAdapterInfo) {
            free(pAdapterInfo);
        }

        return network_interfaces;
    }

    void Network::set_interface_for_socket(int socket, const std::string& interface_name) {
        // NOT APPLICABLE
        (void)socket;
        (void)interface_name;
    }
#elif defined(ANDROID)

    jobject fileDescriptor_N2J(JNIEnv* env, int file_descriptor);
    void bind_socket_to_network(JNIEnv* env, jobject j_socket_fd, const std::string& interface_name);

    boost::optional<NetworkInterface> get_wifi_network_interface(JNIEnv* env) {
        jmethodID j_get_wifi_util
                  = env->GetStaticMethodID(
                        jni::g_cls_ref_wifi_util_factory, "getWifiUtil",
                        "()Lcom/philips/lighting/hue/sdk/wrapper/utilities/WifiUtil;");

        JNILocalRef<jobject> j_wifi_util{env->CallStaticObjectMethod(jni::g_cls_ref_wifi_util_factory, j_get_wifi_util)};
        JNILocalRef<jclass> cls{env->GetObjectClass(j_wifi_util)};

        jmethodID j_get_is_enabled = env->GetMethodID(
                cls, "isEnabled",
                GET_METHOD_SIGNATURE("", GET_FULLY_QUALIFIED_CLASS_SIGNATURE(JNI_JAVA_CLASS_BOOLEAN)));

        JNILocalRef<jobject> j_is_enabled_obj{env->CallObjectMethod(j_wifi_util, j_get_is_enabled)};
        auto is_up = huesdk_jni_core::to<boost::optional<bool>>(j_is_enabled_obj);

        if (is_up._value == boost::none) {
            HUE_LOG << HUE_NETWORK << HUE_WARN << "Network (wifi): Could not get is_up for interface from Java, retrieve failed" << HUE_ENDL;
            return boost::none;
        }

        std::string name;
        jmethodID j_get_name
           = env->GetMethodID(
                        cls, "getName",
                        GET_METHOD_SIGNATURE("", GET_FULLY_QUALIFIED_CLASS_SIGNATURE(JNI_JAVA_CLASS_STRING)));

        JNILocalRef<jobject> j_name_obj{env->CallObjectMethod(j_wifi_util, j_get_name)};
        if (j_name_obj != nullptr) {
            name = support::jni::util::string_J2N(env, (jstring)j_name_obj.get());
        } else {
            HUE_LOG << HUE_NETWORK <<  HUE_WARN << "Network (wifi): Could not get name for interface from Java, retrieve failed" << HUE_ENDL;
            return boost::none;
        }

        /* Android wifi manager does not seem to provide a way to retrieve the IPv6 internet address.
           For now, we don't care, since we only support bridge discovery for IPv4 addresses */
        std::string ip_address;
        jmethodID j_get_ip
           = env->GetMethodID(cls, "getIpV4Address", GET_METHOD_SIGNATURE("", GET_FULLY_QUALIFIED_CLASS_SIGNATURE(JNI_JAVA_CLASS_STRING)));

        JNILocalRef<jobject> j_ip_obj{env->CallObjectMethod(j_wifi_util, j_get_ip)};
        if (j_ip_obj != nullptr) {
            ip_address = support::jni::util::string_J2N(env, (jstring)j_ip_obj.get());
        } else {
            HUE_LOG << HUE_NETWORK <<  HUE_WARN << "Network: Could not get ip for interface from Java, retrieve failed" << HUE_ENDL;
            return boost::none;
        }

        NetworkInetType network_internet_type = INET_IPV4;
        bool is_loop_back = false;

        jmethodID j_get_is_connected = env->GetMethodID(
                cls, "isWifiOnAndConnected",
                GET_METHOD_SIGNATURE("", GET_FULLY_QUALIFIED_CLASS_SIGNATURE(JNI_JAVA_CLASS_BOOLEAN)));

        JNILocalRef<jobject> j_is_connected_obj{env->CallObjectMethod(j_wifi_util, j_get_is_connected)};
        auto is_connected = huesdk_jni_core::to<boost::optional<bool>>(j_is_connected_obj);

        if (is_connected._value == boost::none) {
            HUE_LOG << HUE_NETWORK << HUE_WARN << "Network (wifi): Could not get is_connected for interface from Java, retrieve failed" << HUE_ENDL;
            return boost::none;
        }

        return NetworkInterface (
                ip_address, network_internet_type, name,
                *(is_up._value), is_loop_back,
                NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRELESS, *(is_connected._value));
    }

    const std::vector<NetworkInterface> Network::get_network_interfaces() {
        std::vector<NetworkInterface> network_interfaces;
        if (_default_network_interface_set) {
            network_interfaces.push_back(_default_network_interface);
            return network_interfaces;
        }
        HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "Network: get all network interfaces (JNI)" << HUE_ENDL;

        // collect network network_interface data from Java, using NetUtil helper class
        // NetUtil helps to protect against Exceptions, and avoids unnecessary c++ code

        HueJNIEnv env;

        // get NetUtil instance from NativeTools
        jmethodID j_get_netutil = env->GetStaticMethodID(jni::g_cls_ref_tools, "getNetUtil", "()Lcom/philips/lighting/hue/sdk/wrapper/utilities/NetUtil;");
        JNILocalRef<jobject> j_netutil{env->CallStaticObjectMethod(jni::g_cls_ref_tools, j_get_netutil)};
        JNILocalRef<jclass> j_netutil_cls{env->GetObjectClass(j_netutil)};
        jclass cls = j_netutil_cls;  // shorthand

        // first get number of available network interfaces, so we can start a loop
        jmethodID j_get_if_count = env->GetMethodID(cls, "getInterfaceCount", "()I");
        jint j_interface_count = env->CallIntMethod(j_netutil, j_get_if_count);

        // start loop
        for (int i = 0; i < static_cast<int>(j_interface_count); i++) {
            // collect ip, name, type, up, loopback
            // prepare local variables
            string name = "";
            string ip = "";
            NetworkInetType inet_type = INET_IPV4;

            // name
            jmethodID j_get_name = env->GetMethodID(cls, "getInterfaceName", "(I)[B");

            JNILocalRef<jobject> j_name_obj{env->CallObjectMethod(j_netutil, j_get_name, i)};
            if (j_name_obj != nullptr) {
                jbyteArray j_name = static_cast<jbyteArray>(j_name_obj.get());
                const char* n_name = jni::util::byteArrayToString(env, j_name);
                if (n_name != nullptr) {
                    name = string(n_name);
                    delete n_name;
                } else {
                    HUE_LOG << HUE_NETWORK <<  HUE_ERROR << "Network: Could not get name for network_interface from Java, translate failed" << HUE_ENDL;
                }
            } else {
                HUE_LOG << HUE_NETWORK <<  HUE_WARN << "Network: Could not get name for network_interface from Java, retrieve failed" << HUE_ENDL;
            }

            // ip & type, prefer ipv4, use ipv6 as backup
            jmethodID j_get_ipv4 = env->GetMethodID(cls, "getInterfaceIPv4Address", "(I)[B");
            JNILocalRef<jobject> j_ipv4_obj{ env->CallObjectMethod(j_netutil, j_get_ipv4, i)};
            if (j_ipv4_obj != nullptr) {
                // first get ipv4
                jbyteArray j_ip = static_cast<jbyteArray>(j_ipv4_obj.get());
                const char* n_ip = support::jni::util::byteArrayToString(env, j_ip);
                if (n_ip != nullptr) {
                    ip = string(n_ip);
                    // leave type at IPv4
                    delete n_ip;
                } else {
                    HUE_LOG << HUE_NETWORK <<  HUE_ERROR << "Network: Could not get ipv4 for network_interface from Java, translate failed" << HUE_ENDL;
                }
            } else {
                // ipv4 failed, get ipv6
                jmethodID j_get_ipv6 = env->GetMethodID(cls, "getInterfaceIPv6Address", "(I)[B");
                JNILocalRef<jobject> j_ipv6_obj{env->CallObjectMethod(j_netutil, j_get_ipv6, i)};
                if (j_ipv6_obj != nullptr) {
                    jbyteArray j_ip = static_cast<jbyteArray>(j_ipv6_obj.get());
                    const char* n_ip = support::jni::util::byteArrayToString(env, j_ip);
                    if (n_ip != nullptr) {
                        ip = string(n_ip);
                        // set type to IPv6
                        inet_type = INET_IPV6;
                        delete n_ip;
                    } else {
                        HUE_LOG << HUE_NETWORK <<  HUE_ERROR << "Network: Could not get ipv6 for network_interface from Java, translate failed" << HUE_ENDL;
                    }
                } else {
                    HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "Network: Could not get IP for network_interface from Java, retieve failed" << HUE_ENDL;
                }
            }

            // up
            jmethodID j_get_up = env->GetMethodID(cls, "getInterfaceUp", "(I)Z");
            jboolean j_up_obj = env->CallBooleanMethod(j_netutil, j_get_up, i);
            bool up = static_cast<bool>(j_up_obj);

            // loopback
            jmethodID j_get_loopback = env->GetMethodID(cls, "getInterfaceLoopback", "(I)Z");
            jboolean j_loopback_obj = env->CallBooleanMethod(j_netutil, j_get_loopback, i);
            bool loopback = static_cast<bool>(j_loopback_obj);

            // build NetworkInterface struct
            NetworkInterface network_interface;
            network_interface.set_ip(ip);
            network_interface.set_name(name);
            network_interface.set_inet_type(inet_type);
            network_interface.set_up(up);
            network_interface.set_loopback(loopback);
            network_interface.set_is_connected(false);
            network_interface.set_adapter_type(NetworkAdapterType::NETWORK_ADAPTER_TYPE_UNKNOWN);

            network_interfaces.push_back(network_interface);
        }

        /* The java.net.NetworkInterface, does not provide reliable results on android. When
           both 4G and Wifi are on, the Wifi interface is sometimes missing.
           Using android wifi manager, the connected wifi interface is added if not found by the NetworkInterface */
        auto wifi_interface = get_wifi_network_interface(env);

        if (wifi_interface != boost::none && wifi_interface->get_is_connected()) {
            auto it = std::find_if(
                   network_interfaces.begin(),
                   network_interfaces.end(),
                   [ip = wifi_interface->get_ip()](
                      const NetworkInterface& nw_if) {return nw_if.get_ip() == ip;});

            if (it == network_interfaces.end()) {
                network_interfaces.emplace_back(*wifi_interface);
            } else {
                it->set_is_connected(true);
                it->set_adapter_type(NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRELESS);
            }
        }

        // return vector
        return network_interfaces;
    }

    void Network::set_interface_for_socket(int socket, const std::string& interface_name) {
        HueJNIEnv env;
        JNILocalRef<jobject> j_socket_fd{fileDescriptor_N2J(env, socket)};

        if (j_socket_fd != nullptr) {
            bind_socket_to_network(env, j_socket_fd, interface_name);
        }
    }

    jobject fileDescriptor_N2J(JNIEnv* env, int file_descriptor) {
        JNILocalRef<jclass> file_descriptor_class{env->FindClass("java/io/FileDescriptor")};
        if (file_descriptor_class == NULL) {
            return nullptr;
        }

        // construct a new FileDescriptor
        jmethodID file_descriptor_constructor = env->GetMethodID(file_descriptor_class, "<init>", "()V");
        if (file_descriptor_constructor == NULL) {
            return nullptr;
        }
        jobject j_file_descriptor = env->NewObject(file_descriptor_class, file_descriptor_constructor);

        jmethodID j_method = env->GetMethodID(file_descriptor_class, "setInt$", "(I)V");
        if (j_method != nullptr) {
            env->CallVoidMethod(j_file_descriptor, j_method, file_descriptor);
            // clear possible exception, that might occur in future versions of Android, where this call is no longer allowed
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                HUE_LOG << HUE_NETWORK << HUE_ERROR << "Network: Could set file descriptor" << HUE_ENDL;
                return nullptr;
            }
        } else {
            HUE_LOG << HUE_NETWORK << HUE_ERROR << "Network: Could not get file descriptor setter method" << HUE_ENDL;
            return nullptr;
        }

        return j_file_descriptor;
    }

    void bind_socket_to_network(JNIEnv* env, jobject j_socket_fd, const std::string& interface_name) {
        JNILocalRef<jobject> j_interface_name{support::jni::util::string_N2J(env, interface_name.c_str())};

        jmethodID j_get_wifi_util
            = env->GetStaticMethodID(
                jni::g_cls_ref_wifi_util_factory, "getWifiUtil",
                "()Lcom/philips/lighting/hue/sdk/wrapper/utilities/WifiUtil;");

        JNILocalRef<jobject> j_wifi_util{env->CallStaticObjectMethod(jni::g_cls_ref_wifi_util_factory, j_get_wifi_util)};

        JNILocalRef<jclass> cls{env->GetObjectClass(j_wifi_util)};

        jmethodID j_method = env->GetMethodID(
            cls, "networkBindSocket", "(Ljava/io/FileDescriptor;Ljava/lang/String;)V");

        env->CallVoidMethod(j_wifi_util, j_method, j_socket_fd, j_interface_name.get());
    }

#else
    const std::vector<NetworkInterface> Network::get_network_interfaces() {
        std::vector<NetworkInterface> network_interfaces;

        HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "Network: get all network interfaces" << HUE_ENDL;
        if (_default_network_interface_set) {
            network_interfaces.push_back(_default_network_interface);
            return network_interfaces;
        }

        ifaddrs *if_addr    = nullptr;
        ifaddrs *if_addr_it = nullptr;
        // Get all network interfaces
        int result = getifaddrs(&if_addr);

        if (result != -1) {
            HUE_LOG << HUE_NETWORK << HUE_DEBUG << "Network: network interfaces retrieved; check for results" << HUE_ENDL;

            for (if_addr_it = if_addr; if_addr_it != nullptr; if_addr_it = if_addr_it->ifa_next) {
                HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "Network: network_interface found, name: " << if_addr_it->ifa_name << HUE_ENDL;

                // Get inet type
                NetworkInetType inet_type = if_addr_it->ifa_addr->sa_family == AF_INET ? INET_IPV4 : INET_IPV6;

                string ip;
                string netmask;
                // Resolve ip address
                switch (inet_type) {
                    case INET_IPV4: {
                        char ip4[INET_ADDRSTRLEN];
                        char nm[INET_ADDRSTRLEN];

                        in_addr addr4 = reinterpret_cast<sockaddr_in*>(if_addr_it->ifa_addr)->sin_addr;
                        in_addr addr_nm = reinterpret_cast<sockaddr_in*>(if_addr_it->ifa_netmask)->sin_addr;
                        // Convert to ipv4 string
                        inet_ntop(AF_INET, &addr4, ip4, INET_ADDRSTRLEN);
                        inet_ntop(AF_INET, &addr_nm, nm, INET_ADDRSTRLEN);

                        ip = string(ip4);
                        netmask = string(nm);
                        break;
                    }

                    case INET_IPV6: {
                        char ip6[INET6_ADDRSTRLEN];
                        char nm[INET6_ADDRSTRLEN];

                        in6_addr addr6 = reinterpret_cast<sockaddr_in6*>(if_addr_it->ifa_addr)->sin6_addr;
                        in6_addr addr_nm = reinterpret_cast<sockaddr_in6*>(if_addr_it->ifa_addr)->sin6_addr;
                        // Convert to ipv6 string
                        inet_ntop(AF_INET6, &addr6, ip6, INET6_ADDRSTRLEN);
                        inet_ntop(AF_INET6, &addr_nm, nm, INET6_ADDRSTRLEN);

                        ip = string(ip6);
                        netmask = string(nm);
                        break;
                    }
                }

                HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "Network: network network_interface found, ip: " << ip << ", name: " << if_addr_it->ifa_name << HUE_ENDL;

                // build NetworkInterface struct
                NetworkInterface network_interface;
                network_interface.set_ip(ip);
                network_interface.set_netmask(netmask);
                network_interface.set_name(string(if_addr_it->ifa_name));
                network_interface.set_inet_type(inet_type);
                network_interface.set_up(if_addr_it->ifa_flags & IFF_UP);
                network_interface.set_loopback(if_addr_it->ifa_flags & IFF_LOOPBACK);
                network_interface.set_adapter_type(get_adapter_type(network_interface));
                network_interface.set_is_connected(is_network_interface_connected(network_interface));

                network_interfaces.push_back(network_interface);
            }

            freeifaddrs(if_addr);
        } else {
            HUE_LOG << HUE_NETWORK <<  HUE_ERROR << "Network: error retrieving network interfaces" << HUE_ENDL;
        }

        return network_interfaces;
    }

    void Network::set_interface_for_socket(int socket, const std::string& interface_name) {
        // NOT APPLICABLE
        (void)socket;
        (void)interface_name;
    }
#endif

}  // namespace support
