#pragma once

void registerDerivativeRules() {
	derivativeRules.emplace_back(std::make_shared<DerivScalar>());
	derivativeRules.emplace_back(std::make_shared<DerivSumDiff>());
	derivativeRules.emplace_back(std::make_shared<DerivProd>());
	derivativeRules.emplace_back(std::make_shared<DerivQuotient>());
	derivativeRules.emplace_back(std::make_shared<DerivExponent>());
}