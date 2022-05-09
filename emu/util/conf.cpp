#include <cstdint>
#include <cstddef>
#include <vector>
#include <charconv>
#include <fmt/core.h>
#include "conf.hpp"
#include "io.hpp"
#include "string.hpp"

namespace {

std::string type_to_string(conf::Type t)
{
    switch (t) {
    case conf::Type::Int:    return "int";
    case conf::Type::Float:  return "float";
    case conf::Type::Bool:   return "bool";
    case conf::Type::String: return "string";
    default: return "";
    }
}

struct Token {
    enum class Type {
        Ident, Int, Float, True, False,
        String, EqualSign, Newline, Error, End,
    } type;
    std::string_view text;
    std::size_t pos;
};

bool is_ident_char(char c) { return str::is_alpha(c) || c == '_' || c == '-'; }

struct Lexer {
    std::string_view text;
    std::size_t cur   = 0;
    std::size_t start = 0;

    explicit Lexer(std::string_view s) : text{s} {}

    char peek() const               { return text[cur]; }
    char advance()                  { return text[cur++]; }
    bool at_end() const             { return text.size() == cur; }

    auto position_of(Token t) const
    {
        auto tmp = text.substr(0, t.pos);
        auto line = std::count(tmp.begin(), tmp.end(), '\n') + 1;
        auto column = t.pos - tmp.find_last_of('\n');
        return std::make_pair(line, column);
    }

    Token make(Token::Type type, std::string_view msg = "")
    {
        return (Token) {
            .type = type,
            .text = msg.empty() ? text.substr(start, cur - start) : msg,
            .pos  = start,
        };
    }

    void skip()
    {
        for (;;) {
            switch (peek()) {
            case ' ': case '\r': case '\t': cur++; break;
            case '#':
                while (peek() != '\n')
                    advance();
                break;
            default:
                return;
            }
        }
    }

    Token number()
    {
        while (str::is_digit(text[cur]))
            advance();
        if (peek() == '.') {
            advance();
            while (str::is_digit(text[cur]))
                advance();
            return make(Token::Type::Float);
        }
        return make(Token::Type::Int);
    }

    Token ident()
    {
        while (is_ident_char(peek()) || str::is_digit(peek()))
            advance();
        auto word = text.substr(start, cur - start);
        return make(word == "true"  ? Token::Type::True
                  : word == "false" ? Token::Type::False
                  :                   Token::Type::Ident);
    }

    Token string_token()
    {
        while (peek() != '"' && !at_end())
            advance();
        if (at_end())
            return make(Token::Type::Error, "unterminated string");
        advance();
        return make(Token::Type::String);
    }

    Token lex()
    {
        skip();
        start = cur;
        if (at_end())
            return make(Token::Type::End);
        char c = advance();
        return c == '='    ? make(Token::Type::EqualSign)
             : c == '\n'   ? make(Token::Type::Newline)
             : c == '"'    ? string_token()
             : str::is_digit(c) ? number()
             : is_ident_char(c) ? ident()
             : make(Token::Type::Error, "unexpected character");
    }
};

struct Parser {
    Lexer lexer;
    Token cur, prev;
    bool had_error = false;
    const conf::ValidConfig &valid;

    struct ParseError : std::runtime_error {
        using std::runtime_error::runtime_error;
        using std::runtime_error::what;
    };

    explicit Parser(std::string_view s, const conf::ValidConfig &v) : lexer{s}, valid{v} {}

    void error(Token t, std::string_view msg)
    {
        had_error = true;
        auto [line, col] = lexer.position_of(t);
        throw ParseError(fmt::format("{}:{}: parse error{}: {}",
            line, col,
              t.type == Token::Type::End   ? " on end of file"
            : t.type == Token::Type::Error ? ""
            : fmt::format(" at '{}'", t.text),
            msg)
        );
    }

    void advance()
    {
        prev = cur;
        cur = lexer.lex();
        if (cur.type == Token::Type::Error)
            error(cur, cur.text);
    }

