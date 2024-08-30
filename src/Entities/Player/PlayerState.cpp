#include <Entities/Player/PlayerStateMachine.h>

#include <Entities/Player/PlayerState.h>

namespace App {

PlayerState::PlayerState(Player player, PlayerStateMachine* stateMachine, PlayerData playerData, std::string animName, bool playOnInit)
{
    this->player = player;
    this->stateMachine = stateMachine;
    this->playerData = playerData;
    this->animName = animName;
    this->playOnInit = playOnInit;
}

PlayerState::PlayerState(Player player, PlayerStateMachine* stateMachine, PlayerData playerData, std::string animName)
{
    this->player = player;
    this->stateMachine = stateMachine;
    this->playerData = playerData;
    this->animName = animName;
}

}
