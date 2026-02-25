#ifndef REDIS_CLONE_STORAGE_H
#define REDIS_CLONE_STORAGE_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <utility>

struct Value {
    std::string data;
    // Preparation for TTL (Time To Live in milliseconds)
    bool has_expiry = false;
    std::chrono::time_point<std::chrono::steady_clock> expires_at;

    Value() = default;
    Value(std::string d) : data(std::move(d)), has_expiry(false) {}
};

class Storage {
public:
    Storage() = default;

    // SET key value
    void set(const std::string& key, const std::string& value);

    // GET key. Returns {found, value}
    std::pair<bool, std::string> get(const std::string& key);

    // DEL key
    bool del(const std::string& key);

    // TODO: Add methods for TTL and eviction

private:
    std::unordered_map<std::string, Value> store_;
    mutable std::mutex mutex_; // Allows one writer or reader at a time
};

#endif // REDIS_CLONE_STORAGE_H
