#include <unordered_map>
#include <vector>
#include <tuple>
#include <bitset>
#include <memory>

// Assume each component type has a unique ID
using ComponentTypeID = std::uint32_t;
constexpr std::size_t MaxComponents = 64;

using ArchetypeKey = std::bitset<MaxComponents>;

class Entity {
public:
    std::uint32_t id;
    ArchetypeKey archetypeKey;
};

template <typename... Components>
class ArchetypeChunk {
public:
    using ComponentTuple = std::tuple<std::vector<Components>...>;

    void addEntity(Entity entity, Components... components) {
        entityIDs.push_back(entity.id);
        std::get<std::vector<Components>>(data).emplace_back(components...);
    }

    void removeEntity(std::size_t index) {
        entityIDs.erase(entityIDs.begin() + index);
        // Also remove components associated with this entity
        removeComponents<Components...>(index);
    }

    const std::vector<std::uint32_t>& getEntityIDs() const { return entityIDs; }

    template <typename Component>
    std::vector<Component>& getComponentData() {
        return std::get<std::vector<Component>>(data);
    }

private:
    std::vector<std::uint32_t> entityIDs;
    ComponentTuple data;

    template <typename First, typename... Rest>
    void removeComponents(std::size_t index) {
        std::get<std::vector<First>>(data).erase(std::get<std::vector<First>>(data).begin() + index);
        if constexpr (sizeof...(Rest) > 0) {
            removeComponents<Rest...>(index);
        }
    }
};

class ArchetypeManager {
public:
    template <typename... Components>
    ArchetypeChunk<Components...>& getOrCreateArchetypeChunk() {
        ArchetypeKey key = createArchetypeKey<Components...>();
        auto& chunk = archetypes[key];
        if (!chunk) {
            chunk = std::make_unique<ArchetypeChunk<Components...>>();
        }
        return *dynamic_cast<ArchetypeChunk<Components...>*>(chunk.get());
    }

private:
    std::unordered_map<ArchetypeKey, std::unique_ptr<void>> archetypes;

    template <typename... Components>
    ArchetypeKey createArchetypeKey() {
        ArchetypeKey key;
        ((key.set(ComponentTypeID(typeid(Components).hash_code()))), ...);
        return key;
    }
};
