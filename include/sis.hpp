#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sis {

struct Student {
    std::int64_t id{};
    std::string name;
    std::string surname;
    std::string department;
    std::string email;
};

struct DbConfig {
    std::string host;
    std::string port;
    std::string dbname;
    std::string user;
    std::string password;

    std::string connection_string() const;
    static DbConfig from_env();
};

void log_info(const std::string& msg);
void log_error(const std::string& msg);

bool is_valid_id(std::int64_t id);
bool is_valid_non_empty(const std::string& value, std::size_t max_len);
bool is_valid_email(const std::string& email);
void validate_student(const Student& s);

class StudentRepository {
public:
    explicit StudentRepository(std::string conn_str);

    void add_student(const Student& student);
    std::vector<Student> list_students();
    void update_student(
        std::int64_t id,
        const std::optional<std::string>& name,
        const std::optional<std::string>& surname,
        const std::optional<std::string>& department,
        const std::optional<std::string>& email
    );
    void delete_student(std::int64_t id);

private:
    std::string conn_str_;
};

int run_cli(int argc, char* argv[], StudentRepository& repo);

} // namespace sis
