#pragma once

#include "dungeon.h"

#include <stdexcept>
#include <unordered_set>

Room* FindFinalRoom(Room* starting_room) {
    std::vector<std::string> keys;
    std::queue<Room*> q;
    std::unordered_set<Room*> used;
    std::unordered_set<Door*> locked_doors;
    q.push(starting_room);
    while (!q.empty()) {
        Room* from = q.front();
        q.pop();
        if (from->IsFinal()) {
            return from;
        }
        for (size_t key_ind = 0; key_ind < from->NumKeys(); ++key_ind) {
            std::string key = from->GetKey(key_ind);
            keys.push_back(key);
            for (auto it = locked_doors.begin(); it != locked_doors.end(); ++it) {
                if ((*it)->TryOpen(key) && used.find((*it)->GoThrough()) == used.end()) {
                    q.push((*it)->GoThrough());
                    locked_doors.erase(it);
                    break;
                }
            }
        }
        for (size_t door_ind = 0; door_ind < from->NumDoors(); ++door_ind) {
            Door* door = from->GetDoor(door_ind);
            if (door->IsOpen()) {
                if (used.find(door->GoThrough()) == used.end()) {
                    used.insert(door->GoThrough());
                    q.push(door->GoThrough());
                }
            } else {
                bool is_unlocked = false;
                for (auto key : keys) {
                    if (door->TryOpen(key)) {
                        is_unlocked = true;
                        if (used.find(door->GoThrough()) == used.end()) {
                            used.insert(door->GoThrough());
                            q.push(door->GoThrough());
                        }
                        break;
                    }
                }
                if (!is_unlocked) {
                    locked_doors.insert(door);
                }
            }
        }
    }
    return nullptr;
}
