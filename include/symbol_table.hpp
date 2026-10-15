#pragma once

#include "token.hpp"
#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

enum class SymbolType { Keyword, Operator, Program, Variable, Procedure };

enum class DataType { Unknown, None, Integer, Real };

struct PredefinedSymbol {
  std::string_view lexeme;
  TokenType token_type;
  SymbolType symbol_type;
};

constexpr std::size_t NUM_PREDEFINED = 34;

// List of predefined symbols that are installed into the symbol
// table via its constructor
inline constexpr std::array<PredefinedSymbol, NUM_PREDEFINED> predefined_symbols{{
    {"PROGRAM", TokenType::TOK_PROGRAM, SymbolType::Keyword},
    {"PROCEDURE", TokenType::TOK_PROCEDURE, SymbolType::Keyword},
    {"VAR", TokenType::TOK_VAR, SymbolType::Keyword},
    {"BEGIN", TokenType::TOK_BEGIN, SymbolType::Keyword},
    {"END", TokenType::TOK_END, SymbolType::Keyword},
    {"IF", TokenType::TOK_IF, SymbolType::Keyword},
    {"THEN", TokenType::TOK_THEN, SymbolType::Keyword},
    {"ELSE", TokenType::TOK_ELSE, SymbolType::Keyword},
    {"WHILE", TokenType::TOK_WHILE, SymbolType::Keyword},
    {"DO", TokenType::TOK_DO, SymbolType::Keyword},
    {"INTEGER", TokenType::TOK_INTEGER, SymbolType::Keyword},
    {"REAL", TokenType::TOK_REAL, SymbolType::Keyword},
    {"=", TokenType::TOK_EQUAL, SymbolType::Operator},
    {">", TokenType::TOK_GREATER, SymbolType::Operator},
    {"<", TokenType::TOK_LESS, SymbolType::Operator},
    {">=", TokenType::TOK_GREATER_EQUAL, SymbolType::Operator},
    {"<=", TokenType::TOK_LESS_EQUAL, SymbolType::Operator},
    {"<>", TokenType::TOK_NOT_EQUAL, SymbolType::Operator},
    {"NOT", TokenType::TOK_NOT, SymbolType::Operator},
    {"+", TokenType::TOK_PLUS, SymbolType::Operator},
    {"-", TokenType::TOK_MINUS, SymbolType::Operator},
    {"OR", TokenType::TOK_OR, SymbolType::Operator},
    {"*", TokenType::TOK_MULTIPLY, SymbolType::Operator},
    {"/", TokenType::TOK_DIVIDE, SymbolType::Operator},
    {"DIV", TokenType::TOK_DIV, SymbolType::Operator},
    {"MOD", TokenType::TOK_MOD, SymbolType::Operator},
    {"AND", TokenType::TOK_AND, SymbolType::Operator},
    {":=", TokenType::TOK_ASSIGN, SymbolType::Operator},
    {"(", TokenType::TOK_LEFT_PAREN, SymbolType::Operator},
    {")", TokenType::TOK_RIGHT_PAREN, SymbolType::Operator},
    {",", TokenType::TOK_COMMA, SymbolType::Operator},
    {";", TokenType::TOK_SEMICOLON, SymbolType::Operator},
    {":", TokenType::TOK_COLON, SymbolType::Operator},
    {".", TokenType::TOK_PERIOD, SymbolType::Operator},
}};

// An entry in the name table
struct NameEntry {
  std::optional<std::size_t> current_attribute_index;
};

// An entry in the attribute table
struct AttributeEntry {
  TokenType token_type;
  SymbolType symbol_type;
  DataType data_type;
  std::optional<std::size_t> outer_attribute_index;
};

// A scope that is pushed onto the scope stack
struct Scope {
  std::vector<std::string> declared_names;
};

class SymbolTable {
public:
  /**
   * @brief This class represents a symbol table. It contains
   * functions related to declaring, installing, and finding
   * symbols in the symbol table
   */
  SymbolTable();

  /**
   * @brief Declares an attribute in the current scope.
   *
   * @details Normalizes the name of the symbol to install into the name
   * table, and then installs it into the attribute table if and only if it
   * was not already declared in the current scope
   *
   * @param &name The name of the symbol to be installed into the name table
   * @param symbol_type The symbol type of the symbol
   * @param data_type The data type of the symbol
   *
   * @return True if the name is not already declared in the current scope,
   * false otherwise
   */
  bool declare(const std::string_view &name, SymbolType symbol_type,
               DataType data_type);

  /**
   * @brief Performs a lookup in the name table to find an associated
   * attribute table index
   *
   * @param &name The name of the symbol to perform a lookup on
   *
   * @return The index of the name in the attribute table, nullopt otherwise
   */
  std::optional<std::size_t> lookup(const std::string_view &name) const;

  /**
   * @brief Performs a lookup in the attribute table to check
   * for a predefined token
   *
   * @param &lexeme The lexeme to lookup in the attribute table to check if
   * it is a predefined token
   *
   * @return The TokenType of the predefined token if it exists in the
   * attribute table. nullopt if it either does not exist in the attribute
   * table, or if the symbol type is not Keyword or Operator
   *
   * @see lookup(const std::string &name)
   */
  std::optional<TokenType>
  lookup_predefined_token(const std::string_view &lexeme) const;

  /**
   * @brief Retrieves the attribute entry at a given attribute index
   *
   * @param attribute_index The index of the desired attribute entry
   *
   * @return A reference to the AttributeEntry at the given attribute index
   *
   * @note Pre-condition: attribute_index points to a valid index in the
   * attribute table vector
   */
  const AttributeEntry &get_attribute(std::size_t attribute_index) const;

  /**
   * @brief Enters a new scope by adding it to the scope stack
   *
   * @see exit_scope()
   */
  void enter_scope();

  /**
   * @brief Exits the current scope.
   *
   * @details Iterates through each declared name in the current scope and
   * checks if it has an index in the attribute table. Restores its outer
   * declaration if it exists, or no declaration if none existed
   *
   * @throws std::logic_error Throws if attempting to exit the global scope,
   * which should never happen, or if a symbol has no active entry in the
   * attribute table
   *
   * @see enter_scope()
   */
  void exit_scope();

private:
  std::unordered_map<std::string, NameEntry> m_name_table;
  std::vector<AttributeEntry> m_attribute_table;
  std::vector<Scope> m_scope_stack; // Acts as the auxiliary table

  /**
   * @brief Normalizes a name (string) to be all uppercase
   *
   * @return The normalized name
   */
  static std::string normalize_name(const std::string_view &name);

  /**
   * @brief Installs a name into the name table.
   *
   * @return A reference to the newly installed name if successfully installed,
   * otherwise a reference to the name if it already exists in the name table
   */
  NameEntry &install_name(const std::string &normalized_name);

  /**
   * @brief Installs an attribute into the attribute table
   *
   * @details Installs a normalized name into the name table,
   * optionally retrieves its outer index if the name already exists in a 
   * different scope, and then installs the AttributeEntry into the attribute
   * table. Upon successful installation, pushes the name to the current stack's
   * list of declared names
   *
   * @return The attribute table index of the newly installed attribute
   */
  std::size_t install_attribute(const std::string &normalized_name, SymbolType symbol_type,
                                DataType data_type, TokenType token_type);

  /**
   * @brief Determines if a given name is already declared in the current
   * scope
   *
   * @return True if it already exists in the current scope, False otherwise
   */
  bool declared_in_current_scope(const std::string_view &normalized_name) const;
};
