#pragma once

#include "symbol_table.hpp"
#include "token.hpp"
#include <vector>

class Scanner {
public:
  /**
   * @brief This class represents a scanner. It contains functions related
   * to scanning and tokenizing the stream
   *
   * @param &src A string containing the source code passed into the executable
   * as an argument. It is moved into member variable m_src
   */
  Scanner(const std::string &src, const SymbolTable &symbol_table);

  /**
   * @brief Scans through the given source stored in m_src and tokenizes it.
   *
   * @details Achieved by going through each character in the stream, and
   * determining where a lexeme begins and ends, as well as its TokenType based
   * on a given set of rules:
   *
   * - TOK_<PREDEFINED>, where PREDEFINED is one of the predefined tokens found
   * in the symbol table.
   * - TOK_IDENTIFIER if the lexeme contains alphanumeric characters
   * - TOK_NUMBER if the lexeme contains only digits, and optionally a
   *   decimal point.
   * - TOK_WORD if the lexeme does not match any other token type.
   *
   * The tokenized stream is stored in member variable m_tokens.
   */
  void tokenize_stream();

  /**
   * @brief Prints the tokenized stream into a list of the following format:
   *
   * LEXEME : TOKENTYPE
   */
  void print_token_stream() const;

  const std::vector<Token> &tokens() const { return m_tokens;  }

private:
  std::string m_src;           // The original source stream
  std::vector<Token> m_tokens; // The scanned, tokenized stream
  const SymbolTable &m_symbol_table; // Shared symbol table

  /**
   * @brief A helper function that pushes the current lexeme to the
   * token stream, if the lexeme string is not empty. Calls helper function
   * check_token_type to determine the TokenType of the lexeme
   *
   * @param &lexeme a reference to the current lexeme string
   * @param line_number the source line where the lexeme starts
   *
   * @see check_token_type()
   */
  void push_lexeme(std::string &lexeme, std::size_t line_number);

  /**
   * @brief A helper function that determines the token type of a lexeme
   *
   * @param &lexeme a reference to the current lexeme string
   *
   * @return The TokenType associated with the lexeme
   */
  TokenType get_token_type(const std::string_view &lexeme) const;
};
