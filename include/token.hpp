#pragma once

#include <cstddef>
#include <string>

enum class TokenType {
  // Keywords
  TOK_PROGRAM,
  TOK_PROCEDURE,
  TOK_VAR,
  TOK_BEGIN,
  TOK_END,
  TOK_IF,
  TOK_THEN,
  TOK_ELSE,
  TOK_WHILE,
  TOK_DO,

  // Data Types
  TOK_INTEGER,
  TOK_REAL,

  // Rel ops
  TOK_EQUAL,         // =
  TOK_GREATER,       // >
  TOK_LESS,          // <
  TOK_GREATER_EQUAL, // >=
  TOK_LESS_EQUAL,    // <=
  TOK_NOT_EQUAL,     // <>
  TOK_NOT,

  // Add ops
  TOK_PLUS, // +
  TOK_MINUS, // -
  TOK_OR,

  // Mul ops
  TOK_MULTIPLY, // *
  TOK_DIVIDE, // /
  TOK_DIV,
  TOK_MOD,
  TOK_AND,

  // Assign ops
  TOK_ASSIGN, // :=

  // Symbol types
  TOK_IDENTIFIER,
  TOK_NUMBER,

  // Separating operators
  TOK_LEFT_PAREN, // (
  TOK_RIGHT_PAREN, // )
  TOK_COMMA, // ,
  TOK_SEMICOLON, // ;
  TOK_COLON, // :
  TOK_PERIOD, // .

  // Extra token types
  TOK_WORD,
};

struct Token {
  std::string lexeme;
  TokenType type;
  std::size_t line_number{1};
};
