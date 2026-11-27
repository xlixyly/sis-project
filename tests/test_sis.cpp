#include "sis.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {

void test_valid_id() {
    assert(sis::is_valid_id(1));
    assert(sis::is_valid_id(1001));
    assert(!sis::is_valid_id(0));
    assert(!sis::is_valid_id(-5));
}

void test_valid_non_empty() {
    assert(sis::is_valid_non_empty("Ali", 100));
    assert(sis::is_valid_non_empty("  CSE  ", 50));
    assert(!sis::is_valid_non_empty("", 100));
    assert(!sis::is_valid_non_empty("   ", 100));
    assert(!sis::is_valid_non_empty("abcdef", 5));
}

void test_valid_email() {
    assert(sis::is_valid_email("ali@school.edu"));
    assert(sis::is_valid_email("hasan.yasin@stu.fbu.edu.tr"));
    assert(!sis::is_valid_email("invalid-email"));
    assert(!sis::is_valid_email("@school.edu"));
    assert(!sis::is_valid_email("ali@"));
    assert(!sis::is_valid_email("ali@school"));
    assert(!sis::is_valid_email("a@b@c.com"));
}

void test_validate_student_accepts_valid() {
    sis::Student s{1001, "Ali", "Yilmaz", "CSE", "ali@school.edu"};
    sis::validate_student(s);
}

void test_validate_student_rejects_bad_id() {
    bool threw = false;
    try {
        sis::validate_student(sis::Student{0, "Ali", "Yilmaz", "CSE", "ali@school.edu"});
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
}

void test_validate_student_rejects_blank_name() {
    bool threw = false;
    try {
        sis::validate_student(sis::Student{1001, "   ", "Yilmaz", "CSE", "ali@school.edu"});
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
}

void test_validate_student_rejects_bad_email() {
    bool threw = false;
    try {
        sis::validate_student(sis::Student{1001, "Ali", "Yilmaz", "CSE", "bad-email"});
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
}

void test_db_config_connection_string() {
    sis::DbConfig cfg{"sis_postgres", "5432", "sis_db", "sis_user", "secret"};
    const std::string conn = cfg.connection_string();
    assert(conn.find("host=sis_postgres") != std::string::npos);
    assert(conn.find("port=5432") != std::string::npos);
    assert(conn.find("dbname=sis_db") != std::string::npos);
    assert(conn.find("user=sis_user") != std::string::npos);
}

void test_integration_crud_if_enabled() {
    const char* run_int = std::getenv("RUN_DB_TESTS");
    if (run_int == nullptr || std::string(run_int) != "1") {
        std::cout << "[SKIP] Integration DB CRUD test (set RUN_DB_TESTS=1 to run)\n";
        return;
    }

    sis::StudentRepository repo(sis::DbConfig::from_env().connection_string());
    const std::int64_t test_id = 999901;
    try {
        repo.delete_student(test_id);
    } catch (...) {
    }

    repo.add_student(sis::Student{test_id, "Test", "User", "CSE", "test.user999901@stu.fbu.edu.tr"});
    auto all = repo.list_students();
    bool found = false;
    for (const auto& s : all) {
        if (s.id == test_id && s.email == "test.user999901@stu.fbu.edu.tr") {
            found = true;
        }
    }
    assert(found);

    repo.update_student(test_id, std::nullopt, std::nullopt, std::string("SWE"), std::string("updated999901@stu.fbu.edu.tr"));
    all = repo.list_students();
    bool updated = false;
    for (const auto& s : all) {
        if (s.id == test_id && s.department == "SWE" && s.email == "updated999901@stu.fbu.edu.tr") {
            updated = true;
        }
    }
    assert(updated);

    repo.delete_student(test_id);
    std::cout << "[PASS] Integration DB CRUD test\n";
}

} // namespace

int main() {
    test_valid_id();
    test_valid_non_empty();
    test_valid_email();
    test_validate_student_accepts_valid();
    test_validate_student_rejects_bad_id();
    test_validate_student_rejects_blank_name();
    test_validate_student_rejects_bad_email();
    test_db_config_connection_string();
    test_integration_crud_if_enabled();
    std::cout << "All tests passed!\n";
    return 0;
}
