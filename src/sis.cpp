#include "sis.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <pqxx/pqxx>
#include <sstream>
#include <stdexcept>

namespace sis {

namespace {
std::string trim(const std::string& s) {
    const auto first = std::find_if_not(s.begin(), s.end(), [](unsigned char ch) {
        return std::isspace(ch);
    });
    if (first == s.end()) {
        return "";
    }
    const auto last = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char ch) {
        return std::isspace(ch);
    }).base();
    return std::string(first, last);
}

std::string get_env_or(const char* key, const std::string& fallback) {
    const char* val = std::getenv(key);
    if (val != nullptr && *val != '\0') {
        return std::string(val);
    }
    return fallback;
}
} // namespace

void log_info(const std::string& msg) {
    std::clog << "[INFO] " << msg << '\n';
}

void log_error(const std::string& msg) {
    std::cerr << "[ERROR] " << msg << '\n';
}

std::string DbConfig::connection_string() const {
    std::ostringstream oss;
    oss << "host=" << host
        << " port=" << port
        << " dbname=" << dbname
        << " user=" << user
        << " password=" << password;
    return oss.str();
}

DbConfig DbConfig::from_env() {
    DbConfig cfg;
    cfg.host = get_env_or("DB_HOST", "localhost");
    cfg.port = get_env_or("DB_PORT", "5432");
    cfg.dbname = get_env_or("DB_NAME", get_env_or("POSTGRES_DB", "sis_db"));
    cfg.user = get_env_or("DB_USER", get_env_or("POSTGRES_USER", "sis_user"));
    cfg.password = get_env_or("DB_PASSWORD", get_env_or("POSTGRES_PASSWORD", ""));
    return cfg;
}

bool is_valid_id(std::int64_t id) {
    return id > 0;
}

bool is_valid_non_empty(const std::string& value, std::size_t max_len) {
    const std::string t = trim(value);
    return !t.empty() && t.size() <= max_len;
}

bool is_valid_email(const std::string& email) {
    const std::string t = trim(email);
    if (t.empty() || t.size() > 150) {
        return false;
    }
    const auto at = t.find('@');
    if (at == std::string::npos || at == 0 || at + 3 >= t.size()) {
        return false;
    }
    if (t.find('@', at + 1) != std::string::npos) {
        return false;
    }
    const auto dot = t.find('.', at + 2);
    return dot != std::string::npos && dot + 1 < t.size();
}

void validate_student(const Student& s) {
    if (!is_valid_id(s.id)) {
        throw std::invalid_argument("Student ID must be a positive integer.");
    }
    if (!is_valid_non_empty(s.name, 100)) {
        throw std::invalid_argument("Name must be non-empty and at most 100 characters.");
    }
    if (!is_valid_non_empty(s.surname, 100)) {
        throw std::invalid_argument("Surname must be non-empty and at most 100 characters.");
    }
    if (!is_valid_non_empty(s.department, 50)) {
        throw std::invalid_argument("Department must be non-empty and at most 50 characters.");
    }
    if (!is_valid_email(s.email)) {
        throw std::invalid_argument("Email format is invalid.");
    }
}

StudentRepository::StudentRepository(std::string conn_str)
    : conn_str_(std::move(conn_str)) {}

void StudentRepository::add_student(const Student& student) {
    validate_student(student);
    const std::string clean_name = trim(student.name);
    const std::string clean_surname = trim(student.surname);
    const std::string clean_dept = trim(student.department);
    const std::string clean_email = trim(student.email);

    pqxx::connection conn(conn_str_);
    pqxx::work txn(conn);
    txn.exec_params(
        "INSERT INTO students (id, name, surname, department, email) "
        "VALUES ($1, $2, $3, $4, $5)",
        student.id,
        clean_name,
        clean_surname,
        clean_dept,
        clean_email
    );
    txn.commit();
    log_info("Added student ID=" + std::to_string(student.id));
}

std::vector<Student> StudentRepository::list_students() {
    pqxx::connection conn(conn_str_);
    pqxx::read_transaction txn(conn);
    const pqxx::result rows = txn.exec(
        "SELECT id, name, surname, department, email FROM students ORDER BY id ASC"
    );
    std::vector<Student> out;
    out.reserve(rows.size());
    for (const auto& row : rows) {
        Student s;
        s.id = row["id"].as<std::int64_t>();
        s.name = row["name"].as<std::string>();
        s.surname = row["surname"].as<std::string>();
        s.department = row["department"].as<std::string>();
        s.email = row["email"].as<std::string>();
        out.push_back(std::move(s));
    }
    txn.commit();
    log_info("Listed " + std::to_string(out.size()) + " student(s).");
    return out;
}

