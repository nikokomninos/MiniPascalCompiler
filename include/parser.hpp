#pragma once

#include "scanner.hpp"
#include "symbol_table.hpp"
#include "token.hpp"

// All possible types of Non-terminal symbols
enum class NonTerm {
  Error,
  Program,
  Header,
  Declarations,
  VariableDeclarationSection,
  VariableDeclarations,
  VariableDeclarationsPrime,
  VariableDeclaration,
  IdentifierList,
  IdentifierListPrime,
  Type,
  ProcedureDeclarations,
  ProcedureHeader,
  Block,
  Statements,
  StatementsPrime,
  Statement,
  IdStatement,
  ElseClause,
  ExpressionList,
  ExpressionListPrime,
  Expression,
  ExpressionPrime,
  SimpleExpression,
  SimpleExpressionPrime,
  Term,
  TermPrime,
  Factor
};

// All possible types of Terminal symbols
enum class Term {
  Error,
  Program,
  Var,
  Procedure,
  Id,
  Comma,
  Colon,
  Integer,
  Real,
  Begin,
  If,
  While,
  Semicolon,
  End,
  Else,
  Num,
  Assign, // :=
  LeftParen,
  Not,
  Period, // TODO possibly remove
  AddOp,
  Then,
  Do,
  RelOp,
  RightParen,
  MulOp
};

class Parser {
public:
  /**
   * @brief This class represents a parser. It contains functions related
   * to parsing the tokenized stream using an LL(1) grammar
   *
   * @param &scanner The scanner containing the tokenized source stream
   * @param &symbol_table The shared symbol table
   *
   * @note Pre-condition: the scanner has tokenized the source before parse()
   * is called. The scanner and symbol table must outlive the parser
   */
  Parser(const Scanner &scanner, const SymbolTable &symbol_table);

  /**
   * @brief Parses the tokenized stream stored in m_tokens.
   *
   * @details Achieved by using a stack of terminal and non-terminal symbols,
   * starting with Program. Non-terminal symbols are expanded using the
   * production table and the current lookahead token. Terminal symbols are
   * matched against the current token before advancing through the stream.
   * Each expanded non-terminal and matched terminal is printed on its own line
   *
   * @throws std::length_error Throws if the tokenized stream is empty
   * @throws std::logic_error Throws if the input ends unexpectedly, a token
   * has no valid terminal type, no production exists for the current
   * lookahead, a terminal does not match, or trailing tokens remain
   *
   * @see convert_token_type(TokenType token_type)
   */
  void parse();

private:
  [[maybe_unused]] const Scanner &m_scanner; // The scanner containing the tokenized stream
  [[maybe_unused]] const SymbolTable &m_symbol_table; // Shared symbol table
  const std::vector<Token> &m_tokens; // The scanned, tokenized stream

  /**
   * @brief A helper function that converts a token type to a terminal symbol
   *
   * @details Groups relational, additive, and multiplicative operators into
   * RelOp, AddOp, and MulOp respectively. Other recognized token types are
   * converted to their corresponding terminal symbols
   *
   * @param token_type The TokenType to convert to a terminal symbol
   *
   * @return The corresponding Term, or Term::Error if the token type is
   * not recognized by the grammar
   */
  Term convert_token_type(TokenType token_type);
};
