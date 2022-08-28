#pragma once

void registerSimplifications() {
	simplificationRules.emplace_back(std::make_shared<SimplifyPlus>());
	simplificationRules.emplace_back(std::make_shared<SimplifyMinus>());
	simplificationRules.emplace_back(std::make_shared<SimplifyAdd>());
	simplificationRules.emplace_back(std::make_shared<SimplifySub>());
	simplificationRules.emplace_back(std::make_shared<SimplifyMul>());
	simplificationRules.emplace_back(std::make_shared<SimplifyDiv>());
	simplificationRules.emplace_back(std::make_shared<SimplifyExponent>());
}