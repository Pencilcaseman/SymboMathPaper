#include <librapid/librapid.hpp>

#include "include/symbomath.hpp"
#include <memory>
#include <functional>
#include <utility>

namespace lrc = librapid;

inline constexpr int64_t formatWidth = 15;

/**
 * The most fundamental type. All numbers, functions, variables, etc. inherit from this.
 */
class Component {
public:
	Component() = default;

	LR_NODISCARD("") virtual double eval() const {
		LR_ASSERT(false, "{} object cannot be evaluated (numerically) directly", type());
	}

	LR_NODISCARD("") virtual const std::vector<std::shared_ptr<Component>> &tree() const {
		LR_ASSERT(false,
				  "Only a value of type Tree is allowed to use this function. Type of *this is {}",
				  type());
		return m_tmpTree;
	}

	LR_NODISCARD("") virtual std::vector<std::shared_ptr<Component>> &tree() {
		LR_ASSERT(false,
				  "Only a value of type Tree is allowed to use this function. Type of *this is {}",
				  type());
		return m_tmpTree;
	}

	LR_NODISCARD("") virtual std::string str(uint64_t indent) const {
		return fmt::format("{: >{}}{}", "", indent, "NONE");
	}

	LR_NODISCARD("")
	virtual std::string repr(uint64_t indent, uint64_t typeWidth, uint64_t valWidth) const {
		return fmt::format(
		  "{: >{}}[ {:^{}} ] [ {:^{}} ]", "", indent, type(), typeWidth, str(0), valWidth);
	}

	LR_NODISCARD("") virtual std::string type() const { return "COMPONENT"; }

private:
	std::vector<std::shared_ptr<Component>> m_tmpTree; // Empty value to return in tree()
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
	explicit Variable(std::string name) : Component(), m_name(std::move(name)) {}

	LR_NODISCARD("") std::string str(uint64_t indent) const override { return m_name; }

	LR_NODISCARD("") std::string type() const override { return "VARIABLE"; }

private:
	std::string m_name = "NONAME";
};

class Tree : public Component {
public:
	Tree() : Component() {}

	LR_NODISCARD("") const std::vector<std::shared_ptr<Component>> &tree() const override {
		return m_tree;
	}

	LR_NODISCARD("") std::vector<std::shared_ptr<Component>> &tree() override { return m_tree; }

	LR_NODISCARD("") std::string str(uint64_t indent) const override {
		// Format stuff really nicely :)
		uint64_t longestType = 0, longestValue = 0;
		for (const auto &val : m_tree) {
			longestType	 = lrc::max(longestType, val->type().length());
			longestValue = lrc::max(longestValue, val->str(0).length());
		}

		std::string res = fmt::format("{: >{}}[ TREE ]", "", indent);

		for (const auto &val : m_tree)
			res += fmt::format("\n{}", val->repr(indent + 4, longestType, longestValue));

		return res;
	}

	LR_NODISCARD("") std::string type() const override { return "TREE"; }

private:
	std::vector<std::shared_ptr<Component>> m_tree;
};

class Function : public Component {
public:
	Function() : Component() {}

	explicit Function(std::string name, std::string format,
					  std::function<double(const std::vector<double> &)> functor,
					  std::vector<std::shared_ptr<Component>> values) :
			Component(),
			m_name(std::move(name)), m_format(std::move(format)), m_functor(std::move(functor)),
			m_values(std::move(values)) {}

	LR_NODISCARD("") std::string str(uint64_t indent) const override {
		return fmt::format("{: >{}}{}", "", indent, m_name);
	}

	LR_NODISCARD("")
	std::string repr(uint64_t indent, uint64_t typeWidth, uint64_t valWidth) const override {
		std::string res = fmt::format(
		  "{: >{}}[ {:^{}} ] [ {:^{}} ]", "", indent, type(), typeWidth, str(0), valWidth);

		// Format stuff really nicely :)
		uint64_t longestType = 0, longestValue = 0;
		for (const auto &val : m_values) {
			longestType	 = lrc::max(longestType, val->type().length());
			longestValue = lrc::max(longestValue, val->str(0).length());
		}

		for (const auto &val : m_values)
			res += fmt::format("\n{}", val->repr(indent + 4, longestType, longestValue));

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
	std::string name;
	char val;
};

std::vector<Token> tokenize(const std::string &input) {
	std::vector<Token> res;

	// Scan the input string
	for (const auto &c : input) {
		if ('0' <= c && c <= '9')
			res.emplace_back(Token {"DIGIT", c});
		else if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
			res.emplace_back(Token {"CHAR", c});
		else if (c == '+')
			res.emplace_back(Token {"ADD", c});
		else if (c == '-')
			res.emplace_back(Token {"SUB", c});
		else if (c == '*')
			res.emplace_back(Token {"MUL", c});
		else if (c == '/')
			res.emplace_back(Token {"DIV", c});
		else if (c == '^')
			res.emplace_back(Token {"CARET", c});
		else if (c == '(')
			res.emplace_back(Token {"LPAREN", c});
		else if (c == ')')
			res.emplace_back(Token {"RPAREN", c});
		else if (c == '.')
			res.emplace_back(Token {"POINT", c});
		else if (c == ' ')
			continue;
		else
			LR_ASSERT(false, "Unknown token '{}'", c);
	}

	return res;
}

struct Tokenized {
	std::string name;
	std::string value;
};

std::vector<Tokenized> lexer(const std::vector<Token> &tokens) {
	for (const auto &tok : tokens) {

	}
}

int main() {
	fmt::print("Hello, World\n");

	auto tree = std::make_shared<Tree>();

	auto node1 = std::make_shared<Function>(
	  "ADD",
	  "{} + {}",
	  [](const std::vector<double> &args) { return args[0] + args[1]; },
	  std::vector<std::shared_ptr<Component>> {std::make_shared<Number>(123),
											   std::make_shared<Variable>("x")});

	auto node2 = std::make_shared<Function>(
	  "ADD",
	  "{} + {}",
	  [](const std::vector<double> &args) { return args[0] + args[1]; },
	  std::vector<std::shared_ptr<Component>> {std::make_shared<Number>(456),
											   std::make_shared<Variable>("y")});

	tree->tree().emplace_back(node1);
	tree->tree().emplace_back(node2);

	fmt::print("{}\n", tree->str(0));

	for (const auto &val : tokenize("123.456 + xyz")) {
		fmt::print("[ {:^6} ] {}\n", val.name, val.val);
	}

	return 0;
}
