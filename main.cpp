#include <librapid/librapid.hpp>

#include "include/symbomath.hpp"
#include <memory>
#include <functional>
#include <utility>

namespace lrc = librapid;

using Scalar = lrc::mpfr; // Type used in all calculations
inline constexpr int64_t formatWidth = 15;

static inline constexpr uint64_t TYPE_VARIABLE = 1ULL << 63;
static inline constexpr uint64_t TYPE_OPERATOR = 1ULL << 62;

// Tokens
static inline constexpr uint64_t TYPE_DIGIT	 = 1ULL << 0;
static inline constexpr uint64_t TYPE_CHAR	 = 1ULL << 1;
static inline constexpr uint64_t TYPE_ADD	 = 1ULL << 2;
static inline constexpr uint64_t TYPE_SUB	 = 1ULL << 3;
static inline constexpr uint64_t TYPE_MUL	 = 1ULL << 4;
static inline constexpr uint64_t TYPE_DIV	 = 1ULL << 5;
static inline constexpr uint64_t TYPE_CARET	 = 1ULL << 6;
static inline constexpr uint64_t TYPE_LPAREN = 1ULL << 7;
static inline constexpr uint64_t TYPE_RPAREN = 1ULL << 8;
static inline constexpr uint64_t TYPE_POINT	 = 1ULL << 9;

// High-level types
static inline constexpr uint64_t TYPE_NUMBER   = 1ULL << 10;
static inline constexpr uint64_t TYPE_STRING   = 1ULL << 11;
static inline constexpr uint64_t TYPE_FUNCTION = 1ULL << 12;

std::string tokenToString(uint64_t tok) {
	// Remove high-level specifiers
	tok &= ~TYPE_VARIABLE;
	tok &= ~TYPE_OPERATOR;

	if (!tok || tok == (uint64_t)-1) return "NONE";
	if (tok & TYPE_DIGIT) return "DIGIT";
	if (tok & TYPE_CHAR) return "CHAR";
	if (tok & TYPE_ADD) return "ADD";
	if (tok & TYPE_SUB) return "SUB";
	if (tok & TYPE_MUL) return "MUL";
	if (tok & TYPE_DIV) return "DIV";
	if (tok & TYPE_CARET) return "CARET";
	if (tok & TYPE_LPAREN) return "LPAREN";
	if (tok & TYPE_RPAREN) return "RPAREN";
	if (tok & TYPE_POINT) return "POINT";
	if (tok & TYPE_NUMBER) return "NUMBER";
	if (tok & TYPE_FUNCTION) return "FUNCTION"; // Check this before STRING
	if (tok & TYPE_STRING) return "STRING";
	return "UNKNOWN";
}

int64_t precedence(const uint64_t type) {
	if (type & TYPE_ADD || type & TYPE_SUB) return 1;
	if (type & TYPE_MUL || type & TYPE_DIV) return 2;
	if (type & TYPE_CARET) return 3;
	if (type & TYPE_FUNCTION) return 4;
	return 0;
}

/**
 * The most fundamental type. All numbers, functions, variables, etc. inherit
 * from this.
 */
class Component {
public:
	Component() = default;

	LR_NODISCARD("") virtual Scalar eval() const {
		LR_ASSERT(false,
				  "{} object cannot be evaluated (numerically) directly",
				  type());
		return 0;
	}

	LR_NODISCARD("")
	virtual const std::vector<std::shared_ptr<Component>> &tree() const {
		LR_ASSERT(false,
				  "Only a value of type Tree is allowed to use this function. "
				  "Type of *this is {}",
				  type());
		return m_tmpTree;
	}

	LR_NODISCARD("") virtual std::vector<std::shared_ptr<Component>> &tree() {
		LR_ASSERT(false,
				  "Only a value of type Tree is allowed to use this function. "
				  "Type of *this is {}",
				  type());
		return m_tmpTree;
	}

	LR_NODISCARD("") virtual std::string str(uint64_t indent) const {
		return fmt::format("{: >{}}{}", "", indent, "NONE");
	}

	LR_NODISCARD("") virtual std::string name() const {
		return "BUILT_IN_COMPONENT_TYPE";
	}

	LR_NODISCARD("")
	virtual std::string repr(uint64_t indent, uint64_t typeWidth,
							 uint64_t valWidth) const {
		return fmt::format("{: >{}}[ {:^{}} ] [ {:^{}} ]",
						   "",
						   indent,
						   type(),
						   typeWidth,
						   str(0),
						   valWidth);
	}

