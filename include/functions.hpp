#pragma once

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
	  1));

	// Add the function cos(x)
	functions.emplace_back(std::make_shared<Function>(
	  "cos",
	  "cos({})",
	  [](const std::vector<Scalar> &args) { return lrc::cos(args[0]); },
	  1));

	// Add the function tan(x)
	functions.emplace_back(std::make_shared<Function>(
	  "tan",
	  "tan({})",
	  [](const std::vector<Scalar> &args) { return lrc::tan(args[0]); },
	  1));

	// Add the function csc(x)
	functions.emplace_back(std::make_shared<Function>(
	  "csc",
	  "csc({})",
	  [](const std::vector<Scalar> &args) { return lrc::csc(args[0]); },
	  1));

	// Add the function sec(x)
	functions.emplace_back(std::make_shared<Function>(
	  "sec",
	  "sec({})",
	  [](const std::vector<Scalar> &args) { return lrc::sec(args[0]); },
	  1));

	// Add the function cot(x)
	functions.emplace_back(std::make_shared<Function>(
	  "cot",
	  "cot({})",
	  [](const std::vector<Scalar> &args) { return lrc::cot(args[0]); },
	  1));
}
