#include "parser.hpp"
#include "scanner.hpp"
#include "symbol_table.hpp"
#include "token.hpp"
#include <array>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <variant>
#include <vector>

namespace {
// A grammar symbol can be either a non-terminal or a terminal
using Symbol = std::variant<NonTerm, Term>;

constexpr std::size_t NUM_NONTERM = 28;
constexpr std::size_t NUM_TERM = 26;
constexpr std::size_t NUM_PRODUCTIONS = 47;

// Lookup table for printing non-terminal names (since enum names cannot
// be printed)
inline constexpr std::array<std::string_view, NUM_NONTERM> nonterm_names{
    "Error",
    "Program",
    "Header",
    "Declarations",
    "VariableDeclarationSection",
    "VariableDeclarations",
    "VariableDeclarationsPrime",
    "VariableDeclaration",
    "IdentifierList",
    "IdentifierListPrime",
    "Type",
    "ProcedureDeclarations",
    "ProcedureHeader",
    "Block",
    "Statements",
    "StatementsPrime",
    "Statement",
    "IdStatement",
    "ElseClause",
    "ExpressionList",
    "ExpressionListPrime",
    "Expression",
    "ExpressionPrime",
    "SimpleExpression",
    "SimpleExpressionPrime",
    "Term",
    "TermPrime",
    "Factor",
};

// Lookup table for printing terminal names (since enum names cannot be printed)
inline constexpr std::array<std::string_view, NUM_TERM> term_names{
    "Error",   "Program", "Var",    "Procedure",  "Id",    "Comma",     "Colon",
    "Integer", "Real",    "Begin",  "If",         "While", "Semicolon", "End",
    "Else",    "Num",     "Assign", "LeftParen",  "Not",   "Period",    "AddOp",
    "Then",    "Do",      "RelOp",  "RightParen", "MulOp"};

// Lookup table for selecting a production from a non-terminal and lookahead
// terminal. Entries with value zero represent parsing errors
inline constexpr auto production_table = [] {
  std::array<std::array<std::size_t, NUM_TERM>, NUM_NONTERM> table{};
  const auto set = [&table](NonTerm non_term, Term term,
                            std::size_t production) {
    table.at(static_cast<std::size_t>(non_term))
        .at(static_cast<std::size_t>(term)) = production;
  };

  // Program
  set(NonTerm::Program, Term::Program, 1);

  // Header
  set(NonTerm::Header, Term::Program, 2);

  // Declarations
  set(NonTerm::Declarations, Term::Var, 3);
  set(NonTerm::Declarations, Term::Procedure, 3);
  set(NonTerm::Declarations, Term::Begin, 3);

  // VariableDeclarationSection
  set(NonTerm::VariableDeclarationSection, Term::Var, 4);
  set(NonTerm::VariableDeclarationSection, Term::Procedure, 5);
  set(NonTerm::VariableDeclarationSection, Term::Begin, 5);

  // VariableDeclarations
  set(NonTerm::VariableDeclarations, Term::Id, 6);

  // VariableDeclarations'
  set(NonTerm::VariableDeclarationsPrime, Term::Procedure, 8);
  set(NonTerm::VariableDeclarationsPrime, Term::Begin, 8);
  set(NonTerm::VariableDeclarationsPrime, Term::Id, 7);

  // VariableDeclaration
  set(NonTerm::VariableDeclaration, Term::Id, 9);

  // IdentifierList
  set(NonTerm::IdentifierList, Term::Id, 10);

  // IdentifierList'
  set(NonTerm::IdentifierListPrime, Term::Comma, 11);
  set(NonTerm::IdentifierListPrime, Term::Colon, 12);

  // Type
  set(NonTerm::Type, Term::Integer, 13);
  set(NonTerm::Type, Term::Real, 14);

  // ProcedureDeclarations
  set(NonTerm::ProcedureDeclarations, Term::Procedure, 15);
  set(NonTerm::ProcedureDeclarations, Term::Begin, 47);

  // ProcedureHeader
  set(NonTerm::ProcedureHeader, Term::Procedure, 16);

  // Block
  set(NonTerm::Block, Term::Begin, 17);

  // Statements
  set(NonTerm::Statements, Term::Id, 18);
  set(NonTerm::Statements, Term::Begin, 18);
  set(NonTerm::Statements, Term::If, 18);
  set(NonTerm::Statements, Term::While, 18);
  set(NonTerm::Statements, Term::Semicolon, 18);
  set(NonTerm::Statements, Term::End, 18);

  // Statements'
  set(NonTerm::StatementsPrime, Term::Semicolon, 19);
  set(NonTerm::StatementsPrime, Term::End, 20);

  // Statement
  set(NonTerm::Statement, Term::Id, 21);
  set(NonTerm::Statement, Term::Begin, 22);
  set(NonTerm::Statement, Term::If, 23);
  set(NonTerm::Statement, Term::While, 24);
  set(NonTerm::Statement, Term::Semicolon, 25);
  set(NonTerm::Statement, Term::End, 25);
  set(NonTerm::Statement, Term::Else, 25);

  // IdStatement
  set(NonTerm::IdStatement, Term::Assign, 26);
  set(NonTerm::IdStatement, Term::LeftParen, 27);

  // ElseClause
  set(NonTerm::ElseClause, Term::Semicolon, 29);
  set(NonTerm::ElseClause, Term::End, 29);
  // Prefer else Statement (28) over epsilon (29) for the nearest if.
  set(NonTerm::ElseClause, Term::Else, 28);

  // ExpressionList
  set(NonTerm::ExpressionList, Term::Id, 30);
  set(NonTerm::ExpressionList, Term::Num, 30);
  set(NonTerm::ExpressionList, Term::LeftParen, 30);
  set(NonTerm::ExpressionList, Term::Not, 30);
  set(NonTerm::ExpressionList, Term::AddOp, 30);

  // ExpressionList'
  set(NonTerm::ExpressionListPrime, Term::Comma, 31);

  // Expression
  set(NonTerm::Expression, Term::Id, 33);
  set(NonTerm::Expression, Term::Num, 33);
  set(NonTerm::Expression, Term::LeftParen, 33);
  set(NonTerm::Expression, Term::Not, 33);
  set(NonTerm::Expression, Term::AddOp, 33);

  // Expression'
  set(NonTerm::ExpressionPrime, Term::Semicolon, 35);
  set(NonTerm::ExpressionPrime, Term::End, 35);
  set(NonTerm::ExpressionPrime, Term::Else, 35);
  set(NonTerm::ExpressionPrime, Term::Then, 35);
  set(NonTerm::ExpressionPrime, Term::Do, 35);
  set(NonTerm::ExpressionPrime, Term::RelOp, 34);
  set(NonTerm::ExpressionPrime, Term::RightParen, 35);

  // SimpleExpression
  set(NonTerm::SimpleExpression, Term::Id, 36);
  set(NonTerm::SimpleExpression, Term::Num, 36);
  set(NonTerm::SimpleExpression, Term::LeftParen, 36);
  set(NonTerm::SimpleExpression, Term::Not, 36);
  set(NonTerm::SimpleExpression, Term::AddOp, 37);

  // SimpleExpression'
  set(NonTerm::SimpleExpressionPrime, Term::Semicolon, 39);
  set(NonTerm::SimpleExpressionPrime, Term::End, 39);
  set(NonTerm::SimpleExpressionPrime, Term::Else, 39);
  set(NonTerm::SimpleExpressionPrime, Term::AddOp, 38);
  set(NonTerm::SimpleExpressionPrime, Term::Then, 39);
  set(NonTerm::SimpleExpressionPrime, Term::Do, 39);
  set(NonTerm::SimpleExpressionPrime, Term::RelOp, 39);
  set(NonTerm::SimpleExpressionPrime, Term::RightParen, 39);

  // Term
  set(NonTerm::Term, Term::Id, 40);
  set(NonTerm::Term, Term::Num, 40);
  set(NonTerm::Term, Term::LeftParen, 40);
  set(NonTerm::Term, Term::Not, 40);

  // Term'
  set(NonTerm::TermPrime, Term::Semicolon, 42);
  set(NonTerm::TermPrime, Term::End, 42);
  set(NonTerm::TermPrime, Term::Else, 42);
  set(NonTerm::TermPrime, Term::AddOp, 42);
  set(NonTerm::TermPrime, Term::Then, 42);
  set(NonTerm::TermPrime, Term::Do, 42);
  set(NonTerm::TermPrime, Term::RelOp, 42);
  set(NonTerm::TermPrime, Term::RightParen, 42);
  set(NonTerm::TermPrime, Term::MulOp, 41);

  // Factor
  set(NonTerm::Factor, Term::Id, 43);
  set(NonTerm::Factor, Term::Num, 44);
  set(NonTerm::Factor, Term::LeftParen, 45);
  set(NonTerm::Factor, Term::Not, 46);

  return table;
}();

// The right-hand side of each grammar production, stored in grammar order
inline const auto productions = [] {
  std::array<std::vector<Symbol>, NUM_PRODUCTIONS + 1> table{};

  // Slot zero is reserved for parsing errors. Empty productions are epsilon.
  table.at(1) = {NonTerm::Header, NonTerm::Declarations, NonTerm::Block,
                 Term::Period};
  table.at(2) = {Term::Program, Term::Id, Term::Semicolon};
  table.at(3) = {NonTerm::VariableDeclarationSection,
                 NonTerm::ProcedureDeclarations};
  table.at(4) = {Term::Var, NonTerm::VariableDeclarations};
  table.at(5) = {}; // VariableDeclarationSection ::= epsilon
  table.at(6) = {NonTerm::VariableDeclaration,
                 NonTerm::VariableDeclarationsPrime};
  table.at(7) = {NonTerm::VariableDeclaration,
                 NonTerm::VariableDeclarationsPrime};
  table.at(8) = {}; // VariableDeclarationsPrime ::= epsilon
  table.at(9) = {NonTerm::IdentifierList, Term::Colon, NonTerm::Type,
                 Term::Semicolon};
  table.at(10) = {Term::Id, NonTerm::IdentifierListPrime};
  table.at(11) = {Term::Comma, Term::Id, NonTerm::IdentifierListPrime};
  table.at(12) = {}; // IdentifierListPrime ::= epsilon
  table.at(13) = {Term::Integer};
  table.at(14) = {Term::Real};
  table.at(15) = {NonTerm::ProcedureHeader, NonTerm::Declarations,
                  NonTerm::Block, Term::Semicolon};
  table.at(16) = {Term::Procedure, Term::Id, Term::Semicolon};
  table.at(17) = {Term::Begin, NonTerm::Statements, Term::End};
  table.at(18) = {NonTerm::Statement, NonTerm::StatementsPrime};
  table.at(19) = {Term::Semicolon, NonTerm::Statement,
                  NonTerm::StatementsPrime};
  table.at(20) = {}; // StatementsPrime ::= epsilon
  table.at(21) = {Term::Id, NonTerm::IdStatement};
  table.at(22) = {NonTerm::Block};
  table.at(23) = {Term::If, NonTerm::Expression, Term::Then, NonTerm::Statement,
                  NonTerm::ElseClause};
  table.at(24) = {Term::While, NonTerm::Expression, Term::Do,
                  NonTerm::Statement};
  table.at(25) = {}; // Statement ::= epsilon
  table.at(26) = {Term::Assign, NonTerm::Expression};
  table.at(27) = {Term::LeftParen, Term::RightParen};
  table.at(28) = {Term::Else, NonTerm::Statement};
  table.at(29) = {}; // ElseClause ::= epsilon
  table.at(30) = {NonTerm::Expression, NonTerm::ExpressionListPrime};
  table.at(31) = {Term::Comma, NonTerm::Expression,
                  NonTerm::ExpressionListPrime};
  table.at(32) = {}; // ExpressionListPrime ::= epsilon
  table.at(33) = {NonTerm::SimpleExpression, NonTerm::ExpressionPrime};
  table.at(34) = {Term::RelOp, NonTerm::SimpleExpression};
  table.at(35) = {}; // ExpressionPrime ::= epsilon
  table.at(36) = {NonTerm::Term, NonTerm::SimpleExpressionPrime};
  table.at(37) = {Term::AddOp, NonTerm::Term, NonTerm::SimpleExpressionPrime};
  table.at(38) = {Term::AddOp, NonTerm::Term, NonTerm::SimpleExpressionPrime};
  table.at(39) = {}; // SimpleExpressionPrime ::= epsilon
  table.at(40) = {NonTerm::Factor, NonTerm::TermPrime};
  table.at(41) = {Term::MulOp, NonTerm::Factor, NonTerm::TermPrime};
  table.at(42) = {}; // TermPrime ::= epsilon
  table.at(43) = {Term::Id};
  table.at(44) = {Term::Num};
  table.at(45) = {Term::LeftParen, NonTerm::Expression, Term::RightParen};
  table.at(46) = {Term::Not, NonTerm::Factor};
  // Extends the spreadsheet grammar so declarations can terminate at a block.
  table.at(47) = {}; // ProcedureDeclarations ::= epsilon

  return table;
}();

}; // namespace

