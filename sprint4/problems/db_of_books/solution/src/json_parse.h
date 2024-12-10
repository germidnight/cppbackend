#pragma once
#include "pg_database.h"

#include <boost/json.hpp>
#include <string>
#include <optional>
#include <vector>

namespace json {

enum class Command {
    ADD_BOOK,
    GET_ALL_BOOKS,
    EXIT
};

struct ParsingResult {
    Command cmd;
    std::optional<database::book> payload;
};

ParsingResult ParseCmd(std::string json_str) {
    const std::string action_str = "action";
    const std::string payload_str = "payload";

    const std::string title_str = "title";
    const std::string year_str = "year";
    const std::string author_str = "author";
    const std::string isbn_str = "ISBN";

    const std::string command_add_book = "add_book";
    const std::string command_all_books = "all_books";
    const std::string command_exit = "exit";

    auto command_data = boost::json::parse(std::move(json_str));
    auto action_val = command_data.at(action_str).as_string();

    ParsingResult result;
    if (action_val == command_add_book) {
        result.cmd = Command::ADD_BOOK;
        auto payload_obj = command_data.at(payload_str).as_object();
        database::book temp_book;
        temp_book.title = payload_obj.at(title_str).as_string();
        temp_book.author = payload_obj.at(author_str).as_string();
        temp_book.year = static_cast<uint64_t>(payload_obj.at(year_str).as_int64());
        if (payload_obj.at(isbn_str).is_null()) {
            temp_book.isbn = std::nullopt;
        } else {
            temp_book.isbn = payload_obj.at(isbn_str).as_string();
        }
        result.payload = std::move(temp_book);
    } else if (action_val == command_all_books) {
        result.cmd = Command::GET_ALL_BOOKS;
    } else if (action_val == command_exit) {
        result.cmd = Command::EXIT;
    } else {
        result.cmd = Command::EXIT;
    }
    return result;
}

std::string ReturnAddBookRequest(bool result) {
    const std::string result_str = "result";

    boost::json::object res_obj;
    res_obj[result_str] = result;
    boost::json::value val_json(res_obj);
    return {boost::json::serialize(val_json)};
}

std::string ReturnGetAllBooksRequest(std::vector<database::book> books_vec) {
    const std::string id_str = "id";
    const std::string title_str = "title";
    const std::string year_str = "year";
    const std::string author_str = "author";
    const std::string isbn_str = "ISBN";

    boost::json::array books_arr;
    for (auto& book : books_vec) {
        boost::json::object book_obj;
        book_obj[id_str]        = book.id;
        book_obj[title_str]     = std::move(book.title);
        book_obj[author_str]    = std::move(book.author);
        book_obj[year_str]      = book.year;
        if (book.isbn) {
            book_obj[isbn_str]  = std::move(book.isbn.value());
        } else {
            book_obj[isbn_str]  = nullptr;
        }
        books_arr.emplace_back(std::move(book_obj));
    }
    boost::json::value val_json(books_arr);
    return {boost::json::serialize(val_json)};
}

} // namespace json_loader
