#pragma once

#include <Networking/Packets.h>
#include <Components.h>
#include <Tabby.h>

namespace App {

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
        // DynamicEntity.AddComponent<App::PlayerComponent>();
        // auto& rb = DynamicEntity.AddComponent<Tabby::Rigidbody2DComponent>();
        // rb.Type = Tabby::Rigidbody2DComponent::BodyType::Dynamic;
        // rb.OnCollisionEnterCallback = [](Tabby::Collision a) {
        //     TB_INFO("Enter: {}", a.CollidedEntity.GetName());
        // };
        // rb.OnCollisionExitCallback = [](Tabby::Collision a) {
        //     TB_INFO("Exit: {}", a.CollidedEntity.GetName());
        // };
        // auto& bc = DynamicEntity.AddComponent<Tabby::BoxCollider2DComponent>();
        // bc.Size = { 2.0f, 0.5f };

        return DynamicEntity.GetUUID();
    }

    static void Update(Tabby::Entity entity, const PlayerInputState* state)
    {
        auto& tc = entity.GetComponent<Tabby::TransformComponent>();

        tc.position.x += state->move.x * 100.0f * Tabby::Time::GetDeltaTime();
        tc.position.y += state->move.y * 100.0f * Tabby::Time::GetDeltaTime();
    }
};

}
