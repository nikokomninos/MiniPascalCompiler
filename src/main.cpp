#include "parser.hpp"
#include "scanner.hpp"
#include "symbol_table.hpp"
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc == 1) {
    std::cerr << "pascalc - \033[31merror\033[0m: no input file\n";
    return 1;
  }

  if (argc > 2) {
    std::cerr << "pascalc - \033[31merror\033[0m: too many arguments\n";
    return 1;
  }

  std::ifstream file(argv[1]);

  if (!file.is_open()) {
    std::cerr << "pascalc - \033[31merror\033[0m: failed to open input file\n";
    return 1;
  }

  // Reads the source code file into a string using an input stream buffer
  // iterator, which automatically closes when reaching EOF
  std::string src{(std::istreambuf_iterator<char>(file)),
                  std::istreambuf_iterator<char>()};

  file.close();

  SymbolTable symbol_table;
  Scanner scanner(src, symbol_table);

  scanner.tokenize_stream();
  // scanner.print_token_stream();

  Parser parser(scanner, symbol_table);

  try {
    parser.parse();
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
  }

  /*--- Symbol Table Tests --- */

  /*
  std::cout << "\n";

  // Show all pre-defined symbols are installed upon creation of the
  // symbol table
  for (const auto &p : predefined_symbols) {
    const auto index{symbol_table.lookup(p.lexeme)};
    if (index.has_value()) {
      std::cout << std::left << std::setw(15) << p.lexeme
                << ": Attribute Index " << index.value() << "\n";
    }
  }

  std::cout << "\n";

  // Test declaration of symbol in global scope
  symbol_table.declare("x", SymbolType::Variable, DataType::Integer);

  const auto outer_x_index{symbol_table.lookup("x")};
  if (outer_x_index.has_value()) {
    std::cout << "global x index: " << outer_x_index.value() << "\n";
  } else {
    std::cout << "global x index: out of scope\n";
  }

  symbol_table.enter_scope();

  std::cout << "New scope entered\n";

  // Test declaration of local variable in new scope
  symbol_table.declare("y", SymbolType::Variable, DataType::Integer);

  const auto inner_y_index{symbol_table.lookup("y")};
  if (inner_y_index.has_value()) {
    std::cout << "local y index: " << inner_y_index.value() << "\n";
  } else {
    std::cout << "local y index: out of scope\n";
  }

  // Test declaration of local variable in new scope that shares name with a
  // variable in the outer scope (in this case, the global scope)
  symbol_table.declare("x", SymbolType::Variable, DataType::Integer);
  const auto inner_x_index{symbol_table.lookup("x")};
  if (inner_x_index.has_value()) {
    std::cout << "local x index: " << inner_x_index.value() << "\n";
  } else {
    std::cout << "local x index: out of scope\n";
  }

  symbol_table.exit_scope();

  std::cout << "New scope exited\n";

  // Test looking up the name of the variable that shared a name between scopes
  const auto new_outer_x_index{symbol_table.lookup("x")};
  if (new_outer_x_index.has_value()) {
    std::cout << "global x index: " << new_outer_x_index.value() << "\n";
  } else {
    std::cout << "global x index: out of scope\n";
  }

  // Test looking up the name of the local variable in the new scope that no
  // longer exists
  const auto new_outer_y_index{symbol_table.lookup("y")};
  if (new_outer_y_index.has_value()) {
    std::cout << "global y index: " << new_outer_y_index.value() << "\n";
  } else {
    std::cout << "global y index: out of scope\n";
  }

  std::cout << "\n";
  */

  return 0;
}
