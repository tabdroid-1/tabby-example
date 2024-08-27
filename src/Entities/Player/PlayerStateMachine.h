#pragma once

#include <Tabby.h>

namespace App {

class PlayerStateMachine {
public:
    Tabby::Shared<PlayerState> CurrentState;

    void Initialize(PlayerState startingState)
    {
        CurrentState = startingState;
        CurrentState->Enter();
    }

    void ChangeState(PlayerState newState)
    {
        CurrentState->Exit();
        CurrentState = newState;
        GD.Print("Player State: " + newState.animName + "State Start Time: " + CurrentState.startTime);
        CurrentState->Enter();
    }

    void CallOnBodyEnter(Tabby::Collision a)
    {
        CurrentState->OnBodyEnter(a);
    }

    void CallOnBodyExit(Tabby::Collision a)
    {
        CurrentState->OnBodyExit(a);
    }
};

}
