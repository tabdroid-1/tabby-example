#pragma once

#include <Tabby.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

namespace App {

class Client {
public:
    ~Client();
    static void Init();
    static void Shutdown();

    void ClientPollingThread()
    {
        TB_PROFILE_SET_THREAD_NAME("App::Client");
        while (!m_ShouldThreadClose) {
            TB_PROFILE_FRAME();
            TB_PROFILE_SCOPE_NAME("App::Client::OnConnectionStatusChanged");

            ProcessMessage();
            m_Interface->RunCallbacks();
            SendInput();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        m_Interface->CloseConnection(m_Connection, 0, nullptr, false);
        m_Connection = k_HSteamNetConnection_Invalid;
        GameNetworkingSockets_Kill();
    }

    void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
    {
        s_Instance->OnConnectionStatusChanged(pInfo);
    }

private:
    Client();

    void SendInput();
    void ProcessMessage();

private:
    uint32_t m_CurrentInputSequenceNumber;

    HSteamNetConnection m_Connection;
    ISteamNetworkingSockets* m_Interface;
    std::mutex m_ClientLock;
    std::atomic_bool m_ShouldThreadClose;
    std::thread m_PollingThread;
    inline static Client* s_Instance;
};

}
// namespace App {

// class Client {
// public:
//     ~Client();
//     static void Init();
//     static void Shutdown();
//
//     void ClientPollingThread()
//     {
//         TB_PROFILE_SET_THREAD_NAME("App::Client");
//         while (!m_ShouldThreadClose) {
//             TB_PROFILE_FRAME();
//             TB_PROFILE_SCOPE_NAME("App::Client::OnConnectionStatusChanged");
//
//             SendInput();
//             m_Interface->RunCallbacks();
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//         }
//
//         m_Interface->CloseConnection(m_Connection, 0, nullptr, false);
//         m_Connection = k_HSteamNetConnection_Invalid;
//         GameNetworkingSockets_Kill();
//     }
//
//     void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
//
//     static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
//     {
//         s_Instance->OnConnectionStatusChanged(pInfo);
//     }
//
// private:
//     Client();
//
//     void SendInput();
//     // void ProcessMessage();
//
//     // void ProcessChatMessage()
//     // {
//     //     ISteamNetworkingMessage* pIncomingMsg = nullptr;
//     //     int numMsgs = m_Interface->ReceiveMessagesOnConnection(m_Connection, &pIncomingMsg, 1);
//     //     if (numMsgs == 0)
//     //         return;
//     //     if (numMsgs < 0)
//     //         TB_ERROR("Error checking for messages");
//     //
//     //     // Just echo anything we get from the server
//     //     // fwrite(pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout);
//     //     // fputc('\n', stdout);
//     //
//     //     std::string message(reinterpret_cast<char*>(pIncomingMsg->m_pData), pIncomingMsg->m_cbSize);
//     //     TB_INFO("{}", message);
//     //
//     //     // We don't need this anymore.
//     //     pIncomingMsg->Release();
//     // }
//
// private:
//     uint32_t m_CurrentInputSequenceNumber;
//
//     HSteamNetConnection m_Connection;
//     ISteamNetworkingSockets* m_Interface;
//     std::mutex m_ClientLock;
//     std::atomic_bool m_ShouldThreadClose;
//     std::thread m_PollingThread;
//     inline static Client* s_Instance;
// };
//
// }