void StudentRepository::update_student(
    std::int64_t id,
    const std::optional<std::string>& name,
    const std::optional<std::string>& surname,
    const std::optional<std::string>& department,
    const std::optional<std::string>& email
) {
    if (!is_valid_id(id)) {
        throw std::invalid_argument("Student ID must be a positive integer.");
    }
    if (!name && !surname && !department && !email) {
        throw std::invalid_argument("At least one field (--name, --surname, --dept, --email) must be provided.");
    }
    if (name && !is_valid_non_empty(*name, 100)) {
        throw std::invalid_argument("Invalid --name value.");
    }
    if (surname && !is_valid_non_empty(*surname, 100)) {
        throw std::invalid_argument("Invalid --surname value.");
    }
    if (department && !is_valid_non_empty(*department, 50)) {
        throw std::invalid_argument("Invalid --dept value.");
    }
    if (email && !is_valid_email(*email)) {
        throw std::invalid_argument("Invalid --email value.");
    }

    pqxx::connection conn(conn_str_);
    pqxx::work txn(conn);
    const pqxx::result existing = txn.exec_params(
        "SELECT name, surname, department, email FROM students WHERE id = $1",
        id
    );
    if (existing.empty()) {
        throw std::runtime_error("Student not found with ID=" + std::to_string(id));
    }

    const std::string new_name = name ? trim(*name) : existing[0]["name"].as<std::string>();
    const std::string new_surname = surname ? trim(*surname) : existing[0]["surname"].as<std::string>();
    const std::string new_dept = department ? trim(*department) : existing[0]["department"].as<std::string>();
    const std::string new_email = email ? trim(*email) : existing[0]["email"].as<std::string>();

    txn.exec_params(
        "UPDATE students SET name = $1, surname = $2, department = $3, email = $4, updated_at = NOW() "
        "WHERE id = $5",
        new_name,
        new_surname,
        new_dept,
        new_email,
        id
    );
    txn.commit();
    log_info("Updated student ID=" + std::to_string(id));
}

void StudentRepository::delete_student(std::int64_t id) {
    if (!is_valid_id(id)) {
        throw std::invalid_argument("Student ID must be a positive integer.");
    }
    pqxx::connection conn(conn_str_);
    pqxx::work txn(conn);
    const pqxx::result res = txn.exec_params(
        "DELETE FROM students WHERE id = $1",
        id
    );
    if (res.affected_rows() == 0) {
        throw std::runtime_error("Student not found with ID=" + std::to_string(id));
    }
    txn.commit();
    log_info("Deleted student ID=" + std::to_string(id));
}

namespace {
std::int64_t parse_id(const std::string& raw) {
    std::size_t idx = 0;
    std::int64_t val = 0;
    try {
        val = std::stoll(raw, &idx);
    } catch (const std::exception&) {
        throw std::invalid_argument("Invalid numeric ID: " + raw);
    }
    if (idx != raw.size() || !is_valid_id(val)) {
        throw std::invalid_argument("Invalid positive integer ID: " + raw);
    }
    return val;
}

void print_usage() {
    std::cout
        << "Usage:\n"
        << "  sis add --id <id> --name <name> --surname <surname> --dept <dept> --email <email>\n"
        << "  sis list\n"
        << "  sis update --id <id> [--name <name>] [--surname <surname>] [--dept <dept>] [--email <email>]\n"
        << "  sis delete --id <id>\n";
}
} // namespace

int run_cli(int argc, char* argv[], StudentRepository& repo) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    const std::string cmd = argv[1];
    try {
        if (cmd == "--help" || cmd == "-h" || cmd == "help") {
            print_usage();
            return 0;
        }

        if (cmd == "list") {
            const auto students = repo.list_students();
            if (students.empty()) {
                std::cout << "No students found.\n";
                return 0;
            }
            std::cout << "ID | Name | Surname | Department | Email\n";
            for (const auto& s : students) {
                std::cout << s.id << " | " << s.name << " | " << s.surname
                          << " | " << s.department << " | " << s.email << '\n';
            }
            return 0;
        }

        std::optional<std::int64_t> id;
        std::optional<std::string> name;
        std::optional<std::string> surname;
        std::optional<std::string> dept;
        std::optional<std::string> email;

        for (int i = 2; i < argc; ++i) {
            const std::string flag = argv[i];
            if (i + 1 >= argc) {
                throw std::invalid_argument("Missing value for argument: " + flag);
            }
            const std::string val = argv[++i];
            if (flag == "--id") {
                id = parse_id(val);
            } else if (flag == "--name") {
                name = val;
            } else if (flag == "--surname") {
                surname = val;
            } else if (flag == "--dept") {
                dept = val;
            } else if (flag == "--email") {
                email = val;
            } else {
                throw std::invalid_argument("Unknown option: " + flag);
            }
        }

        if (cmd == "add") {
            if (!id || !name || !surname || !dept || !email) {
                throw std::invalid_argument("add requires --id, --name, --surname, --dept, and --email.");
            }
            repo.add_student(Student{*id, *name, *surname, *dept, *email});
            std::cout << "Student added: ID=" << *id << '\n';
            return 0;
        }

        if (cmd == "update") {
            if (!id) {
                throw std::invalid_argument("update requires --id.");
            }
            repo.update_student(*id, name, surname, dept, email);
            std::cout << "Student updated: ID=" << *id << '\n';
            return 0;
        }

        if (cmd == "delete") {
            if (!id) {
                throw std::invalid_argument("delete requires --id.");
            }
            repo.delete_student(*id);
            std::cout << "Student deleted: ID=" << *id << '\n';
            return 0;
        }

        throw std::invalid_argument("Unknown command: " + cmd);
    } catch (const std::exception& ex) {
        log_error(ex.what());
        return 1;
    }
}

} // namespace sis
