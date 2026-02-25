#include "storage.h"

void Storage::set(const std::string& key, const std::string& value) {
    std::unique_lock<std::shared_mutex> lock(mutex_); // Exclusive lock for writing
    store_[key] = Value(value);
}

std::optional<std::string> Storage::get(const std::string& key) {
    std::shared_lock<std::shared_mutex> lock(mutex_); // Shared lock for reading
    
    auto it = store_.find(key);
    if (it != store_.end()) {
        // TODO: For phase 2, check TTL here and lazily evict if expired
        return it->second.data;
    }
    return std::nullopt;
}

bool Storage::del(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_); // Exclusive lock for writing
    return store_.erase(key) > 0;
}
