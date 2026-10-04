#pragma once

#include <string>
#include <unordered_map>

namespace library {
using Row = std::unordered_map<std::string, std::string>;

struct SessionUser {
    int id{};
    std::string role;
    std::string name;
    std::string email;
};

struct BookInput {
    std::string title, author, category, isbn, publisher;
    int publicationYear{}, totalQuantity{};
};

struct StudentInput {
    std::string studentCode, name, email, password, phone, department, course;
    int studyYear{};
};
} // namespace library
