#pragma once

void registerConstants() {
#if defined(SYMBOMATH_MULTIPRECISION)
	constants["pi"] = autoParse(lrc::str(lrc::constPi()));
	// autoParse(lrc::str(lrc::PI),
	// autoParse(lrc::str(lrc::E))),
	// autoParse(lrc::str(lrc::GOLDENRATIO))),
	// autoParse(lrc::str(lrc::GOLDENRATIO))),
#else
#endif
}
