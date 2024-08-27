#include <Tabby.h>
#include <Networking/Client.h>
#include <Networking/Packets.h>

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

namespace App {
const uint16_t DEFAULT_SERVER_PORT = 27020;
SteamNetworkingMicroseconds g_clientLogTimeZero;

static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
    SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_clientLogTimeZero;
    TB_INFO("{} - {}", time * 1e-6, pszMsg);
    if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
        TB_CORE_VERIFY(false);
    }
}

Client::Client()
{
    TB_PROFILE_SCOPE_NAME("App::Client::Constructor");

    TB_CORE_VERIFY_TAGGED(!s_Instance, "Client instance already exists!");
    s_Instance = this;
    m_ShouldThreadClose = false;

    SteamNetworkingIPAddr addrClient;
    addrClient.Clear();
    addrClient.ParseString("127.0.0.1");
    addrClient.m_port = DEFAULT_SERVER_PORT;

    {

        SteamDatagramErrMsg errMsg;
        if (!GameNetworkingSockets_Init(nullptr, errMsg))
            TB_CRITICAL("GameNetworkingSockets_Init failed.  {}", errMsg);

        g_clientLogTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

        SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
    }

    s_Instance->m_Interface = SteamNetworkingSockets();

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
    m_Connection = m_Interface->ConnectByIPAddress(addrClient, 1, &opt);
    TB_CORE_VERIFY_TAGGED(m_Connection != k_HSteamNetConnection_Invalid, "Failed to create connection.");

    m_PollingThread = std::thread(&Client::ClientPollingThread, this);
}

Client::~Client()
{
    m_ShouldThreadClose = true;
    if (m_PollingThread.joinable()) {
        m_PollingThread.join();
    }
}

void Client::Init()
{
    TB_PROFILE_SCOPE_NAME("App::Client::Init");

    if (!s_Instance)
        s_Instance = new Client();
}

void Client::Shutdown()
{
    TB_PROFILE_SCOPE_NAME("App::Client::Shutdown");

    if (s_Instance)
        delete s_Instance;
}

void Client::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
    TB_PROFILE_SCOPE_NAME("App::Client::OnConnectionStatusChanged");

    switch (pInfo->m_info.m_eState) {
    case k_ESteamNetworkingConnectionState_None:
        break;

    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
        m_ShouldThreadClose = true;

        if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
            TB_WARN("Failed to connect.  {}", pInfo->m_info.m_szEndDebug);
        } else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
            TB_ERROR("Contact with the host.  {}", pInfo->m_info.m_szEndDebug);
        } else {
            TB_INFO("The host hath bidden us farewell. {}", pInfo->m_info.m_szEndDebug);
        }

        m_Interface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
        m_Connection = k_HSteamNetConnection_Invalid;
        break;
    }

    case k_ESteamNetworkingConnectionState_Connecting:
        TB_INFO("Connecting...");
        break;

    case k_ESteamNetworkingConnectionState_Connected:

        // SteamNetConnectionInfo_t info;
        // if (SteamNetworkingSockets()->GetConnectionInfo(pInfo->m_hConn, &info)) {
        //     char ipStr[SteamNetworkingIPAddr::k_cchMaxString];
        //     info.m_addrRemote.ToString(ipStr, sizeof(ipStr), true);
        //     TB_INFO("Connected to {}.", ipStr);
        // } else {
        //     TB_INFO("Failed to get connection info.");
        // }
        break;

    default:
        break;
    }
}

void Client::SendInput()
{
    // Create and populate the message
    MessageHeader header;
    header.messageType = MESSAGE_TYPE_INPUT;

    InputState inputState;
    inputState.sequenceNumber = ++m_CurrentInputSequenceNumber;

    if (Tabby::Input::GetKey(Tabby::Key::A))
        inputState.move.x++;

    if (Tabby::Input::GetKey(Tabby::Key::D))
        inputState.move.x--;

    if (Tabby::Input::GetKey(Tabby::Key::W))
        inputState.move.y++;

    if (Tabby::Input::GetKey(Tabby::Key::S))
        inputState.move.y--;

    // Combine header and inputState into one message
    char buffer[sizeof(MessageHeader) + sizeof(InputState)];
    memcpy(buffer, &header, sizeof(MessageHeader));
    memcpy(buffer + sizeof(MessageHeader), &inputState, sizeof(InputState));

    // Send the message to the server
    m_Interface->SendMessageToConnection(m_Connection, buffer, sizeof(buffer), k_nSteamNetworkingSend_Unreliable, nullptr);
}

}