	LR_NODISCARD("") virtual std::string type() const { return "COMPONENT"; }

private:
	std::vector<std::shared_ptr<Component>>
	  m_tmpTree; // Empty value to return in tree()
};

class Number : public Component {
public:
	Number() : Component() {}
	explicit Number(const Scalar &value) : Component(), m_value(value) {}

	explicit Number(const std::string &value) : Component() {
		scn::scan(value, "{}", m_value);
	}

	LR_NODISCARD("") Scalar eval() const override { return m_value; }

	LR_NODISCARD("") std::string str(uint64_t indent) const override {
		return fmt::format("{: >{}}{}", "", indent, m_value);
	}

	LR_NODISCARD("") std::string name() const override {
		return "BUILT_IN_NUMBER_TYPE";
	}

	LR_NODISCARD("") std::string type() const override { return "NUMBER"; }

private:
	Scalar m_value = 0;
};

class Variable : public Component {
public:
	Variable() : Component() {}
	explicit Variable(std::string name) :
			Component(), m_name(std::move(name)) {}

	LR_NODISCARD("") std::string str(uint64_t indent) const override {
		return m_name;
	}

	LR_NODISCARD("") std::string name() const override { return m_name; }

	LR_NODISCARD("") std::string type() const override { return "VARIABLE"; }

private:
	std::string m_name = "NONAME";
};

class Tree : public Component {
public:
	Tree() : Component() {}

	LR_NODISCARD("") Scalar eval() const override { return m_tree[0]->eval(); }

	LR_NODISCARD("")
	const std::vector<std::shared_ptr<Component>> &tree() const override {
		return m_tree;
	}

	LR_NODISCARD("") std::vector<std::shared_ptr<Component>> &tree() override {
		return m_tree;
	}

	LR_NODISCARD("") std::string str(uint64_t indent) const override {
		// Format stuff really nicely :)
		uint64_t longestType = 0, longestValue = 0;
		for (const auto &val : m_tree) {
			longestType	 = lrc::max(longestType, val->type().length());
			longestValue = lrc::max(longestValue, val->str(0).length());
		}

		std::string res = fmt::format("{: >{}}[ TREE ]", "", indent);

		for (const auto &val : m_tree)
			res += fmt::format(
			  "\n{}", val->repr(indent + 4, longestType, longestValue));

		return res;
	}

	LR_NODISCARD("") virtual std::string name() const {
		return "BUILT_IN_TREE_TYPE";
	}

	LR_NODISCARD("") std::string type() const override { return "TREE"; }

private:
	std::vector<std::shared_ptr<Component>> m_tree;
};

class Function : public Component {
public:
	Function() : Component() {}

	Function(const Function &other) = default;

	explicit Function(
	  std::string name, std::string format,
	  std::function<Scalar(const std::vector<Scalar> &)> functor,
	  uint64_t numOperands,
	  std::vector<std::shared_ptr<Component>> values = {}) :
			Component(),
			m_name(std::move(name)), m_format(std::move(format)),
			m_functor(std::move(functor)), m_numOperands(numOperands),
			m_values(std::move(values)) {}

	LR_NODISCARD("") Scalar eval() const override {
		std::vector<Scalar> operands;
		for (const auto &val : m_values) operands.push_back(val->eval());
		return m_functor(operands);
	}

	LR_NODISCARD("") std::string str(uint64_t indent) const override {
		return fmt::format("{: >{}}{}", "", indent, m_name);
	}

	LR_NODISCARD("")
	std::string repr(uint64_t indent, uint64_t typeWidth,
					 uint64_t valWidth) const override {
		std::string res = fmt::format("{: >{}}[ {:^{}} ] [ {:^{}} ]",
									  "",
									  indent,
									  type(),
									  typeWidth,
									  str(0),
									  valWidth);

		// Format stuff really nicely :)
		uint64_t longestType = 0, longestValue = 0;
		for (const auto &val : m_values) {
			longestType	 = lrc::max(longestType, val->type().length());
			longestValue = lrc::max(longestValue, val->str(0).length());
		}

		for (const auto &val : m_values)
			res += fmt::format(
			  "\n{}", val->repr(indent + 4, longestType, longestValue));

		return res;
	}

