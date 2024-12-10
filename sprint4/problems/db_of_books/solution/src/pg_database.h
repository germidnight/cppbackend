#pragma once
#include <cstdint>
#include <pqxx/pqxx>
#include <string>
#include <vector>

// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;

namespace database {

struct book {
    uint64_t id;
    std::string title;
    std::string author;
    uint64_t year;
    std::optional<std::string> isbn;
};

class PGSQLExchange {
    const pqxx::zview add_book_sql = "add_book"_zv;
public:
    PGSQLExchange(std::string conn_params)
                : conn_{conn_params} {
        pqxx::work w(conn_);
        w.exec("CREATE TABLE IF NOT EXISTS books (id SERIAL PRIMARY KEY, \
                        title varchar(100) NOT NULL, \
                        author varchar(100) NOT NULL, \
                        year integer NOT NULL CHECK (year > 0), \
                        ISBN char(13) UNIQUE);"_zv);
        w.commit();
        conn_.prepare(add_book_sql, "INSERT INTO books (id, title, author, year, ISBN) VALUES \
                                                    (DEFAULT, $1, $2, $3, $4);"_zv);
    }

    bool AddBook(book insert_book) {
        pqxx::work w(conn_);
        try {
            w.exec_prepared(add_book_sql, insert_book.title, insert_book.author,
                                          insert_book.year, insert_book.isbn);
            w.commit();
        } catch (const std::exception& exc) {
            return false;
        }
        return true;
    }

    std::vector<book> GetAllBooks() {
        pqxx::read_transaction r(conn_);
        std::vector<book> books;
        constexpr auto get_all_books_sql = "SELECT id, title, author, year, isbn FROM books \
                                ORDER BY year DESC, title ASC, author ASC, isbn ASC;"_zv;
        for (auto [id, title, author, year, isbn] :
            r.query<uint64_t, std::string, std::string, uint64_t, std::optional<std::string>>(get_all_books_sql)) {
            books.emplace_back(id, std::move(title), std::move(author), year, std::move(isbn));
        }
        return books;
    }
private:
    pqxx::connection conn_;
};

} // namespace database
