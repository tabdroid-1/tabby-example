#pragma once

#include <Entities/Player/PlayerStateMachine.h>
#include <Components.h>
#include <Tabby.h>

namespace App {
struct PlayerComponent {
    PlayerStateMachine stateMachine;
    PlayerComponent() = default;
    PlayerComponent(const PlayerComponent&) = default;
};

class Player {
public:
    static Tabby::UUID Spawn(const std::string& name)
    {
        std::vector<Tabby::TransformComponent> spawnpoints;
        auto view = Tabby::World::GetAllEntitiesWith<Tabby::TransformComponent, App::SpawnpointComponent>();
        for (auto entity : view) {
            auto [tc, sc] = view.get<Tabby::TransformComponent, App::SpawnpointComponent>(entity);
            if (sc.entityName == "Player")
                spawnpoints.push_back(tc);
        }

        const auto spawnpoint = spawnpoints[Tabby::Random::Range(0, spawnpoints.size())];

        Tabby::Entity DynamicEntity = Tabby::World::CreateEntity("Player");

        auto& tr = DynamicEntity.GetComponent<Tabby::TransformComponent>();
        tr.position = spawnpoint.position;
        tr.rotation = spawnpoint.rotation;
        tr.scale = spawnpoint.scale;

        auto& sc = DynamicEntity.AddComponent<Tabby::SpriteRendererComponent>();
        sc.Texture = Tabby::AssetManager::LoadAssetSource("textures/Tabby.png");
        DynamicEntity.AddComponent<App::PlayerComponent>();
        auto& rb = DynamicEntity.AddComponent<Tabby::Rigidbody2DComponent>();
        rb.Type = Tabby::Rigidbody2DComponent::BodyType::Dynamic;
        rb.OnCollisionEnterCallback = [](Tabby::Collision a) {
            TB_INFO("Enter: {}", a.CollidedEntity.GetName());
        };
        rb.OnCollisionExitCallback = [](Tabby::Collision a) {
            TB_INFO("Exit: {}", a.CollidedEntity.GetName());
        };
        auto& cc = DynamicEntity.AddComponent<Tabby::CircleCollider2DComponent>();
        cc.Radius = 0.75f;

        return DynamicEntity.GetUUID();
    }

    static void Update(Tabby::Entity entity)
    {
        auto& tc = entity.GetComponent<Tabby::TransformComponent>();
    }
};

}
