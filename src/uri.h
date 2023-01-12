#ifndef URI_H_
#define URI_H_

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

using namespace std;

class Uri {
public:
    Uri() = default;
    explicit Uri(const string &path) : path_(path) { SetPathToLowercase(); }
    ~Uri() = default;

    inline bool operator<(const Uri &other) const { return path_ < other.path_; }
    inline bool operator==(const Uri &other) const {
        return path_ == other.path_;
    }

    void SetPath(const string &path) {
        path_ = move(path);
        SetPathToLowercase();
    }

    string scheme() const { return scheme_; }
    string host() const { return host_; }
    uint16_t port() const { return port_; }
    string path() const { return path_; }

private:
    // Only the path is supported for now
    string path_;
    string scheme_;
    string host_;
    uint16_t port_;

    void SetPathToLowercase() {
        transform(path_.begin(), path_.end(), path_.begin(),
                       [](char c) { return tolower(c); });
    }
};

#endif  // URI_H_