	LR_NODISCARD("") uint64_t numOperands() const { return m_numOperands; }

	LR_NODISCARD("") auto values() const { return m_values; }

	void addValue(const std::shared_ptr<Component> &value) {
		m_values.push_back(value);
	}

	LR_NODISCARD("") std::string name() const override { return m_name; }

	LR_NODISCARD("") std::string type() const override { return "FUNCTION"; }

	LR_NODISCARD("") std::string format() const { return m_format; }

private:
	std::string m_name	 = "NULLOP";
	std::string m_format = "NULLOP";
	std::function<Scalar(const std::vector<Scalar> &)> m_functor;
	uint64_t m_numOperands = 0;

	std::vector<std::shared_ptr<Component>> m_values = {};
};

// Registered functions
static inline std::vector<std::shared_ptr<Function>> functions;

auto findFunction(const std::string &name) {
	// This should probably be a hash-map
	return std::find_if(functions.begin(),
						functions.end(),
						[&](const auto &val) { return val->name() == name; });
}

struct Token {
	uint64_t type;
	char val;
};

std::vector<Token> tokenize(const std::string &input) {
	/*
	 * Valid objects:
	 *
	 * 0-9: Numbers
	 * a-z | A-Z: Characters
	 * +: Addition
	 * -: Subtraction
	 * *: Multiplication
	 * /: Division
	 * ^: Exponentiation
	 * (: Left Parenthesis
	 * ): Right Parenthesis
	 * .: Point
	 */

	std::vector<Token> res;

	// Scan the input string
	for (const auto &c : input) {
		if ('0' <= c && c <= '9')
			res.emplace_back(Token {TYPE_DIGIT, c});
		else if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
			res.emplace_back(Token {TYPE_CHAR, c});
		else if (c == '+')
			res.emplace_back(Token {TYPE_ADD | TYPE_OPERATOR, c});
		else if (c == '-')
			res.emplace_back(Token {TYPE_SUB | TYPE_OPERATOR, c});
		else if (c == '*')
			res.emplace_back(Token {TYPE_MUL | TYPE_OPERATOR, c});
		else if (c == '/')
			res.emplace_back(Token {TYPE_DIV | TYPE_OPERATOR, c});
		else if (c == '^')
			res.emplace_back(Token {TYPE_CARET | TYPE_OPERATOR, c});
		else if (c == '(')
			res.emplace_back(Token {TYPE_LPAREN, c});
		else if (c == ')')
			res.emplace_back(Token {TYPE_RPAREN, c});
		else if (c == '.')
			res.emplace_back(Token {TYPE_POINT, c});
		else if (c == ' ')
			continue;
		else
			LR_ASSERT(false, "Unknown token '{}'", c);
	}

	return res;
}

struct Lexed {
	uint64_t type;
	std::string val;
};

// Take a list of tokens and return a list of lexed objects
std::vector<Lexed> lexer(const std::vector<Token> &tokens) {
	/*
	 * Grammar:
	 *
	 * <digit> ::= 0-9
	 * <character> ::= a-z | A-Z
	 * <number> ::= <digit>+ | <digit>+ "." <digit>+
	 * <string> ::= <character>+
	 * <operator> ::= + | - | * | / | ^
	 * <parenthesis> ::= ( | )
	 */

	std::vector<Lexed> res;

	// Stores the current value
	std::string currentLex;

	// Valid next characters that would continue the string
	uint64_t validNext;
	uint64_t type = -1;

	// Iterate over all tokens, "eating" them to form a list of lexed objects
	auto it = tokens.begin();
	while (it != tokens.end()) {
		// If currentLex is empty, append value
		if (currentLex.empty() || it->type & validNext) {
			// Next type can be appended because:
			// 1. current string is empty
			// 2. it's a valid next character
			currentLex += it->val;
		} else {
			// Invalid to append, so cache current result
			// and reset
			Lexed lexed {type, currentLex};
			res.emplace_back(lexed);
			currentLex = it->val;
		}

		if (it->type & TYPE_DIGIT) {
			validNext = TYPE_DIGIT | TYPE_POINT;
			type	  = TYPE_NUMBER | TYPE_VARIABLE;
		} else if (it->type & TYPE_POINT) {
			validNext = TYPE_DIGIT;
			type	  = TYPE_NUMBER | TYPE_VARIABLE;
		} else if (it->type & TYPE_CHAR) {
			validNext = TYPE_CHAR;
			type	  = TYPE_STRING | TYPE_VARIABLE;
		} else {
			validNext = 0;
			type	  = it->type;
		}

		++it;
	}

	// Add whatever is left in currentLex
	Lexed lexed {type, currentLex};
	res.emplace_back(lexed);

	return res;
}

