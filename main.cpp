#include <librapid/librapid.hpp>

#include <memory>
#include <functional>
#include <utility>

namespace lrc = librapid;

// #define SYMBOMATH_MULTIPRECISION

// Type used in all calculations
#if defined(SYMBOMATH_MULTIPRECISION)
using Scalar = lrc::mpfr; // MPFR multiprecision float
#else
using Scalar = double; // Normal 64bit float
#endif

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

// Unary +-
static inline constexpr uint64_t TYPE_PLUS	= 1ULL << 10;
static inline constexpr uint64_t TYPE_MINUS = 1ULL << 11;

// High-level types
static inline constexpr uint64_t TYPE_NUMBER   = 1ULL << 12;
static inline constexpr uint64_t TYPE_STRING   = 1ULL << 13;
static inline constexpr uint64_t TYPE_FUNCTION = 1ULL << 14;

// Object Statuses
static inline constexpr uint64_t STATUS_MOVED = 1ULL << 32;

int64_t precedence(const uint64_t type) {
	if (type & TYPE_ADD || type & TYPE_SUB) return 1;
	if (type & TYPE_MUL || type & TYPE_DIV) return 2;
	if (type & TYPE_PLUS || type & TYPE_MINUS) return 2;
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

	virtual void treeDepth(int64_t &depth) const {}

	LR_NODISCARD("")
	virtual Scalar eval() const {
		LR_ASSERT(false,
				  "{} object cannot be evaluated (numerically) directly",
				  type());
		return 0;
	}

	LR_NODISCARD("")
	virtual std::shared_ptr<Component> substitute(
	  const std::map<std::string, std::shared_ptr<Component>> &substitutions)
	  const {
		LR_ASSERT(false, "{} object cannot be substituted into", type());
		return nullptr;
	}

	LR_NODISCARD("")
	virtual std::shared_ptr<Component>
	differentiate(const std::string &wrt) const {
		LR_ASSERT(false, "{} object cannot be differentiated", type());
		return nullptr;
	}

	LR_NODISCARD("") virtual bool canEval() const { return false; }

	LR_NODISCARD("") virtual std::string str(uint64_t indent) const {
		return fmt::format("{:>{}}{}", "", indent, "NONE");
	}

	LR_NODISCARD("") virtual std::string name() const {
		return "BUILT_IN_COMPONENT_TYPE";
	}

	LR_NODISCARD("")
	virtual std::string repr(uint64_t indent, uint64_t typeWidth,
							 uint64_t valWidth) const {
		return fmt::format("{:>{}}[ {:^{}} ] [ {:^{}} ]",
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

class Tree : public Component {
public:
	Tree() : Component() {}

	LR_NODISCARD("")
	Scalar eval() const override { return m_tree[0]->eval(); }

	LR_NODISCARD("")
	std::shared_ptr<Component> substitute(
	  const std::map<std::string, std::shared_ptr<Component>> &substitutions)
	  const override {
		std::shared_ptr<Tree> res = std::make_shared<Tree>();
		res->tree().emplace_back(m_tree[0]->substitute(substitutions));
		return res;
	}

	LR_NODISCARD("") bool canEval() const override {
		return m_tree[0]->canEval();
	}

	LR_NODISCARD("")
	const std::vector<std::shared_ptr<Component>> &tree() const {
		return m_tree;
	}

	LR_NODISCARD("") std::vector<std::shared_ptr<Component>> &tree() {
		return m_tree;
	}

	void treeDepth(int64_t &depth) const override {
		m_tree[0]->treeDepth(depth);
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

class Number : public Component {
public:
	Number() : Component() {}
	explicit Number(const Scalar &value) : Component(), m_value(value) {}

	explicit Number(const std::string &value) : Component() {
		scn::scan(value, "{}", m_value);
	}

	void treeDepth(int64_t &depth) const override { ++depth; }

	LR_NODISCARD("")
	Scalar eval() const override { return m_value; }

	LR_NODISCARD("")
	std::shared_ptr<Component> substitute(
	  const std::map<std::string, std::shared_ptr<Component>> &substitutions)
	  const override {
		return std::make_shared<Number>(m_value);
	}

	LR_NODISCARD("") bool canEval() const override { return true; }

	LR_NODISCARD("") std::string str(uint64_t indent) const override {
		return fmt::format("{: >{}}{}", "", indent, m_value);
	}

	LR_NODISCARD("") std::string name() const override {
		return "BUILT_IN_NUMBER_TYPE";
	}

	LR_NODISCARD("") std::string type() const override { return "NUMBER"; }

	LR_NODISCARD("") Scalar value() const { return m_value; }

private:
	Scalar m_value = 0;
};

class Variable : public Component {
public:
	Variable() : Component() {}
	explicit Variable(std::string name) :
			Component(), m_name(std::move(name)) {}

	LR_NODISCARD("")
	Scalar eval() const override {
		LR_ASSERT(false,
				  "Cannot numerically evaluate variable {}. Missing call to "
				  "'substitute'?",
				  m_name);

		return Scalar(0);
	}

	void treeDepth(int64_t &depth) const override { ++depth; }

	LR_NODISCARD("")
	std::shared_ptr<Component> substitute(
	  const std::map<std::string, std::shared_ptr<Component>> &substitutions)
	  const override {
		auto it =
		  std::find_if(substitutions.begin(),
					   substitutions.end(),
					   [&](auto &pair) { return pair.first == m_name; });
		if (it != substitutions.end())
			return it->second->substitute(substitutions);

		return std::make_shared<Variable>(m_name);
	}

	LR_NODISCARD("") bool canEval() const override { return false; }

	LR_NODISCARD("") std::string str(uint64_t indent) const override {
		return m_name;
	}

	LR_NODISCARD("") std::string name() const override { return m_name; }

	LR_NODISCARD("") std::string type() const override { return "VARIABLE"; }

private:
	std::string m_name = "NONAME";
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

	void treeDepth(int64_t &depth) const override {
		std::vector<int64_t> depths(numOperands(), 0);
		for (int64_t i = 0; i < numOperands(); ++i) {
			int64_t tmpDepth = depth;
			m_values[i]->treeDepth(tmpDepth);
			depths[i] = tmpDepth;
		}

		depth = *std::max_element(depths.begin(), depths.end());
		++depth;
	}

	LR_NODISCARD("")
	Scalar eval() const override {
		std::vector<Scalar> operands;
		for (const auto &val : m_values) operands.push_back(val->eval());
		return m_functor(operands);
	}

	LR_NODISCARD("")
	std::shared_ptr<Component> substitute(
	  const std::map<std::string, std::shared_ptr<Component>> &substitutions)
	  const override {
		std::shared_ptr<Function> res = std::make_shared<Function>(
		  m_name, m_format, m_functor, m_numOperands);
		for (const auto &val : m_values)
			res->addValue(val->substitute(substitutions));
		return res;
	}

	LR_NODISCARD("") bool canEval() const override {
		return std::all_of(m_values.begin(), m_values.end(), [](auto &val) {
			return val->canEval();
		});
	}

	LR_NODISCARD("") std::string str(uint64_t indent) const override {
		return fmt::format("{:>{}}{}", "", indent, m_name);
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
			if (val->type() == "TREE") continue;
			longestType	 = lrc::max(longestType, val->type().length());
			longestValue = lrc::max(longestValue, val->str(0).length());
		}

		for (const auto &val : m_values) {
			if (val->type() == "TREE") {
				res += fmt::format(
				  "\n{}",
				  std::dynamic_pointer_cast<Tree>(val)->tree()[0]->repr(
					indent + 4, longestType, longestValue));
			} else {
				res += fmt::format(
				  "\n{}", val->repr(indent + 4, longestType, longestValue));
			}
		}

		return res;
	}

	LR_NODISCARD("") uint64_t numOperands() const { return m_numOperands; }

	LR_NODISCARD("")
	const std::vector<std::shared_ptr<Component>> &values() const {
		return m_values;
	}

	LR_NODISCARD("") std::vector<std::shared_ptr<Component>> &values() {
		return m_values;
	}

	void addValue(const std::shared_ptr<Component> &value) {
		m_values.push_back(value);
	}

	void clearValues() { m_values.clear(); }

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

// All derivative rules will inherit from this class
class DerivativeRule {
public:
	DerivativeRule() = default;

	// Returns true if this rule can be applied to the input component
	LR_NODISCARD("")
	virtual bool applicable(const std::shared_ptr<Component> &component,
							const std::string &wrt) const = 0;

	// Returns the derivative of the input component
	LR_NODISCARD("")
	virtual std::shared_ptr<Component>
	derivative(const std::shared_ptr<Component> &component,
			   const std::string &wrt) const = 0;

private:
};

// Registered functions
static inline std::vector<std::shared_ptr<Function>> functions;

// Registered constants
static inline std::map<std::string, std::shared_ptr<Component>> constants;

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
	 * <number> ::= <digit>+ | <digit>+ "." <digit>+ | -<digit>+
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
	 *
	 * Identify unary minus vs subtraction
	 * Identity unary plus vs addition
	 */

	std::vector<Lexed> tmp(lexed.begin(), lexed.end());
	std::vector<Lexed> res;

	bool addParen = false;

	for (int64_t i = 0; i < tmp.size() - 1; ++i) {
		// Unary plus if first term or previous was <insert here>
		if ((i == 0 || tmp[i - 1].type &
						 (TYPE_ADD | TYPE_SUB | TYPE_MUL | TYPE_DIV |
						  TYPE_CARET | TYPE_LPAREN | TYPE_PLUS | TYPE_MINUS)) &&
			tmp[i].type & TYPE_ADD) {
			tmp[i].type = TYPE_PLUS | TYPE_OPERATOR;
		}

		// Unary minus if first term or previous was <insert here>
		if ((i == 0 || tmp[i - 1].type &
						 (TYPE_ADD | TYPE_SUB | TYPE_MUL | TYPE_DIV |
						  TYPE_CARET | TYPE_LPAREN | TYPE_PLUS | TYPE_MINUS)) &&
			tmp[i].type & TYPE_SUB) {
			tmp[i].type = TYPE_MINUS | TYPE_OPERATOR;
		}

		if (tmp[i].type & TYPE_NUMBER &&
			tmp[i + 1].type & (TYPE_LPAREN | TYPE_STRING)) {
			// <number> <lparen>
			// addParen = true;
			// res.emplace_back(Lexed {TYPE_LPAREN, "("});
			res.emplace_back(tmp[i]);
			res.emplace_back(Lexed {TYPE_MUL | TYPE_OPERATOR, "*"});
		} else if (tmp[i].type & TYPE_STRING && tmp[i + 1].type & TYPE_LPAREN) {
			// Check the value is not a function
			if (std::find_if(
				  functions.begin(), functions.end(), [&](const auto &f) {
					  return f->name() == tmp[i].val;
				  }) == functions.end()) {
				res.emplace_back(tmp[i]);
				res.emplace_back(Lexed {TYPE_MUL | TYPE_OPERATOR, "*"});
				if (addParen) {
					res.emplace_back(Lexed {TYPE_RPAREN, ")"});
					addParen = false;
				}
			} else {
				// It's a function, so mark it as such and move it to the end of
				// the term
				if (!(tmp[i].type & STATUS_MOVED)) {
					int64_t bracketCount = 1;
					auto tmpIndex		 = i + 1;
					while (bracketCount > 0) {
						tmpIndex++;
						if (tmp[tmpIndex].type & TYPE_LPAREN)
							++bracketCount;
						else if (tmp[tmpIndex].type & TYPE_RPAREN)
							--bracketCount;
					}

					tmp.insert(
					  tmp.begin() + tmpIndex + 1, // After bracket
					  Lexed {tmp[i].type | TYPE_FUNCTION | STATUS_MOVED,
							 tmp[i].val});
					tmp.erase(tmp.begin() + i);
					--i; // Take into account the removed value
				} else {
					// Value already moved, so push it back
					res.emplace_back(
					  Lexed {tmp[i].type | TYPE_FUNCTION, tmp[i].val});
					if (addParen) {
						res.emplace_back(Lexed {TYPE_RPAREN, ")"});
						addParen = false;
					}
				}
			}
		} else {
			res.emplace_back(tmp[i]);
			if (addParen) {
				res.emplace_back(Lexed {TYPE_RPAREN, ")"});
				addParen = false;
			}
		}
	}

	res.emplace_back(tmp.back());
	if (addParen) {
		res.emplace_back(Lexed {TYPE_RPAREN, ")"});
		addParen = false;
	}

	return res;
}

std::vector<Lexed> toPostfix(const std::vector<Lexed> &processed) {
	std::vector<Lexed> postfix;
	std::vector<Lexed> stack;

	for (const auto &lex : processed) {
		if (lex.type & TYPE_VARIABLE) { // Number or string
			postfix.emplace_back(lex);
		} else if (lex.type & (TYPE_OPERATOR | TYPE_FUNCTION)) {
			// Pop operators with higher precedence than the current one
			while (!stack.empty() &&
				   precedence(stack.back().type) >= precedence(lex.type)) {
				postfix.emplace_back(stack.back());
				stack.pop_back();
			}

			// Push the current operator onto the stack
			stack.emplace_back(lex);
		} else if (lex.type & TYPE_LPAREN) {
			stack.emplace_back(lex);
		} else if (lex.type & TYPE_RPAREN) {
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
			if (lex.type & TYPE_PLUS) func = findFunction("PLUS");
			if (lex.type & TYPE_MINUS) func = findFunction("MINUS");
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
		if (lex->type() == "NUMBER" || lex->type() == "VARIABLE") {
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

#include "include/functions.hpp"

Scalar eval(const std::shared_ptr<Component> &tree) {
	// Numerically evaluate the tree
	return tree->eval();
}

std::shared_ptr<Component> substitute(
  const std::shared_ptr<Component> &tree,
  const std::map<std::string, std::shared_ptr<Component>> &substitutions = {}) {
	return tree->substitute(substitutions);
}

std::shared_ptr<Component> autoParse(const std::string &input) {
	auto tokenized = tokenize(input);
	auto lexed	   = lexer(tokenized);

	if (lexed.size() == 1) { // A single term
		if (lexed[0].type & TYPE_NUMBER)
			return std::make_shared<Number>(lexed[0].val);
		else if (lexed[0].type & TYPE_VARIABLE)
			return std::make_shared<Variable>(lexed[0].val);
	}

	auto processed = process(lexed);
	auto postfix   = toPostfix(processed);
	auto parsed	   = parse(postfix);
	auto tree	   = genTree(parsed);

	return tree; // Return the tree object
}

// Forward declare
std::shared_ptr<Component> differentiate(const std::shared_ptr<Component> &,
										 const std::string &);
std::shared_ptr<Component> simplify(const std::shared_ptr<Component> &);

// Differentiate a scalar or variable (not wrt)
class DerivScalar : public DerivativeRule {
public:
	DerivScalar() : DerivativeRule() {}

	LR_NODISCARD("")
	bool applicable(const std::shared_ptr<Component> &component,
					const std::string &wrt) const override {
		std::string type = component->type();
		if (type == "NUMBER" || type == "VARIABLE") return true;
		return false;
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	derivative(const std::shared_ptr<Component> &component,
			   const std::string &wrt) const override {
		/*
		 * d/dx a = 0, for a in R
		 * d/dx y = 0
		 * d/dx x = 1
		 */

		if (component->type() == "NUMBER")
			return std::make_shared<Number>(0);
		else if (component->type() == "VARIABLE") {
			if (std::dynamic_pointer_cast<Variable>(component)->name() == wrt)
				return std::make_shared<Number>(1);
			else
				return std::make_shared<Number>(0);
		}
		return nullptr;
	}
};

// Differentiate unary PLUS or MINUS
class DerivUnaryPlusMinus : public DerivativeRule {
public:
	DerivUnaryPlusMinus() : DerivativeRule() {}

	LR_NODISCARD("")
	bool applicable(const std::shared_ptr<Component> &component,
					const std::string &wrt) const override {
		std::string type = component->type();
		if (type == "FUNCTION") {
			auto func = std::dynamic_pointer_cast<Function>(component);
			if (func->name() == "PLUS" || func->name() == "MINUS") return true;
		}
		return false;
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	derivative(const std::shared_ptr<Component> &component,
			   const std::string &wrt) const override {
		/*
		 * d/dx (+a) = +(d/dx a)
		 * d/dx (-a) = -(d/dx a)
		 */

		// Extract operands
		auto op	  = std::dynamic_pointer_cast<Function>(component);
		auto vals = op->values();
		LR_ASSERT(vals.size() == 1, "Expected 1 operand");
		std::shared_ptr<Component> lhs = differentiate(vals[0], wrt);

		// Duplicate function
		auto it = findFunction(op->name());
		LR_ASSERT(it != functions.end(), "Function not found");

		auto func = std::make_shared<Function>(**it);
		func->addValue(lhs);

		return func;
	}
};

// Differentiate addition and subtraction
class DerivSumDiff : public DerivativeRule {
public:
	DerivSumDiff() : DerivativeRule() {}

	LR_NODISCARD("")
	bool applicable(const std::shared_ptr<Component> &component,
					const std::string &wrt) const override {
		std::string type = component->type();
		if (type == "FUNCTION") {
			auto func = std::dynamic_pointer_cast<Function>(component);
			if (func->name() == "ADD" || func->name() == "SUB") return true;
		}
		return false;
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	derivative(const std::shared_ptr<Component> &component,
			   const std::string &wrt) const override {
		/*
		 * d/dx (a + b) = d/dx a + d/dx b
		 * d/dx (a - b) = d/dx a - d/dx b
		 */

		// Extract operands
		auto op	  = std::dynamic_pointer_cast<Function>(component);
		auto vals = op->values();
		LR_ASSERT(vals.size() == 2, "Expected 2 operands");
		std::shared_ptr<Component> lhs, rhs;
		lhs = differentiate(vals[0], wrt);
		rhs = differentiate(vals[1], wrt);

		// Duplicate addition function
		auto it = findFunction(op->name());
		LR_ASSERT(it != functions.end(), "Function not found");

		auto func = std::make_shared<Function>(**it);
		func->addValue(lhs);
		func->addValue(rhs);

		return func;
	}
};

// Differentiate multiplication
class DerivProd : public DerivativeRule {
public:
	DerivProd() : DerivativeRule() {}

	LR_NODISCARD("")
	bool applicable(const std::shared_ptr<Component> &component,
					const std::string &wrt) const override {
		std::string type = component->type();
		if (type == "FUNCTION") {
			auto func = std::dynamic_pointer_cast<Function>(component);
			if (func->name() == "MUL") return true;
		}
		return false;
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	derivative(const std::shared_ptr<Component> &component,
			   const std::string &wrt) const override {
		/*
		 * d/dx (a * b) = d/dx a * b + a * d/dx b
		 */

		// Extract operands
		auto op	  = std::dynamic_pointer_cast<Function>(component);
		auto vals = op->values();
		LR_ASSERT(vals.size() == 2, "Expected 2 operands");
		std::shared_ptr<Component> da, db;
		da = differentiate(vals[0], wrt);
		db = differentiate(vals[1], wrt);

		// Duplicate addition function
		auto addIt = findFunction("ADD");
		auto mulIt = findFunction("MUL");
		LR_ASSERT(addIt != functions.end(), "Function not found");
		LR_ASSERT(mulIt != functions.end(), "Function not found");

		auto leftMul = std::make_shared<Function>(**mulIt);
		leftMul->addValue(da);
		leftMul->addValue(vals[1]);

		auto rightMul = std::make_shared<Function>(**mulIt);
		rightMul->addValue(vals[0]);
		rightMul->addValue(db);

		auto sum = std::make_shared<Function>(**addIt);
		sum->addValue(leftMul);
		sum->addValue(rightMul);

		return sum;
	}
};

// Differentiate division
class DerivQuotient : public DerivativeRule {
public:
	DerivQuotient() : DerivativeRule() {}

	LR_NODISCARD("")
	bool applicable(const std::shared_ptr<Component> &component,
					const std::string &wrt) const override {
		std::string type = component->type();
		if (type == "FUNCTION") {
			auto func = std::dynamic_pointer_cast<Function>(component);
			if (func->name() == "DIV") return true;
		}
		return false;
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	derivative(const std::shared_ptr<Component> &component,
			   const std::string &wrt) const override {
		/*
		 * d/dx (a / b) = (d/dx a * b - a * d/dx b) / b^2
		 */

		// Extract operands
		auto op	  = std::dynamic_pointer_cast<Function>(component);
		auto vals = op->values();
		LR_ASSERT(vals.size() == 2, "Expected 2 operands");
		std::shared_ptr<Component> da, db;
		da = differentiate(vals[0], wrt);
		db = differentiate(vals[1], wrt);

		// Duplicate addition function
		auto subIt = findFunction("SUB");
		auto mulIt = findFunction("MUL");
		auto divIt = findFunction("DIV");
		auto powIt = findFunction("POW");
		LR_ASSERT(subIt != functions.end(), "Function not found");
		LR_ASSERT(mulIt != functions.end(), "Function not found");
		LR_ASSERT(divIt != functions.end(), "Function not found");
		LR_ASSERT(powIt != functions.end(), "Function not found");

		auto leftMul = std::make_shared<Function>(**mulIt);
		leftMul->addValue(da);
		leftMul->addValue(vals[1]);

		auto rightMul = std::make_shared<Function>(**mulIt);
		rightMul->addValue(vals[0]);
		rightMul->addValue(db);

		auto sum = std::make_shared<Function>(**subIt);
		sum->addValue(leftMul);
		sum->addValue(rightMul);

		auto bSquare = std::make_shared<Function>(**powIt);
		bSquare->addValue(vals[1]);
		bSquare->addValue(autoParse("2"));

		auto div = std::make_shared<Function>(**divIt);
		div->addValue(sum);
		div->addValue(bSquare);

		return div;
	}
};

// Differentiate exponent
class DerivExponent : public DerivativeRule {
public:
	DerivExponent() : DerivativeRule() {}

	LR_NODISCARD("")
	bool applicable(const std::shared_ptr<Component> &component,
					const std::string &wrt) const override {
		std::string type = component->type();
		if (type == "FUNCTION") {
			auto func = std::dynamic_pointer_cast<Function>(component);
			if (func->name() == "POW") return true;
		}
		return false;
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	derivative(const std::shared_ptr<Component> &component,
			   const std::string &wrt) const override {
		/*
		 * d/dx (a ^ b) = b * a ^ (b - 1) * d/dx a			for b in R
		 */

		// Extract operands
		auto op	  = std::dynamic_pointer_cast<Function>(component);
		auto vals = op->values();
		LR_ASSERT(vals.size() == 2, "Expected 2 operands");

		if (vals[1]->canEval()) {
			std::shared_ptr<Component> da, db;
			da = differentiate(vals[0], wrt);

			// Duplicate addition function
			auto subIt = findFunction("SUB");
			auto mulIt = findFunction("MUL");
			auto divIt = findFunction("DIV");
			auto powIt = findFunction("POW");
			LR_ASSERT(subIt != functions.end(), "Function not found");
			LR_ASSERT(mulIt != functions.end(), "Function not found");
			LR_ASSERT(divIt != functions.end(), "Function not found");
			LR_ASSERT(powIt != functions.end(), "Function not found");

			// (b - 1)
			auto bSub = std::make_shared<Function>(**subIt);
			bSub->addValue(vals[1]);
			bSub->addValue(autoParse("1"));

			// a ^ (b - 1)
			auto aPow = std::make_shared<Function>(**powIt);
			aPow->addValue(vals[0]);
			aPow->addValue(bSub);

			// b * a ^ (b - 1)
			auto bMul = std::make_shared<Function>(**mulIt);
			bMul->addValue(vals[1]);
			bMul->addValue(aPow);

			// b * a ^ (b - 1) * d/dx a
			auto mul = std::make_shared<Function>(**mulIt);
			mul->addValue(bMul);
			mul->addValue(da);

			return mul;
		}

		LR_ASSERT(false, "Exponent cannot (yet) be differentiated");
		return component;
	}
};

// Derivative rules
static inline std::vector<std::shared_ptr<DerivativeRule>> derivativeRules;

class SimplificationRule {
public:
	SimplificationRule() = default;

	LR_NODISCARD("")
	virtual bool
	applicable(const std::shared_ptr<Component> &component) const = 0;

	LR_NODISCARD("")
	virtual std::shared_ptr<Component>
	simplifyInput(const std::shared_ptr<Component> &component) const = 0;
};

// Anything that can be numerically evaluated can be simplified
class SimplifyEval : public SimplificationRule {
public:
	SimplifyEval() = default;

	LR_NODISCARD("")
	bool
	applicable(const std::shared_ptr<Component> &component) const override {
		return component->canEval();
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	simplifyInput(const std::shared_ptr<Component> &component) const override {
		return std::make_shared<Number>(component->eval());
	}
};

// Simplify unary plus
class SimplifyPlus : public SimplificationRule {
public:
	SimplifyPlus() = default;

	LR_NODISCARD("")
	bool
	applicable(const std::shared_ptr<Component> &component) const override {
		return component->type() == "FUNCTION" && component->name() == "PLUS";
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	simplifyInput(const std::shared_ptr<Component> &component) const override {
		auto func = std::dynamic_pointer_cast<Function>(component);
		LR_ASSERT(func->numOperands() == 1, "Expected 1 operand");
		auto left = simplify(func->values()[0]);

		// +x = x
		return left;
	}
};

// Simplify unary MINUS
class SimplifyMinus : public SimplificationRule {
public:
	SimplifyMinus() = default;

	LR_NODISCARD("")
	bool
	applicable(const std::shared_ptr<Component> &component) const override {
		return component->type() == "FUNCTION" && component->name() == "MINUS";
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	simplifyInput(const std::shared_ptr<Component> &component) const override {
		auto func = std::dynamic_pointer_cast<Function>(component);
		LR_ASSERT(func->numOperands() == 1, "Expected 1 operand");
		auto left = simplify(func->values()[0]);

		// --x = x
		if (left->type() == "FUNCTION" && left->name() == "MINUS") {
			auto leftFunc = std::dynamic_pointer_cast<Function>(left);
			LR_ASSERT(leftFunc->numOperands() == 1, "Expected 1 operand");
			return simplify(leftFunc->values()[0]);
		}

		auto res = std::make_shared<Function>(*func);
		res->clearValues();
		res->addValue(left);
		return res;
	}
};

// Simplify addition
class SimplifyAdd : public SimplificationRule {
public:
	SimplifyAdd() = default;

	LR_NODISCARD("")
	bool
	applicable(const std::shared_ptr<Component> &component) const override {
		return component->type() == "FUNCTION" && component->name() == "ADD";
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	simplifyInput(const std::shared_ptr<Component> &component) const override {
		auto func = std::dynamic_pointer_cast<Function>(component);
		LR_ASSERT(func->numOperands() == 2, "Expected 2 operands");
		auto left  = simplify(func->values()[0]);
		auto right = simplify(func->values()[1]);

		// 0 + x = x
		if (left->type() == "NUMBER") {
			if (std::dynamic_pointer_cast<Number>(left)->value() == 0) {
				return right;
			}
		}

		// x + 0 = x
		if (right->type() == "NUMBER") {
			if (std::dynamic_pointer_cast<Number>(right)->value() == 0) {
				return left;
			}
		}

		auto res = std::make_shared<Function>(*func);
		res->clearValues();
		res->addValue(left);
		res->addValue(right);
		return res;
	}
};

// Simplify addition
class SimplifySub : public SimplificationRule {
public:
	SimplifySub() = default;

	LR_NODISCARD("")
	bool
	applicable(const std::shared_ptr<Component> &component) const override {
		return component->type() == "FUNCTION" && component->name() == "SUB";
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	simplifyInput(const std::shared_ptr<Component> &component) const override {
		auto func = std::dynamic_pointer_cast<Function>(component);
		LR_ASSERT(func->numOperands() == 2, "Expected 2 operands");
		auto left  = simplify(func->values()[0]);
		auto right = simplify(func->values()[1]);

		// 0 - x = -x
		if (left->type() == "NUMBER") {
			if (std::dynamic_pointer_cast<Number>(left)->value() == 0) {
				auto minusIt = findFunction("MINUS");
				LR_ASSERT(minusIt != functions.end(), "Function not found");
				auto minus = std::make_shared<Function>(**minusIt);
				minus->addValue(right);
				return minus;
			}
		}

		// x - 0 = x
		if (right->type() == "NUMBER") {
			if (std::dynamic_pointer_cast<Number>(right)->value() == 0) {
				return left;
			}
		}

		auto res = std::make_shared<Function>(*func);
		res->clearValues();
		res->addValue(left);
		res->addValue(right);
		return res;
	}
};

// Simplify multiplication
class SimplifyMul : public SimplificationRule {
public:
	SimplifyMul() = default;

	LR_NODISCARD("")
	bool
	applicable(const std::shared_ptr<Component> &component) const override {
		return component->type() == "FUNCTION" && component->name() == "MUL";
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	simplifyInput(const std::shared_ptr<Component> &component) const override {
		auto func = std::dynamic_pointer_cast<Function>(component);
		LR_ASSERT(func->numOperands() == 2, "Expected 2 operands");
		auto left  = simplify(func->values()[0]);
		auto right = simplify(func->values()[1]);

		if (left->type() == "NUMBER") {
			// 0 * x = 0
			if (std::dynamic_pointer_cast<Number>(left)->value() == 0) {
				return std::make_shared<Number>(0);
			}

			// 1 * x = x
			if (std::dynamic_pointer_cast<Number>(left)->value() == 1) {
				return right;
			}
		}

		if (right->type() == "NUMBER") {
			// x * 0 = 0
			if (std::dynamic_pointer_cast<Number>(right)->value() == 0) {
				return std::make_shared<Number>(0);
			}

			// x * 1 = x
			if (std::dynamic_pointer_cast<Number>(right)->value() == 1) {
				return left;
			}
		}

		auto res = std::make_shared<Function>(*func);
		res->clearValues();
		res->addValue(left);
		res->addValue(right);
		return res;
	}
};

// Simplify division
class SimplifyDiv : public SimplificationRule {
public:
	SimplifyDiv() = default;

	LR_NODISCARD("")
	bool
	applicable(const std::shared_ptr<Component> &component) const override {
		return component->type() == "FUNCTION" && component->name() == "DIV";
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	simplifyInput(const std::shared_ptr<Component> &component) const override {
		auto func = std::dynamic_pointer_cast<Function>(component);
		LR_ASSERT(func->numOperands() == 2, "Expected 2 operands");
		auto left  = simplify(func->values()[0]);
		auto right = simplify(func->values()[1]);

		if (left->type() == "NUMBER") {
			// 0 / x = 0
			if (std::dynamic_pointer_cast<Number>(left)->value() == 0) {
				return std::make_shared<Number>(0);
			}
		}

		if (right->type() == "NUMBER") {
			// x / 1 = x
			if (std::dynamic_pointer_cast<Number>(right)->value() == 1) {
				return left;
			}
		}

		auto res = std::make_shared<Function>(*func);
		res->clearValues();
		res->addValue(left);
		res->addValue(right);
		return res;
	}
};

// Simplify exponentiation
class SimplifyExponent : public SimplificationRule {
public:
	SimplifyExponent() = default;

	LR_NODISCARD("")
	bool
	applicable(const std::shared_ptr<Component> &component) const override {
		return component->type() == "FUNCTION" && component->name() == "POW";
	}

	LR_NODISCARD("")
	std::shared_ptr<Component>
	simplifyInput(const std::shared_ptr<Component> &component) const override {
		auto func = std::dynamic_pointer_cast<Function>(component);
		LR_ASSERT(func->numOperands() == 2, "Expected 2 operands");
		auto left  = simplify(func->values()[0]);
		auto right = simplify(func->values()[1]);

		if (left->type() == "NUMBER") {
			// 0 ^ x = 0
			if (std::dynamic_pointer_cast<Number>(left)->value() == 0) {
				return std::make_shared<Number>(0);
			}
		}

		if (right->type() == "NUMBER") {
			// x ^ 0 = 1
			if (std::dynamic_pointer_cast<Number>(right)->value() == 0) {
				return std::make_shared<Number>(1);
			}

			if (std::dynamic_pointer_cast<Number>(right)->value() == 1) {
				return left;
			}
		}

		auto res = std::make_shared<Function>(*func);
		res->clearValues();
		res->addValue(left);
		res->addValue(right);
		return res;
	}
};

// Simplification rules
static inline std::vector<std::shared_ptr<SimplificationRule>>
  simplificationRules;

std::string prettyPrint(const std::shared_ptr<Component> &object) {
	if (object->type() == "TREE")
		return prettyPrint(std::dynamic_pointer_cast<Tree>(object)->tree()[0]);

	if (object->type() == "NUMBER")
		return lrc::str(std::dynamic_pointer_cast<Number>(object)->eval());

	if (object->type() == "VARIABLE")
		return std::dynamic_pointer_cast<Variable>(object)->name();

	if (object->type() == "FUNCTION") {
		auto func	= std::dynamic_pointer_cast<Function>(object);
		auto format = func->format();
		std::vector<std::string> args;
		for (uint64_t i = 0; i < func->numOperands(); ++i) {
			std::shared_ptr<Component> current = func->values()[i];
			std::string printed				   = prettyPrint(current);
			int64_t depth					   = 0;
			current->treeDepth(depth);
			if (depth > 1)
				args.emplace_back("(" + printed + ")");
			else
				args.emplace_back(printed);
		}

		switch (func->numOperands()) {
			case 1: return fmt::format(format, args[0]);
			case 2: return fmt::format(format, args[0], args[1]);
			case 3: return fmt::format(format, args[0], args[1], args[2]);
			case 4:
				return fmt::format(format, args[0], args[1], args[2], args[3]);
			default: return "too_many_args";
		}
	}

	return "";
}

#include "include/differentiate.hpp"
#include "include/constants.hpp"
#include "include/simplify.hpp"

std::shared_ptr<Component>
differentiate(const std::shared_ptr<Component> &input,
			  const std::string &wrt = "x") {
	if (input->type() == "TREE") {
		auto item =
		  differentiate(std::dynamic_pointer_cast<Tree>(input)->tree()[0], wrt);
		auto tree = std::make_shared<Tree>();
		tree->tree().emplace_back(item);
		return tree;
	}

	for (const auto &rule : derivativeRules) {
		if (rule->applicable(input, wrt)) {
			return rule->derivative(input, wrt);
		}
	}

	LR_ASSERT(false, "No applicable rule found for type {}", input->type());
}

std::shared_ptr<Component> simplify(const std::shared_ptr<Component> &input) {
	if (input->type() == "TREE") {
		auto item = simplify(std::dynamic_pointer_cast<Tree>(input)->tree()[0]);
		auto tree = std::make_shared<Tree>();
		tree->tree().emplace_back(item);
		return tree;
	}

	static auto evalRule = std::make_shared<SimplifyEval>();
	auto current		 = input;

	for (const auto &rule : simplificationRules) {
		if (rule->applicable(current)) {
			current = rule->simplifyInput(current);
		}
	}

	// Apply numeric evaluation after all simplification is complete
	if (evalRule->applicable(current)) {
		current = evalRule->simplifyInput(current);
	}

	return current;
}

int main() {
	lrc::prec(1000);

	registerFunctions();
	registerDerivativeRules();
	registerConstants();
	registerSimplifications();

	std::string equation("1/x");

	std::map<std::string, std::shared_ptr<Component>> variables = {
	  std::make_pair("x", autoParse("5"))};

	auto tree	= autoParse(equation);
	auto subbed = substitute(tree, variables);
	auto diff	= differentiate(tree, "x");
	auto simple = simplify(tree);

	fmt::print("\n\nEquation: {}\n", prettyPrint(tree));
	fmt::print("Simplified: {}\n\n", prettyPrint(simple));
	fmt::print("\nDerivative: {}\n", prettyPrint(diff));
	fmt::print("Simplified: {}\n\n", prettyPrint(simplify(diff)));

	// fmt::print("{}\n\n\n", tree->str(0));
	// fmt::print("{}\n\n\n", subbed->str(0));
	// fmt::print("{}\n", diff->str(0));
	fmt::print("Numeric result: {}\n", lrc::str(eval(subbed)));

	//	{
	//		auto help = autoParse("1/x");
	//		for (int i = 0; i < 3; ++i) {
	//			// fmt::print("{}\n", prettyPrint(help));
	//			help = simplify(differentiate(help));
	//		}
	//
	//		fmt::print("{}\n", prettyPrint(help));
	//	}

	{
		std::string eqn("((x^2 - x - 1)/(x^2 + x + 1))^5");
		lrc::timeVerbose([&]() { auto res = autoParse(eqn); }, -1, -1, 1);
		auto parsed = autoParse(eqn);
		lrc::timeVerbose(
		  [&]() { auto res = differentiate(parsed); }, -1, -1, 1);
		auto differentiated = differentiate(parsed);
		lrc::timeVerbose(
		  [&]() { auto res = simplify(differentiated); }, -1, -1, 1);

		std::map<std::string, std::shared_ptr<Component>> vars = {
		  std::make_pair("x", autoParse("5"))};

		fmt::print("Evaluating\n");
		lrc::timeVerbose(
		  [&]() { auto res = eval(substitute(parsed, vars)); }, -1, -1, 10);

		fmt::print("{}\n", prettyPrint(parsed));
		fmt::print("{}\n", prettyPrint(differentiated));
		fmt::print("{}\n", prettyPrint(simplify(differentiated)));
	}

	return 0;
}
