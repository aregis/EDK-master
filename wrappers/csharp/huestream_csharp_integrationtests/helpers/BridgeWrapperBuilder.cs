using System;

namespace huestream_tests
{
    public class BridgeWrapperBuilder
    {
        /**
         * Constructs the Bridge Wrapper. If some params are not passed, returns null
         * @return IBridgeWrapper instance, if all parameters have been provided; null otherwise.
         */
        public IBridgeWrapper Build()
        {
            if (_userName.Length == 0 || _clientKey.Length == 0 || _ip4Address.Length == 0 || _bridgeId.Length == 0 || _tcpPort.Length == 0)
            {
                return null;
            }

            return new BridgeWrapperImpl(_userName, _clientKey, _ip4Address, _bridgeId, _tcpPort);
        }

        public BridgeWrapperBuilder WithUserName(String userName)
        {
            _userName = userName;
            return this;
        }

        public BridgeWrapperBuilder WithClientKey(String clientKey)
        {
            _clientKey = clientKey;
            return this;
        }

        public BridgeWrapperBuilder WithIPv4Address(String address)
        {
            _ip4Address = address;
            return this;
        }

        public BridgeWrapperBuilder WithBridgeId(String bridgeId)
        {
            _bridgeId = bridgeId;
            return this;
        }

        public BridgeWrapperBuilder WithTcpPort(String tcpPort)
        {
            _tcpPort = tcpPort;
            return this;
        }

        private String _userName = "";
        private String _clientKey = "";
        private String _ip4Address = "";
        private String _bridgeId = "";
        private String _tcpPort = "";
    }
}