std::vector<Lexed> process(const std::vector<Lexed> &lexed) {
	/*
	 * Apply certain rules:
	 *
	 * <number> <lparen> ::= <number> "*" <lparen>
	 * <number> <string> ::= <number> "*" <string>
	 */

	std::vector<Lexed> res;

	for (auto it = lexed.begin(); it != lexed.end() - 1; ++it) {
		if (it->type & TYPE_NUMBER && (it + 1)->type & TYPE_LPAREN) {
			res.emplace_back(*it);
			res.emplace_back(Lexed {TYPE_MUL | TYPE_OPERATOR, "*"});
		} else if (it->type & TYPE_STRING && (it + 1)->type & TYPE_LPAREN) {
			// Check the value is not a function
			if (std::find_if(
				  functions.begin(), functions.end(), [&](const auto &f) {
					  return f->name() == it->val;
				  }) == functions.end()) {
				res.emplace_back(*it);
				res.emplace_back(Lexed {TYPE_MUL | TYPE_OPERATOR, "*"});
			} else {
				// It's a function, so mark it as such
				res.emplace_back(Lexed {it->type | TYPE_FUNCTION, it->val});
			}
		} else {
			res.emplace_back(*it);
		}
	}

	res.emplace_back(lexed.back());

	return res;
}

std::vector<Lexed> toPostfix(const std::vector<Lexed> &processed) {
	std::vector<Lexed> postfix;
	std::vector<Lexed> stack;

	for (auto lex = processed.begin(); lex != processed.end(); ++lex) {
		if (lex->type & TYPE_VARIABLE) { // Number or string
			postfix.emplace_back(*lex);
		} else if (lex->type & (TYPE_OPERATOR | TYPE_FUNCTION)) {
			// Pop operators with higher precedence than the current one
			while (!stack.empty() &&
				   precedence(stack.back().type) >= precedence(lex->type)) {
				postfix.emplace_back(stack.back());
				stack.pop_back();
			}

			// Push the current operator onto the stack
			stack.emplace_back(*lex);
		} else if (lex->type & TYPE_LPAREN) {
			stack.emplace_back(*lex);
		} else if (lex->type & TYPE_RPAREN) {
			// Pop all operators until the matching LPAREN
			while (stack.back().type != TYPE_LPAREN) {
				postfix.emplace_back(stack.back());
				stack.pop_back();
			}
			// Remove the LPAREN
			stack.pop_back();
		}
	}

	// Pop the remaining operators on the stack
	while (!stack.empty()) {
		postfix.emplace_back(stack.back());
		stack.pop_back();
	}

	return postfix;
}

std::vector<std::shared_ptr<Component>>
parse(const std::vector<Lexed> &postfix) {
	// Convert all lexed objects in postfix into value types
	std::vector<std::shared_ptr<Component>> res;

	for (const auto &lex : postfix) {
		if (lex.type & TYPE_NUMBER) {
			res.emplace_back(std::make_shared<Number>(lex.val));
		} else if (lex.type & TYPE_STRING) {
			// Check for a function
			auto func = findFunction(lex.val);
			if (func != functions.end()) {
				// Function was found, add it to result
				res.emplace_back(*func);
			} else {
				// Not a function. Use as a variable
				res.emplace_back(std::make_shared<Variable>(lex.val));
			}
		} else if (lex.type & TYPE_OPERATOR) {
			// Operators are just special functions
			auto func = findFunction("_");
			if (lex.type & TYPE_ADD) func = findFunction("ADD");
			if (lex.type & TYPE_SUB) func = findFunction("SUB");
			if (lex.type & TYPE_MUL) func = findFunction("MUL");
			if (lex.type & TYPE_DIV) func = findFunction("DIV");
			if (lex.type & TYPE_CARET) func = findFunction("POW");

			LR_ASSERT(func != functions.end(), "Operator not found");

			res.emplace_back(*func);
		}
	}

	return res;
}

