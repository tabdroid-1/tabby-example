#include <Entities/Player/PlayerStateMachine.h>
#include <Entities/Player/PlayerState.h>

namespace App {

PlayerStateMachine::PlayerStateMachine()
{
}

void PlayerStateMachine::Initialize(PlayerState* startingState)
{
    m_CurrentState = startingState;
    m_CurrentState->Enter();
}

void PlayerStateMachine::ChangeState(PlayerState* newState)
{
    m_CurrentState->Exit();
    m_CurrentState = newState;
    TB_INFO("Player State: {} State Start Time: {}", newState->animName, m_CurrentState->startTime);
    m_CurrentState->Enter();
}

void PlayerStateMachine::CallOnBodyEnter(Tabby::Collision a)
{
    m_CurrentState->OnBodyEnter(a);
}

void PlayerStateMachine::CallOnBodyExit(Tabby::Collision a)
{
    m_CurrentState->OnBodyExit(a);
}

}
