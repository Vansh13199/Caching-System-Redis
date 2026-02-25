#include "storage.h"

void Storage::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_); // Exclusive lock
    store_[key] = Value(value);
}

std::pair<bool, std::string> Storage::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_); // Exclusive lock for simplicity/compatibility
    
    auto it = store_.find(key);
    if (it != store_.end()) {
        // TODO: For phase 2, check TTL here and lazily evict if expired
        return {true, it->second.data};
    }
    return {false, ""};
}

bool Storage::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_); // Exclusive lock
    return store_.erase(key) > 0;
}
