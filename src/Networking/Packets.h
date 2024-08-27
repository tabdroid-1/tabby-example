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

struct PlayerStatePacket {
    Tabby::Vector2 velocity;
    Tabby::Vector2 position;
    float angularVelocity;
    float rotation;
};

struct InputState {
    Tabby::Vector2 move;
    bool jump;
    uint32_t sequenceNumber; // to track input order
};

struct ChatMessage {
    std::string message; // Example chat message structure
};
}
