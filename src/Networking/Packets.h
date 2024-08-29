#pragma once

#include <Tabby.h>

namespace App {

enum MessageType {
    MESSAGE_TYPE_INPUT = 1,
    MESSAGE_TYPE_CHAT = 2,
    MESSAGE_TYPE_PING = 3,
};

struct MessageHeader {
    uint16_t messageType;
};

// Server sends
struct PlayerJoinPacket {
    Tabby::UUID id;
    std::string name;
    Tabby::Vector2 position;
    float rotation;
};

// Server sends
struct PlayerStatePacket {
    Tabby::UUID id;
    Tabby::Vector2 velocity;
    Tabby::Vector2 position;
    float angularVelocity;
    float rotation;
};

// Client and server sends
struct ChatMessage {
    char message[256];
};

// Client sends
struct PlayerInputState {
    Tabby::Vector2 move;
    bool jump;
    uint32_t sequenceNumber; // to track input order
};
}
