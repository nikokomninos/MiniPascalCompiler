#include "scanner.hpp"
#include "symbol_table.hpp"
#include "token.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iomanip>
#include <iostream>

namespace {

// Lookup table for printing token type names (since enum names cannot
// be printed)
inline constexpr std::array<std::string, 37> token_type_names{
    "TOK_PROGRAM",    "TOK_PROCEDURE", "TOK_VAR",        "TOK_BEGIN",
    "TOK_END",        "TOK_IF",        "TOK_THEN",       "TOK_ELSE",
    "TOK_WHILE",      "TOK_DO",        "TOK_INTEGER",    "TOK_REAL",
    "TOK_EQUAL",      "TOK_GREATER",   "TOK_LESS",       "TOK_GREATER_EQUAL",
    "TOK_LESS_EQUAL", "TOK_NOT_EQUAL", "TOK_NOT",        "TOK_PLUS",
    "TOK_MINUS",      "TOK_OR",        "TOK_MULTIPLY",   "TOK_DIVIDE",
    "TOK_DIV",        "TOK_MOD",       "TOK_AND",        "TOK_ASSIGN",
    "TOK_IDENTIFIER", "TOK_NUMBER",    "TOK_LEFT_PAREN", "TOK_RIGHT_PAREN",
    "TOK_COMMA",      "TOK_SEMICOLON", "TOK_COLON",      "TOK_PERIOD",
    "TOK_WORD",
};
} // namespace

Scanner::Scanner(const std::string &src, const SymbolTable &symbol_table)
    : m_src{src}, m_symbol_table{symbol_table} {}

void Scanner::get_tokens() {
  std::string lexeme{};
  [[maybe_unused]] std::size_t line_number{1}; // For future use

  // Lambda function to look a character ahead without going out
  // of bounds
  auto look_ahead{[this](std::size_t index) {
    return index + 1 < m_src.size() ? m_src.at(index + 1) : '\0';
  }};

  for (std::size_t i{0}; i < m_src.size(); ++i) {
    const char c{m_src.at(i)};

    if (c == '\n')
      ++line_number;

    if (std::isspace(c)) {
      push_lexeme(lexeme);
      continue;
    }

    if (std::isalnum(c)) {
      lexeme.push_back(c);
      continue;
    }

    // A decimal point belongs to a number only when followed by a digit
    if (c == '.' && !lexeme.empty() &&
        std::all_of(lexeme.begin(), lexeme.end(),
                    [](unsigned char digit) { return std::isdigit(digit); }) &&
        std::isdigit(look_ahead(i))) {
      lexeme.push_back(c);
      continue;
    }

    // Symbols end the preceding lexeme and are pushed as their own tokens
    push_lexeme(lexeme);
    std::string symbol{c};
    const char next{look_ahead(i)};
    if ((c == ':' && next == '=') ||
        (c == '<' && (next == '=' || next == '>')) ||
        (c == '>' && next == '=')) {
      symbol.push_back(next);
      ++i;
    }
    m_tokens.emplace_back(symbol, get_token_type(symbol));
  }

  push_lexeme(lexeme);
}

void Scanner::push_lexeme(std::string &lexeme) {
  if (!lexeme.empty()) {
    m_tokens.emplace_back(lexeme, get_token_type(lexeme));
    lexeme.clear();
  }
}

void Scanner::print_token_stream() const {
  for (const Token &t : m_tokens) {
    std::cout << std::left << std::setw(15) << t.lexeme << " : "
              << token_type_names.at(static_cast<size_t>(t.type)) << "\n";
  }
}

TokenType Scanner::get_token_type(const std::string &lexeme) const {
  // Checks the symbol table explicitly for pre-defined tokens
  if (const auto predefined_token{
          m_symbol_table.lookup_predefined_token(lexeme)}) {
    return *predefined_token;
  }

  if (!lexeme.empty() &&
      std::all_of(lexeme.begin(), lexeme.end(),
                  [](unsigned char c) { return std::isdigit(c) || c == '.'; }))
    return TokenType::TOK_NUMBER;

  if (!lexeme.empty() && std::isalpha(lexeme.at(0)) &&
      std::all_of(lexeme.begin() + 1, lexeme.end(),
                  [](unsigned char c) { return std::isalnum(c); }))
    return TokenType::TOK_IDENTIFIER;

  return TokenType::TOK_WORD;
}
