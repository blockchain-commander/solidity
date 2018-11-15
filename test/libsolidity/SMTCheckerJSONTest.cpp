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

#include <test/libsolidity/SMTCheckerJSONTest.h>
#include <test/Options.h>
#include <libsolidity/interface/StandardCompiler.h>
#include <libdevcore/JSON.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/throw_exception.hpp>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <sstream>

using namespace dev;
using namespace solidity;
using namespace dev::solidity::test;
using namespace dev::solidity::test::formatting;
using namespace std;
namespace fs = boost::filesystem;
using namespace boost::unit_test;

SMTCheckerTest::SMTCheckerTest(string const& _filename)
: SyntaxTest(_filename)
{
	if (!boost::algorithm::ends_with(_filename, ".sol"))
		BOOST_THROW_EXCEPTION(runtime_error("Invalid test contract file name: \"" + _filename + "\"."));

	string jsonFilename = _filename.substr(0, _filename.size() - 4) + ".json";
	BOOST_CHECK(jsonParseFile(jsonFilename, m_smtResponses));
	BOOST_CHECK(m_smtResponses.isObject());
}

bool SMTCheckerTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	StandardCompiler compiler;

	Json::Value input = buildJson();
	Json::Value result = compiler.compile(input);

	vector<string> outHashes = hashesFromJson(result, "auxiliaryInputRequested", "smtlib2queries");
	string auxInput("auxiliaryInput");
	vector<string> inHashes = hashesFromJson(m_smtResponses, auxInput, "smtlib2responses");

	BOOST_CHECK(outHashes == inHashes);
	BOOST_CHECK(m_smtResponses.isMember(auxInput));
	input[auxInput] = m_smtResponses[auxInput];

	Json::Value endResult = compiler.compile(input);

	BOOST_CHECK(endResult.isMember("errors"));
	Json::Value const& errors = endResult["errors"];
	for (auto const& err: errors)
	{
		BOOST_CHECK(err.isMember("type") && err["type"].isString());
		BOOST_CHECK(err.isMember("message") && err["message"].isString());
		if (!err.isMember("sourceLocation"))
			continue;
		Json::Value const& location = err["sourceLocation"];
		BOOST_CHECK(location.isMember("start") && location["start"].isInt());
		BOOST_CHECK(location.isMember("end") && location["end"].isInt());
		m_errorList.emplace_back(SyntaxTestError{
			err["type"].asString(),
			err["message"].asString(),
			location["start"].asInt(),
			location["end"].asInt()
		});
	}

	return printExpectationAndError(_stream, _linePrefix, _formatted);
}

vector<string> SMTCheckerTest::hashesFromJson(Json::Value const& _jsonObj, string const& _auxInput, string const& _smtlib)
{
	vector<string> hashes;
	Json::Value const& auxInputs = _jsonObj[_auxInput];
	if (!!auxInputs)
	{
		Json::Value const& smtlib = auxInputs[_smtlib];
		if (!!smtlib)
			for (auto const& hashString: smtlib.getMemberNames())
				hashes.push_back(hashString);
	}
	return hashes;
}

Json::Value SMTCheckerTest::buildJson()
{
	string language = "\"language\": \"Solidity\"";
	string sourceName = "\"A\"";
	string sourceContent = "\"" + m_source + "\"";
	string sourceObj = "{ \"content\": " + sourceContent + "}";
	string sources = " \"sources\": { " + sourceName + ": " + sourceObj + "}";
	string input = "{" + language + ", " + sources + "}";
	Json::Value source;
	jsonParse(input, source);
	return source;
}
