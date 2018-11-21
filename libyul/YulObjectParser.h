/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * Parser for Yul code and data object container.
 */

#pragma once

#include <libyul/YulString.h>
#include <libyul/YulObject.h>

#include <libsolidity/parsing/ParserBase.h>
#include <libsolidity/interface/ErrorReporter.h>

#include <libdevcore/CommonData.h>

#include <memory>

namespace dev
{
namespace solidity
{
class Scanner;
}

namespace yul
{

/**
 * Yul object parser. Invokes the inline assembly parser.
 */
class YulObjectParser: public solidity::ParserBase
{
public:
	explicit YulObjectParser(
		solidity::ErrorReporter& _errorReporter,
		solidity::assembly::AsmFlavour _flavour = solidity::assembly::AsmFlavour::Loose
	):
		ParserBase(_errorReporter), m_flavour(_flavour) {}

	/// Parses a Yul object.
	/// Falls back to code-only parsing if the source starts with `{`.
	/// @param _reuseScanner if true, do check for end of input after the last `}`.
	/// @returns an empty shared pointer on error.
	std::shared_ptr<YulObject> parse(std::shared_ptr<solidity::Scanner> const& _scanner, bool _reuseScanner);

private:
	std::shared_ptr<Block> parseCode();
	void parseData(YulObject& _currentObject);
	std::shared_ptr<YulObject> parseObject();

	solidity::assembly::AsmFlavour m_flavour;
	std::shared_ptr<solidity::Scanner> m_scanner;
};

}
}
