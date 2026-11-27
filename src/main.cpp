#include "sis.hpp"

#include <exception>

int main(int argc, char* argv[]) {
    try {
        const sis::DbConfig cfg = sis::DbConfig::from_env();
        sis::StudentRepository repo(cfg.connection_string());
        return sis::run_cli(argc, argv, repo);
    } catch (const std::exception& ex) {
        sis::log_error(ex.what());
        return 1;
    }
}
