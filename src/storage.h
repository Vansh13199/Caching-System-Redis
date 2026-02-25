#ifndef REDIS_CLONE_STORAGE_H
#define REDIS_CLONE_STORAGE_H

#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <optional>
#include <chrono>

struct Value {
    std::string data;
    // Preparation for TTL (Time To Live in milliseconds)
    std::optional<std::chrono::time_point<std::chrono::steady_clock>> expires_at;

    Value() = default;
    Value(std::string d) : data(std::move(d)) {}
};

class Storage {
public:
    Storage() = default;

    // SET key value
    void set(const std::string& key, const std::string& value);

    // GET key
    std::optional<std::string> get(const std::string& key);

    // DEL key
    bool del(const std::string& key);

    // TODO: Add methods for TTL and eviction

private:
    std::unordered_map<std::string, Value> store_;
    mutable std::shared_mutex mutex_; // Allows multiple readers or one writer
};

#endif // REDIS_CLONE_STORAGE_H
