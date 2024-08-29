#pragma once

#include <Tabby.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <Tabby.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

namespace App {

class Server {
public:
    struct Client {
        Tabby::UUID wordId;
        std::string name;
    };

    ~Server();
    static void Init();
    static void Shutdown();

    void ServerPollingThread()
    {
        while (!m_ShouldThreadClose) {
            ProcessMessage();
            m_Interface->RunCallbacks();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void SendStringToClient(HSteamNetConnection conn, const std::string& str);

    void SendStringToAllClients(std::string str, HSteamNetConnection except = k_HSteamNetConnection_Invalid)
    {
        for (auto& c : m_Clients) {
            if (c.first != except)
                SendStringToClient(c.first, str);
        }
    }

    void SetClientNick(HSteamNetConnection hConn, const std::string& nick)
    {
        m_Clients[hConn].name = nick;
        m_Interface->SetConnectionName(hConn, nick.c_str());
    }

    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
    {
        s_Instance->OnConnectionStatusChanged(pInfo);
    }

private:
    Server();
    void ProcessMessage();
    void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

private:
    std::map<HSteamNetConnection, Client> m_Clients;
    HSteamListenSocket m_ListenSock;
    HSteamNetPollGroup m_PollGroup;
    ISteamNetworkingSockets* m_Interface;
    std::mutex m_ServerLock;
    std::atomic_bool m_ShouldThreadClose;
    std::thread m_PollingThread;
    inline static Server* s_Instance;
};

}
