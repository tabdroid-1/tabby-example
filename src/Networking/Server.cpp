#include <Tabby.h>
#include <Networking/Server.h>
#include <Networking/Packets.h>
#include <Entities/Player/Player.h>

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

namespace App {
const uint16_t DEFAULT_SERVER_PORT = 27020;
SteamNetworkingMicroseconds g_serverLogTimeZero;

static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
    SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_serverLogTimeZero;
    TB_TRACE("{} - {}", time * 1e-6, pszMsg);
    if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
        TB_CORE_VERIFY(false);
    }
}

Server::Server()
{
    TB_CORE_VERIFY_TAGGED(!s_Instance, "Server instance already exists!");
    s_Instance = this;
    m_ShouldThreadClose = false;

    bool bClient = false;
    SteamNetworkingIPAddr addrServer;
    addrServer.Clear();
    addrServer.m_port = DEFAULT_SERVER_PORT;

    {

        SteamDatagramErrMsg errMsg;
        if (!GameNetworkingSockets_Init(nullptr, errMsg))
            TB_CRITICAL("GameNetworkingSockets_Init failed.  {}", errMsg);

        g_serverLogTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

        SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
    }

    s_Instance->m_Interface = SteamNetworkingSockets();

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
    s_Instance->m_ListenSock = s_Instance->m_Interface->CreateListenSocketIP(addrServer, 1, &opt);
    TB_CORE_VERIFY_TAGGED(s_Instance->m_ListenSock != k_HSteamListenSocket_Invalid, "Failed to create listen socket.");

    s_Instance->m_PollGroup = s_Instance->m_Interface->CreatePollGroup();
    TB_CORE_VERIFY_TAGGED(s_Instance->m_PollGroup != k_HSteamNetPollGroup_Invalid, "Failed to listen on port {}.", DEFAULT_SERVER_PORT);

    SteamNetworkingIPAddr actualAddress;
    if (m_Interface->GetListenSocketAddress(m_ListenSock, &actualAddress)) {
        char ipStr[SteamNetworkingIPAddr::k_cchMaxString];
        actualAddress.ToString(ipStr, sizeof(ipStr), true);
        TB_INFO("Server is listening on ", ipStr);
    } else {
        TB_INFO("Failed to get listen socket address.");
    }
    m_PollingThread = std::thread(&Server::ServerPollingThread, this);
}

Server::~Server()
{
    m_ShouldThreadClose = true;
    if (m_PollingThread.joinable()) {
        m_PollingThread.join();
    }
}

void Server::Init()
{
    TB_PROFILE_SCOPE_NAME("Tabby::AudioEngine::Init");

    if (!s_Instance)
        s_Instance = new Server();
}

void Server::Shutdown()
{
    TB_PROFILE_SCOPE_NAME("Tabby::AudioEngine::Init");

    if (s_Instance)
        delete s_Instance;
}

void Server::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
    std::string message;

    switch (pInfo->m_info.m_eState) {
    case k_ESteamNetworkingConnectionState_None:
        break;

    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
        m_Interface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
        message = m_Clients[pInfo->m_hConn].name + " has left the server.";
        SendStringToAllClients(message, pInfo->m_hConn);

        Tabby::World::DestroyEntityWithChildren(Tabby::World::GetEntityByUUID(m_Clients[pInfo->m_hConn].wordId));
        m_Clients.erase(pInfo->m_hConn);
        break;
    }

    case k_ESteamNetworkingConnectionState_Connecting: {

        TB_CORE_VERIFY_TAGGED(m_Clients.find(pInfo->m_hConn) == m_Clients.end(), "Already contected client is connecting!");
        TB_INFO("Connection request from {}", pInfo->m_info.m_szConnectionDescription);

        if (m_Interface->AcceptConnection(pInfo->m_hConn) != k_EResultOK) {
            m_Interface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
            TB_INFO("Can't accept connection.  (It was already closed?)");
            break;
        }

        if (!m_Interface->SetConnectionPollGroup(pInfo->m_hConn, m_PollGroup)) {
            m_Interface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
            TB_INFO("Failed to set poll group?");
            break;
        }

        std::string nick = "UnknownWarior" + std::to_string(10000 + (rand() % 100000));

        // Send them a welcome message
        // sprintf(temp, "Welcome, stranger.  Thou art known to us for now as '%s'; upon thine command '/nick' we shall know thee otherwise.", nick);

        message = "Your name is " + nick;
        SendStringToClient(pInfo->m_hConn, message);

        // Also send them a list of everybody who is already connected
        if (m_Clients.empty()) {
            SendStringToClient(pInfo->m_hConn, "No players online...");
        } else {
            message = std::to_string(m_Clients.size()) + " player(s) online";
            SendStringToClient(pInfo->m_hConn, message);
            for (auto& c : m_Clients)
                SendStringToClient(pInfo->m_hConn, "    - " + c.second.name);
        }

        // Let everybody else know who they are for now
        message = nick + " has joined the server.";
        SendStringToAllClients(message, pInfo->m_hConn);

        // Add them to the client list, using std::map wacky syntax
        m_Clients[pInfo->m_hConn];
        SetClientNick(pInfo->m_hConn, nick);

        // Create Player
        auto id = Player::Spawn(nick);
        m_Clients[pInfo->m_hConn].wordId = id;

        break;
    }

    case k_ESteamNetworkingConnectionState_Connected:
        TB_INFO("{} connected", m_Clients[pInfo->m_hConn].name);
        break;

    default:
        break;
    }
}

