#include <iostream>
#include <optional>
#include <pqxx/pqxx>
#include <string>

#include "json_parse.h"
#include "pg_database.h"

using namespace std::literals;

int main(int argc, char** argv) {
    try {
        if (argc == 1) {
            std::cout << "Usage: book_manager <conn-string>\n"sv;
            return EXIT_SUCCESS;
        } else if (argc != 2) {
            std::cerr << "Invalid command line\n"sv;
            return EXIT_FAILURE;
        }

        database::PGSQLExchange pgsql(argv[1]);

        std::string command;
        while (std::getline(std::cin, command)) {
            auto parsed_cmd = json::ParseCmd(std::move(command));
	    command = {};
            switch (parsed_cmd.cmd) {
            case json::Command::ADD_BOOK : {
                bool add_result = pgsql.AddBook(std::move(parsed_cmd.payload.value()));
                std::cout << json::ReturnAddBookRequest(add_result) << std::endl;
                break;
            }
            case json::Command::GET_ALL_BOOKS : {
                std::cout << json::ReturnGetAllBooksRequest(pgsql.GetAllBooks()) << std::endl;
                break;
            }
            case json::Command::EXIT: {
                return EXIT_SUCCESS;
            }
            }
        }
    } catch(const std::exception& e) {
        std::cerr << "Ошибка: "sv << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