std::shared_ptr<Tree>
genTree(const std::vector<std::shared_ptr<Component>> &values) {
	// Construct a tree from the processed list
	auto res = std::make_shared<Tree>();
	std::vector<std::shared_ptr<Component>> stack;

	for (const auto &lex : values) {
		if (lex->type() == "NUMBER") {
			stack.emplace_back(lex);
		} else if (lex->type() == "FUNCTION") {
			auto funcCast = std::dynamic_pointer_cast<Function>(lex);
			std::vector<std::shared_ptr<Component>> args;
			for (uint64_t i = 0; i < funcCast->numOperands(); ++i) {
				args.emplace_back(stack.back());
				stack.pop_back();
			}

			// Function is valid -- clone it
			auto node = std::make_shared<Function>(
			  *std::dynamic_pointer_cast<Function>(lex));

			for (auto it = args.rbegin(); it != args.rend(); ++it) {
				node->addValue(*it);
			}

			// Push the node back onto the stack
			stack.emplace_back(node);
		}
	}

	// Push the result to the tree
	res->tree().emplace_back(stack.back());

	return res;
}

void registerFunctions() {
	// Add the addition operator
	functions.emplace_back(std::make_shared<Function>(
	  "ADD",
	  "{} + {}",
	  [](const std::vector<Scalar> &args) { return args[0] + args[1]; },
	  2));

	// Add the subtraction operator
	functions.emplace_back(std::make_shared<Function>(
	  "SUB",
	  "{} - {}",
	  [](const std::vector<Scalar> &args) { return args[0] - args[1]; },
	  2));

	// Add the multiplication operator
	functions.emplace_back(std::make_shared<Function>(
	  "MUL",
	  "{} * {}",
	  [](const std::vector<Scalar> &args) { return args[0] * args[1]; },
	  2));

	// Add the division operator
	functions.emplace_back(std::make_shared<Function>(
	  "DIV",
	  "{} / {}",
	  [](const std::vector<Scalar> &args) { return args[0] / args[1]; },
	  2));

	// Add the exponentiation operator
	functions.emplace_back(std::make_shared<Function>(
	  "POW",
	  "{} ^ {}",
	  [](const std::vector<Scalar> &args) {
		  return lrc::pow(args[0], args[1]);
	  },
	  2));

	// Add the function sin(x)
	functions.emplace_back(std::make_shared<Function>(
	  "sin",
	  "sin({})",
	  [](const std::vector<Scalar> &args) { return lrc::sin(args[0]); },
	  2));
}

std::string prettyPrint(const std::shared_ptr<Component> &object) {
	if (object->type() == "NUMBER")
		return lrc::str(std::dynamic_pointer_cast<Number>(object)->eval());

	if (object->type() == "VARIABLE")
		return std::dynamic_pointer_cast<Variable>(object)->name();

	if (object->type() == "FUNCTION") {
		auto func	= std::dynamic_pointer_cast<Function>(object);
		auto format = func->format();
	}

	return "";
}

Scalar eval(const std::shared_ptr<Tree> &tree) {
	// Numerically evaluate the tree
	return tree->eval();
}

int main() {
	lrc::prec(1000);

	registerFunctions();

	std::string equation = "10 + 10";

	auto tokenized = tokenize(equation);
	auto lexed	   = lexer(tokenized);
	auto processed = process(lexed);
	auto postfix   = toPostfix(processed);
	auto parsed	   = parse(postfix);
	auto tree	   = genTree(parsed);

	// for (const auto &val : parse) { fmt::print("{}\n", val.val); }

	fmt::print("{}\n", tree->str(0));
	fmt::print("Numeric result: {}\n", lrc::str(eval(tree)));

	fmt::print("Tokenize: ");
	lrc::timeFunction([&]() { auto res = tokenize(equation); }, -1, -1, 2);

	fmt::print("Lex: ");
	lrc::timeFunction([&]() { auto res = lexer(tokenized); }, -1, -1, 2);

	fmt::print("Process: ");
	lrc::timeFunction([&]() { auto res = process(lexed); }, -1, -1, 2);

	fmt::print("Postfix: ");
	lrc::timeFunction([&]() { auto res = toPostfix(processed); }, -1, -1, 2);

	fmt::print("Parse: ");
	lrc::timeFunction([&]() { auto res = parse(postfix); }, -1, -1, 2);

	fmt::print("Eval: ");
	lrc::timeFunction([&]() { auto res = eval(tree); }, -1, -1, 2);

	return 0;
}
