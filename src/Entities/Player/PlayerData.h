#pragma once

#include <Tabby.h>

namespace App {

struct PlayerData {
    uint32_t whatIsGround;

    // General State
    float standingColliderHeight = 30.0f;
    float standingColliderRadius = 8.0f;
    Tabby::Vector2 standingColliderPosition = { 0.0f, -15.0f };
    // float gravity = ProjectSettings.GetSetting("physics/2d/default_gravity").AsSingle();
    // float gravityScale = 1;

    // Move State
    float maxWalkVelocity = 50;
    float maxRunVelocity = 350;
    float maxWalkAcceleration = 35;
    float maxWalkDecceleration = 40;
    float maxAcceleration = 200.0f;
    float maxDecceleration = 250.0f;
    float maxTurnSpeed = 80.0f;
    Tabby::Vector2 terminalVelocity;
    float friction = 50.0f;

    // Jump State
    float timeToJumpApex;
    float upwardMovementMultiplier = 1.0f;
    float downwardMovementMultiplier = 6.17f;
    int maxAirJumps = 0;
    // Max velocity that can be obtained when fallin in jump state
    float maxJumpFallVelocity;
    bool variablejumpHeight;
    float jumpCutOff;
    float coyoteTime = 0.15f;
    float jumpBuffer = 0.15f;
    float variableJumpTime = 0.55f;
    float jumpVelocity = 15.0f;
    float jumpSpeed;
    float jumpHeight = 7.3f;

    // Wall Jump State
    float wallJumpVelocity = 20;
    float wallJumpTime = 0.4f;
    Tabby::Vector2 wallJumpAngle = { 1, 2 };

    // In Air State
    float inAirVelocity = 50.0f;
    float maxAirAcceleration;
    float maxAirDeceleration;
    float maxAirTurnSpeed = 80.0f;
    float variableJumpHeightMultiplier = 0.5f;

    // Air Dive State
    float airDiveVelocity = 150.0f;
    float airDiveMaxDuration = 3.0f;

    // Bounce State
    float bounceVelocity = 150.0f;
    float bounceBuffer = 0.45f;
    float bounceTimeout = 0.45f;

    // Wall Slide State
    float wallSlideVelocity = 3.0f;

    // Wall Climb State
    float wallClimbVelocity = 3.0f;

    // Ledge Climb State
    Tabby::Vector2 startOffset;
    Tabby::Vector2 stopOffset;

    // Crouch State
    float crouchColliderHeight = 16.0f;
    float crouchColliderRadius = 8.0f;
    Tabby::Vector2 crouchColliderPosition = { 0.0f, -7.5f };
    float crouchMovementVelocity = 50.0f;

    // Ground Slide State
    float slideVelocity = 100.0f;
    float slideColliderHeight = 16.0f;
    float slideColliderRadius = 8.0f;
    Tabby::Vector2 slideColliderPosition = { 0.0f, -7.5f };

    // Wall Toss State
    // Y Velocity that will be set when tossed to a wall.
    float wallTossUpwardVelocity = 250.0f;
    float wallTossColliderHeight = 16.0f;
    float wallTossColliderRadius = 8.0f;
    Tabby::Vector2 wallTossColliderPosition = { 0.0f, -7.5f };
    // This is used to determine amount of times player will bounce by dividing this from players X velocity
    float velocityToBounce = 400.0f;
    // Minimum required velocity to enter toss state
    float velocityToToss = 600.0f;
};
}