    void consume(Token::Type type, std::string_view msg)
    {
        if (cur.type == type) {
            advance();
            return;
        }
        error(prev, msg);
    }

    bool match(Token::Type type)
    {
        if (cur.type != type)
            return false;
        advance();
        return true;
    }

    void add(conf::Type type, conf::Data &data, Token ident, Token t)
    {
        auto r = valid.find(std::string(ident.text));
        if (r == valid.end())
            error(ident, "invalid key");
        if (type != r->second.type())
            error(t, fmt::format("mismatched types for key '{}' (got {}, should be {})",
                    ident.text, type_to_string(type), type_to_string(r->second.type())));
        auto &pos = data[std::string(ident.text)];
        switch (type) {
        case conf::Type::Int:    pos = conf::Value(str::to_num<  int>(t.text).value()); break;
        case conf::Type::Float:  pos = conf::Value(str::to_num<float>(t.text).value()); break;
        case conf::Type::String: pos = conf::Value(std::string(t.text.substr(1, t.text.size() - 2))); break;
        case conf::Type::Bool:   pos = conf::Value(t.type == Token::Type::True); break;
        }
    }

    std::optional<conf::Data> parse(auto &&display_error)
    {
        conf::Data data;
        advance();
        while (!lexer.at_end()) {
            try {
                if (!match(Token::Type::Newline)) {
                    consume(Token::Type::Ident, "expected identifier");
                    auto ident = prev;
                    consume(Token::Type::EqualSign, fmt::format("expected '=' after identifier '{}'", ident.text));
                         if (match(Token::Type::Int))    add(conf::Type::Int, data, ident, prev);
                    else if (match(Token::Type::Float))  add(conf::Type::Float, data, ident, prev);
                    else if (match(Token::Type::String)) add(conf::Type::String, data, ident, prev);
                    else if (match(Token::Type::True))   add(conf::Type::Bool, data, ident, prev);
                    else if (match(Token::Type::False))  add(conf::Type::Bool, data, ident, prev);
                    else error(prev, "expected value after '='");
                    consume(Token::Type::Newline, fmt::format("expected newline after value '{}'", prev.text));
                }
            } catch (const ParseError &error) {
                display_error(error.what());
                while (cur.type != Token::Type::End && cur.type != Token::Type::Newline)
                    advance();
                advance();
            }
        }
        if (had_error)
            return std::nullopt;
        return data;
    }
};

} // namespace

namespace conf {

std::optional<Data> parse(std::string_view text, const ValidConfig &valid,
                          std::function<void(std::string_view)> display_error)
{
    Parser parser{text, valid};
    auto res = parser.parse(display_error);
    if (!res)
        return std::nullopt;
    auto &data = res.value();
    for (auto [k, v] : valid) {
        if (auto r = data.find(k); r == data.end()) {
            display_error(fmt::format("warning: missing key '{}' (default will be used)", k));
            data[k] = v;
        }
    }
    return data;
}

std::optional<Data> parse_or_create(std::string_view path, const ValidConfig &valid,
                     std::function<void(std::string_view)> display_error)
{
    auto text = io::read_file(path);
    if (!text) {
        display_error(fmt::format("error: couldn't open file {}, creating new one...", path));
        Data data(valid);
        create(path, data, display_error);
        return data;
    }
    return parse(text.value(), valid, display_error);
}

void create(std::string_view pathname, const Data &conf, std::function<void(std::string_view)> display_error)
{
    if (conf.size() == 0) {
        display_error(fmt::format("error: no data"));
        return;
    }
    FILE *file = fopen(pathname.data(), "wb");
    if (!file) {
        display_error(fmt::format("error: couldn't create file {}\n", pathname));
        return;
    }
    auto max = std::max_element(conf.begin(), conf.end(), [](const auto &a, const auto &b) {
        return a.first.size() < b.first.size();
    });
    std::size_t width = max->first.size();
    for (auto [k, v] : conf)
        fmt::print(file, "{:{}} = {}\n", k, width, v.to_string());
}

} // namespace conf
