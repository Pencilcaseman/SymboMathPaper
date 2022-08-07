#include <librapid/librapid.hpp>

#include "include/symbomath.hpp"
#include <memory>
#include <functional>
#include <utility>

namespace lrc = librapid;

inline constexpr int64_t formatWidth = 15;

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
static inline constexpr uint64_t TYPE_NUMBER = 1ULL << 10;
static inline constexpr uint64_t TYPE_STRING = 1ULL << 11;

std::string tokenToString(const uint64_t tok) {
	// Instead of bit-wise checks, ensure there are no other bits set
	// by subtracting and comparing to zero
	if (!tok) return "NONE";
	if (!(tok - TYPE_DIGIT)) return "DIGIT";
	if (!(tok - TYPE_CHAR)) return "CHAR";
	if (!(tok - TYPE_ADD)) return "ADD";
	if (!(tok - TYPE_SUB)) return "SUB";
	if (!(tok - TYPE_MUL)) return "MUL";
	if (!(tok - TYPE_DIV)) return "DIV";
	if (!(tok - TYPE_CARET)) return "CARET";
	if (!(tok - TYPE_LPAREN)) return "LPAREN";
	if (!(tok - TYPE_RPAREN)) return "RPAREN";
	if (!(tok - TYPE_POINT)) return "POINT";
	if (!(tok - TYPE_NUMBER)) return "NUMBER";
	if (!(tok - TYPE_STRING)) return "STRING";
	return "UNKNOWN";
}

/**
 * The most fundamental type. All numbers, functions, variables, etc. inherit
 * from this.
 */
class Component {
public:
	Component() = default;

	LR_NODISCARD("") virtual double eval() const {
		LR_ASSERT(false,
				  "{} object cannot be evaluated (numerically) directly",
				  type());
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
	explicit Number(double value) : Component(), m_value(value) {}

	LR_NODISCARD("") double eval() const override { return m_value; }

	LR_NODISCARD("") std::string str(uint64_t indent) const override {
		return fmt::format("{: >{}}{}", "", indent, m_value);
	}

	LR_NODISCARD("") std::string type() const override { return "NUMBER"; }

private:
	double m_value = 0;
};

class Variable : public Component {
public:
	Variable() : Component() {}
	explicit Variable(std::string name) :
			Component(), m_name(std::move(name)) {}

	LR_NODISCARD("") std::string str(uint64_t indent) const override {
		return m_name;
	}

	LR_NODISCARD("") std::string type() const override { return "VARIABLE"; }

private:
	std::string m_name = "NONAME";
};

class Tree : public Component {
public:
	Tree() : Component() {}

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

	LR_NODISCARD("") std::string type() const override { return "TREE"; }

private:
	std::vector<std::shared_ptr<Component>> m_tree;
};

class Function : public Component {
public:
	Function() : Component() {}

	explicit Function(
	  std::string name, std::string format,
	  std::function<double(const std::vector<double> &)> functor,
	  std::vector<std::shared_ptr<Component>> values) :
			Component(),
			m_name(std::move(name)), m_format(std::move(format)),
			m_functor(std::move(functor)), m_values(std::move(values)) {}

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

	LR_NODISCARD("") std::string type() const override { return "FUNCTION"; }

private:
	std::string m_name	 = "NULLOP";
	std::string m_format = "NULLOP";
	std::function<double(const std::vector<double> &)> m_functor;

	std::vector<std::shared_ptr<Component>> m_values = {};
};

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
			res.emplace_back(Token {TYPE_ADD, c});
		else if (c == '-')
			res.emplace_back(Token {TYPE_SUB, c});
		else if (c == '*')
			res.emplace_back(Token {TYPE_MUL, c});
		else if (c == '/')
			res.emplace_back(Token {TYPE_DIV, c});
		else if (c == '^')
			res.emplace_back(Token {TYPE_CARET, c});
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
	uint64_t type;

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
			type	  = TYPE_NUMBER;
		} else if (it->type & TYPE_POINT) {
			validNext = TYPE_DIGIT;
			type	  = TYPE_NUMBER;
		} else if (it->type & TYPE_CHAR) {
			validNext = TYPE_CHAR;
			type	  = TYPE_STRING;
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

std::shared_ptr<Tree> parse(const std::vector<Lexed> &lexed) {
	/*
	 * Grammar:
	 *
	 * <term> ::= 
	 */
}

int main() {
	fmt::print("Hello, World\n");

	auto tree = std::make_shared<Tree>();

	auto node1 = std::make_shared<Function>(
	  "ADD",
	  "{} + {}",
	  [](const std::vector<double> &args) { return args[0] + args[1]; },
	  std::vector<std::shared_ptr<Component>> {
		std::make_shared<Number>(123), std::make_shared<Variable>("x")});

	auto node2 = std::make_shared<Function>(
	  "ADD",
	  "{} + {}",
	  [](const std::vector<double> &args) { return args[0] + args[1]; },
	  std::vector<std::shared_ptr<Component>> {
		std::make_shared<Number>(456), std::make_shared<Variable>("y")});

	tree->tree().emplace_back(node1);
	tree->tree().emplace_back(node2);

	fmt::print("{}\n", tree->str(0));

	// for (const auto &val : tokenize("123.456 + xyz")) {
	// 	fmt::print("[ {:^6} ] {}\n", tokenToString(val.type), val.val);
	// }

	fmt::print("\n\n\n");

	for (const auto &val : lexer(tokenize("1 + 2 * (3 + 4) ^ 5"))) {
		fmt::print("[ {:^6} ] {}\n", tokenToString(val.type), val.val);
	}

	return 0;
}