Parser::Parser(const Scanner &scanner, const SymbolTable &symbol_table)
    : m_scanner(scanner), m_symbol_table(symbol_table),
      m_tokens(scanner.tokens()) {}

void Parser::parse() {
  if (m_tokens.empty()) {
    throw std::length_error(
        "pascalc - \033[31merror\033[0m: input cannot be empty on line 1");
  }

  std::size_t pos = 0;
  // Begin parsing with the grammar's start symbol
  std::vector<Symbol> stack{NonTerm::Program};

  while (!stack.empty()) {
    if (pos == m_tokens.size()) {
      throw std::logic_error(
          "pascalc - \033[31merror\033[0m: unexpected end of input after token on line " +
          std::to_string(m_tokens.back().line_number));
    }

    const Token &token = m_tokens.at(pos);
    const auto converted_token_type = convert_token_type(token.type);

    if (converted_token_type == Term::Error) {
      throw std::logic_error(
          "pascalc - \033[31merror\033[0m: illegal token type for lexeme: \n" +
          token.lexeme + " on line " + std::to_string(token.line_number));
    }

    const auto symbol = stack.back();

    stack.pop_back();

    if (std::holds_alternative<NonTerm>(symbol)) {
      // Select a production using the non-terminal and current lookahead
      const NonTerm non_term = std::get<NonTerm>(symbol);
      const std::size_t production_num =
          production_table.at(static_cast<size_t>(non_term))
              .at(static_cast<size_t>(converted_token_type));
      if (production_num == 0) {
        throw std::logic_error(
            "pascalc - \033[31merror\033[0m: no production for " +
            std::string(nonterm_names.at(static_cast<std::size_t>(non_term))) +
            " with lookahead: " + token.lexeme + " on line " +
            std::to_string(token.line_number));
      }
      const auto &production_set = productions.at(production_num);

      // Push in reverse order so the leftmost symbol is processed first.
      // Empty productions add nothing to the stack, representing epsilon
      for (const auto &p : std::views::reverse(production_set)) {
        stack.emplace_back(p);
      }

      std::cout << nonterm_names.at(static_cast<std::size_t>(non_term)) << "\n";

    } else {
      const Term term = std::get<Term>(symbol);

      if (term == converted_token_type) {
        // A matched terminal consumes one token from the stream
        ++pos;
        std::cout << term_names.at(static_cast<std::size_t>(term)) << "\n";
      } else {
        throw std::logic_error(
            "pascalc - \033[31merror\033[0m: illegal token type for lexeme: " +
            token.lexeme + " on line " + std::to_string(token.line_number) +
            "\nexpected: " +
            static_cast<std::string>(
                term_names.at(static_cast<size_t>(std::get<Term>(symbol)))));
      }
    }
  }

  // A complete parse must consume the entire tokenized stream
  if (pos != m_tokens.size()) {
    throw std::logic_error(
        "pascalc - \033[31merror\033[0m: unexpected trailing token: " +
        m_tokens.at(pos).lexeme + " on line " +
        std::to_string(m_tokens.at(pos).line_number));
  }
}

