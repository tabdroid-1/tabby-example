#pragma once

#include <Tabby.h>

namespace App {

struct PlayerInputData {
    Tabby::Vector2 RawMovementInput;
    bool JumpInput;
    bool CrouchInput;
};
}
