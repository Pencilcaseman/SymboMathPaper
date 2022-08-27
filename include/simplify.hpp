#pragma once

void registerSimplifications() {
	simplificationRules.emplace_back(std::make_shared<SimplifyAdd>());
	simplificationRules.emplace_back(std::make_shared<SimplifyMul>());
	simplificationRules.emplace_back(std::make_shared<SimplifyDiv>());
	simplificationRules.emplace_back(std::make_shared<SimplifyExponent>());
}