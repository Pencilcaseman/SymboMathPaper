#pragma once

#include "tree.hpp"

namespace symbo {
	class Parser {
	public:
		explicit Parser(const std::string &eqn) : m_eqnStr(eqn) {}

		/**
		 * Given the equation as a string, parse it into an abstract tree representation of
		 * terms and functions
		 */
		void parse() {}

	private:
		std::string m_eqnStr;
	};
} // namespace symbo
