#pragma once
#include "physics/physics.hpp"
#include <SFML/Graphics/Color.hpp>

typedef void (*func)(PhysicSolver &, sf::Color *, int);

inline void single(PhysicSolver &s, sf::Color *colors, int count) {
    for (int i = 0; i < count; i++) {
        const auto id = s.createObject({2.0f, 10.0f + 1.1f * i});
        s.objects[id].last_position.x -= 0.2f;
        s.objects[id].color = colors[id];
    }
}

inline void dual(PhysicSolver &s, sf::Color *colors, int count) {
    for (int i = 0; i < count; i++) {
        auto id = s.createObject({3.0f - 0.1f * i, s.world_size.y - 2.0f - 1.1f * i});
        s.objects[id].last_position.x -= 0.2f;
        s.objects[id].last_position.y -= 0.01f;
        s.objects[id].color = colors[id];
        
        id = s.createObject({s.world_size.x - 3.0f + 0.1f * i, 2.0f + 1.1f * i});
        s.objects[id].last_position.x += 0.2f;
        s.objects[id].last_position.y += 0.01f;
        s.objects[id].color = colors[id];
    }
}

inline void dual_rev(PhysicSolver &s, sf::Color *colors, int count) {
    for (int i = 0; i < count; i++) {
        auto id = s.createObject({3.0f - 0.1f * i, 2.0f + 1.1f * i});
        s.objects[id].last_position.x -= 0.2f;
        s.objects[id].last_position.y -= 0.01f;
        s.objects[id].color = colors[id];
        
        id = s.createObject({s.world_size.x - 3.0f + 0.1f * i, s.world_size.y - 2.0f - 1.1f * i});
        s.objects[id].last_position.x += 0.2f;
        s.objects[id].last_position.y += 0.01f;
        s.objects[id].color = colors[id];
    }
}

inline void quadruple(PhysicSolver &s, sf::Color *colors, int count) {
    for (int i = 0; i < count; i++) {
        auto id = s.createObject({2.0f + 1.1f * i, 2.0f});
        s.objects[id].last_position.y -= 0.2f;
        s.objects[id].color = colors[id];

        id = s.createObject({s.world_size.x - 2.0f, 2.0f + 1.1f * i});
        s.objects[id].last_position.x += 0.2f;
        s.objects[id].color = colors[id];

        id = s.createObject({s.world_size.x - 2.0f - 1.1f * i, s.world_size.y - 2.0f});
        s.objects[id].last_position.y += 0.2f;
        s.objects[id].color = colors[id];

        id = s.createObject({2.0f, s.world_size.y - 2.0f - 1.1f * i});
        s.objects[id].last_position.x -= 0.2f;
        s.objects[id].color = colors[id];
    }
}

inline void quadruple_rev(PhysicSolver &s, sf::Color *colors, int count) {
    for (int i = 0; i < count; i++) {
        auto id = s.createObject({2.0f, 2.0f + 1.1f * i});
        s.objects[id].last_position.x -= 0.2f;
        s.objects[id].color = colors[id];

        id = s.createObject({s.world_size.x - 2.0f - 1.1f * i, 2.0f});
        s.objects[id].last_position.y -= 0.2f;
        s.objects[id].color = colors[id];

        id = s.createObject({s.world_size.x - 2.0f, s.world_size.y - 2.0f - 1.1f * i});
        s.objects[id].last_position.x += 0.2f;
        s.objects[id].color = colors[id];

        id = s.createObject({2.0f + 1.1f * i, s.world_size.y - 2.0f});
        s.objects[id].last_position.y += 0.2f;
        s.objects[id].color = colors[id];
    }
}
