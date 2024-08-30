#pragma once
#include <Entities/Player/PlayerData.h>
#include <Entities/Player/Player.h>

namespace App {

class PlayerStateMachine;

class PlayerState {
public:
    PlayerState(Player player, PlayerStateMachine* stateMachine, PlayerData playerData, std::string animName, bool playOnInit);

    PlayerState(Player player, PlayerStateMachine* stateMachine, PlayerData playerData, std::string animName);

    virtual void Enter()
    {
        DoChecks(0.16f);
        startTime = Tabby::Time::GetTime();
        isAnimationFinished = false;
        isExitingState = false;
        // if (_playOnInit)
        //     player.Anim.Play(animName);
    }

    virtual void Exit()
    {
        isExitingState = true;
    }

    virtual void LogicUpdate(double delta)
    {
        DoChecks(delta);
    }

    virtual void PhysicsUpdate(double delta)
    {
    }

    virtual void DoChecks(double delta)
    {
    }

    virtual void OnBodyEnter(Tabby::Collision a)
    {
    }

    virtual void OnBodyExit(Tabby::Collision a)
    {
    }

    virtual void AnimationTrigger() { }

    virtual void AnimationFinishTrigger() { isAnimationFinished = true; }

protected:
    PlayerStateMachine* stateMachine;
    PlayerData playerData;
    Player player;

    bool isAnimationFinished;
    bool isExitingState;

private:
    double startTime;
    std::string animName;

    bool playOnInit = true;

    friend class PlayerStateMachine;
};

}
