#pragma once
#include <Entities/Player/PlayerStateMachine.h>
#include <Entities/Player/Player.h>

#include <Tabby.h>

namespace App {

class PlayerState {
    PlayerState(Player player, PlayerStateMachine stateMachine, PlayerData playerData, string animName, bool playOnInit)
    {
        this->player = player;
        this->stateMachine = stateMachine;
        this->playerData = playerData;
        this->animName = animName;
        _playOnInit = playOnInit;
    }

    PlayerState(Player player, PlayerStateMachine stateMachine, PlayerData playerData, string animName)
    {
        this.player = player;
        this.stateMachine = stateMachine;
        this.playerData = playerData;
        this.animName = animName;
    }

    virtual void Enter()
    {
        DoChecks(0.16d);
        startTime = Time.GetUnixTimeFromSystem();
        isAnimationFinished = false;
        isExitingState = false;
        if (_playOnInit)
            player.Anim.Play(animName);
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

    virtual void OnBodyEnter(Node node)
    {
    }

    virtual void OnBodyExit(Node node)
    {
    }

    virtual void AnimationTrigger() { }

    virtual void AnimationFinishTrigger() { isAnimationFinished = true; }

protected:
    Tabby::Shared<PlayerStateMachine> stateMachine;
    Tabby::Shared<PlayerData> playerData;
    Tabby::Shared<Player> player;

    bool isAnimationFinished;
    bool isExitingState;

private:
    double startTime;
    std::string animName;

    // This is for states with multiple animations. User can manually set animation in code and not get "animation doesn't exist" error in editor.
    bool _playOnInit = true;
};

}