Term Parser::convert_token_type(TokenType token_type) {
  switch (token_type) {
  case TokenType::TOK_PROGRAM:
    return Term::Program;
  case TokenType::TOK_PROCEDURE:
    return Term::Procedure;
  case TokenType::TOK_VAR:
    return Term::Var;
  case TokenType::TOK_BEGIN:
    return Term::Begin;
  case TokenType::TOK_END:
    return Term::End;
  case TokenType::TOK_IF:
    return Term::If;
  case TokenType::TOK_THEN:
    return Term::Then;
  case TokenType::TOK_ELSE:
    return Term::Else;
  case TokenType::TOK_WHILE:
    return Term::While;
  case TokenType::TOK_DO:
    return Term::Do;
  case TokenType::TOK_INTEGER:
    return Term::Integer;
  case TokenType::TOK_REAL:
    return Term::Real;

  case TokenType::TOK_EQUAL:
  case TokenType::TOK_GREATER:
  case TokenType::TOK_LESS:
  case TokenType::TOK_GREATER_EQUAL:
  case TokenType::TOK_LESS_EQUAL:
  case TokenType::TOK_NOT_EQUAL:
    return Term::RelOp;

  case TokenType::TOK_NOT:
    return Term::Not;

  case TokenType::TOK_PLUS:
  case TokenType::TOK_MINUS:
  case TokenType::TOK_OR:
    return Term::AddOp;

  case TokenType::TOK_MULTIPLY:
  case TokenType::TOK_DIVIDE:
  case TokenType::TOK_DIV:
  case TokenType::TOK_MOD:
  case TokenType::TOK_AND:
    return Term::MulOp;

  case TokenType::TOK_ASSIGN:
    return Term::Assign;
  case TokenType::TOK_IDENTIFIER:
    return Term::Id;
  case TokenType::TOK_NUMBER:
    return Term::Num;
  case TokenType::TOK_LEFT_PAREN:
    return Term::LeftParen;
  case TokenType::TOK_RIGHT_PAREN:
    return Term::RightParen;
  case TokenType::TOK_COMMA:
    return Term::Comma;
  case TokenType::TOK_SEMICOLON:
    return Term::Semicolon;
  case TokenType::TOK_COLON:
    return Term::Colon;
  case TokenType::TOK_PERIOD:
    return Term::Period;
  case TokenType::TOK_WORD:
    return Term::Error;
  }
  return Term::Error;
}
