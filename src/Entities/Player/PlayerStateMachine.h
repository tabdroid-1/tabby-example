#pragma once

#include <Tabby.h>

namespace App {

class PlayerState;

class PlayerStateMachine {
public:
    PlayerStateMachine();
    void Initialize(PlayerState* startingState);
    void ChangeState(PlayerState* newState);

    void CallOnBodyEnter(Tabby::Collision a);
    void CallOnBodyExit(Tabby::Collision a);

private:
    PlayerState* m_CurrentState;
};

}