void Server::ProcessMessage()
{
    ISteamNetworkingMessage* pIncomingMsg[16];
    int numMsgs = m_Interface->ReceiveMessagesOnPollGroup(m_PollGroup, pIncomingMsg, 16);

    for (int i = 0; i < numMsgs; i++) {
        if (numMsgs < 0 || !pIncomingMsg[i]) {
            if (pIncomingMsg[i])
                pIncomingMsg[i]->Release();
            return;
        }
        if (pIncomingMsg[i]->m_cbSize < sizeof(MessageHeader)) {
            std::cerr << "Received message too small to contain header!" << std::endl;
            return;
        }

        const MessageHeader* header = reinterpret_cast<const MessageHeader*>(pIncomingMsg[i]->m_pData);

        // Process the message based on its type
        switch (header->messageType) {
        case MESSAGE_TYPE_INPUT: {
            if (pIncomingMsg[i]->m_cbSize < sizeof(MessageHeader) + sizeof(PlayerInputState)) {
                std::cerr << "Received INPUT message with incorrect size!" << std::endl;
                return;
            }

            const PlayerInputState* inputState = reinterpret_cast<const PlayerInputState*>(static_cast<const char*>(pIncomingMsg[i]->m_pData) + sizeof(MessageHeader));

            Player::Update(Tabby::World::GetEntityByUUID(m_Clients[pIncomingMsg[i]->GetConnection()].wordId), inputState);
            break;
        }
        case MESSAGE_TYPE_CHAT: {
            if (pIncomingMsg[i]->m_cbSize < sizeof(MessageHeader) + sizeof(ChatMessage)) {
                std::cerr << "Received CHAT message with incorrect size!" << std::endl;
                return;
            }

            const ChatMessage* chatMsg = reinterpret_cast<const ChatMessage*>(static_cast<const char*>(pIncomingMsg[i]->m_pData) + sizeof(MessageHeader));
            TB_INFO("{}", chatMsg->message);
            break;
        }
        // Handle other message types similarly
        default:
            std::cerr << "Received unknown message type!" << std::endl;
            break;
        }
    }
}

void Server::SendStringToClient(HSteamNetConnection conn, const std::string& str)
{

    // Create and populate the message
    MessageHeader header;
    header.messageType = MessageType::MESSAGE_TYPE_CHAT;

    ChatMessage chatMsg;
    std::strncpy(chatMsg.message, str.c_str(), 256);
    chatMsg.message[256] = '\0';

    // Combine header and inputState into one message
    char buffer[sizeof(MessageHeader) + sizeof(ChatMessage)];
    memcpy(buffer, &header, sizeof(MessageHeader));
    memcpy(buffer + sizeof(MessageHeader), &chatMsg, sizeof(ChatMessage));

    // Send the message to the server
    m_Interface->SendMessageToConnection(conn, buffer, sizeof(buffer), k_nSteamNetworkingSend_Reliable, nullptr);
}

}
