#include "Components.h"

template <>
void Tabby::World::OnComponentAdded<App::PlayerComponent>(Entity entity, App::PlayerComponent& component)
{
}

template <>
void Tabby::World::OnComponentAdded<App::SpawnpointComponent>(Entity entity, App::SpawnpointComponent& component)
{
}
