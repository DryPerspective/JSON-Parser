#include <algorithm>
#include <vector>
#include <stdexcept>
#include <fstream>

#include "JSONElement.h"

namespace json {

	//Delegating constructor would be preferable to repetition. Alas, we're in C++03
	JSONElement::JSONElement(const std::string& key, const std::string& val) : m_valid(true), m_value(val) {
		putKeyInQuotes(key);
		if (isNotSimpleValue(val)) m_value = val;
		else m_value = '"' + std::string(val) + '"';
	}


	std::string JSONElement::trim(std::string in, const char* letters) {
		std::size_t start = in.find_first_not_of(letters);
		std::size_t end = in.find_last_not_of(letters);
		in = in.substr(start, end - start + 1);
		return in;
	}

	std::string JSONElement::key() const {
		return m_key;
	}

	std::string JSONElement::val() const {
		return m_value;
	}

	bool JSONElement::valid() const {
		return m_valid;
	}

	bool JSONElement::operator!() const {
		return !m_valid;
	}

	bool JSONElement::isList() const {
		std::size_t firstSquare = m_value.find('[');
		//Two situations in which further access is invalid: "a":"b" or "a":[] 
		if (firstSquare == std::string::npos || (firstSquare != m_value.size() - 1 && m_value[firstSquare + 1] == ']')) return false;
		return true;
	}




	/*
	* Valid possibilities we need to deal with:
	* "a":1
	* "a":[]
	* "a":[1]
	* "a":[1,2,3]
	* "a":[{"b":1}]
	* "a":[{"b":1,"c":2}]
	* "a":[{"b":1,"c":2},{"b":3,"c":4}]
	* "":{"b":2,"c":3}						//Result of intermediate process
	* "":{"b":2}
	*/
	JSONElement JSONElement::operator[](std::size_t index) const {
		//Invalid is always invalid
		if (!m_valid) return JSONElement(false);

		std::size_t firstSquareBrace = m_value.find('[');
		std::size_t lastSquareBrace = m_value.rfind(']');
		//First (obvious) case - you can't divide a non-list up further. This stops "a":1
		if (firstSquareBrace == std::string::npos && m_value.find('{') == std::string::npos) return JSONElement(false);

		//Stop "a":[]
		//If everything is inside square braces (not counting leading WS)
		bool everythingInSquareBraces = (m_value.find_first_not_of(" \t\n\r\b") == firstSquareBrace && m_value.find_last_not_of(" \t\n\r\b") == lastSquareBrace);
		//If we're in braces and there are no non-WS characters between the two braces, we have an empty list
		if (everythingInSquareBraces && m_value.find_first_not_of(" \t\n\r\b", firstSquareBrace + 1) == lastSquareBrace) return JSONElement(false);


		//From here it's separating the full "true" lists (those contained within []) from intermediate list terms (those contained within {} only)
		/*
		* "a":[1]
		* "a":[1,2,3]
		* "a":[{"b":1,"c":2}]
		* "a":[{"b":1,"c":2},{"b":3,"c":4}]
		*/
		//In all such cases, the returned value will have an empty key
		if (everythingInSquareBraces) {
			std::vector<std::size_t> commaPos;
			commaPos.reserve(std::count(m_value.begin(), m_value.end(), ','));
			//We want commas which are not within braces, i.e. [{"b":1,"c":2},{"b":3,"c":4}] should only pick up the middle comma, not all 3
			int braceLevel = 0;
			for (int i = 0; i < m_value.size(); ++i) {
				if (m_value[i] == '{') ++braceLevel;
				else if (m_value[i] == '}') --braceLevel;
				else if (m_value[i] == ',' && braceLevel == 0) commaPos.push_back(i);
			}

			//Once we have our commas, this leaves us with three possibilities:
			// * n = 0, and we want what's between the opening [ and the first comma
			// * n = commaCount and we want what's between the closing ] and the last comma
			// * All other n, and we want what's between the commas at commaPos[n-1] and commaPos[n]
			//The only alternative is an out-of-bounds index:
			if (index > commaPos.size()) return JSONElement(false);
			std::size_t startOfTerm = 0;
			std::size_t endOfTerm = 0;
			if (index == 0) {
				startOfTerm = m_value.find_first_of("[");
				if (commaPos.size() > 0) endOfTerm = commaPos[0];
				else endOfTerm = m_value.rfind(']');
			}
			else if (index == commaPos.size()) {
				if (commaPos.size() > 0) startOfTerm = commaPos[commaPos.size() - 1];
				else startOfTerm = m_value.find('[');
				endOfTerm = m_value.rfind(']');
			}
			else {
				startOfTerm = commaPos[index - 1];
				endOfTerm = commaPos[index];
			}
			if (startOfTerm == std::string::npos || endOfTerm == std::string::npos) throw std::runtime_error("Invalid JSON format: " + m_value);
			if (endOfTerm < startOfTerm) throw std::runtime_error("JSON parse error. End of term before start: " + m_value);
			return JSONElement("", trim(m_value.substr(startOfTerm + 1, endOfTerm - startOfTerm - 1)));
		}


		/* And now all we're left with are the intermediate terms, which need to be split up into key-value pairs:
		* {"b":1}
		* {"b":1,"c":2}
		* {"b":1,"c":[{"d":1,"e":2}]}
		*/
		//This is done in a similar pattern to the above terms, except we count colons and ignore terms in square brackets
		std::vector<std::size_t> colonPos;
		colonPos.reserve(std::count(m_value.begin(), m_value.end(), ':'));
		std::size_t braceCount = 0;
		for (int i = 0; i < m_value.length(); ++i) {
			if (m_value[i] == '[') ++braceCount;
			else if (m_value[i] == ']') --braceCount;
			else if (m_value[i] == ':' && braceCount == 0) colonPos.push_back(i);
		}

		//Note the >=. The colons are inside full terms, not between them as with the commas
		if (index >= colonPos.size()) return JSONElement(false);

		/*
		* Again we are left with three possibilities:
		* Index = 0 and we want what's between the comma after first colon and the opening {
		* Index = colonPos.size() - 1 and we want what's between the comma before the last colon and the closing }
		* Index is some other number and we want what's between the commas surrounding colonPos[index]
		*/
		std::size_t startOfTerm = 0;
		std::size_t endOfTerm = 0;
		if (index == 0) {
			//Because keys must be strings, we can be fairly confident this will work, notwithstanding the insane case of a key which includes a {
			startOfTerm = m_value.rfind('{', colonPos[0]);
			//We don't get that luxury with values and need to make sure we get the "top level" comma
			braceCount = 0;
			for (std::size_t i = colonPos[0]; i < m_value.length(); ++i) {
				if (m_value[i] == '[') ++braceCount;
				else if (m_value[i] == ']') --braceCount;
				else if (m_value[i] == ',' && braceCount == 0) {
					endOfTerm = i;
					break;
				}
			}
		}
		else if (index == colonPos.size() - 1) {
			startOfTerm = m_value.rfind(',', colonPos[colonPos.size() - 1]);
			endOfTerm = m_value.rfind('}');
		}
		else {
			startOfTerm = m_value.rfind(',', colonPos[index]);
			braceCount = 0;
			for (std::size_t i = colonPos[0]; i < m_value.length(); ++i) {
				if (m_value[i] == '[') ++braceCount;
				else if (m_value[i] == ']') --braceCount;
				else if (m_value[i] == ',' && braceCount == 0) {
					endOfTerm = i;
					break;
				}
			}
		}
		if (startOfTerm == std::string::npos || endOfTerm == std::string::npos) throw std::runtime_error("Invalid JSON format: " + m_value);
		if (endOfTerm < startOfTerm) throw std::runtime_error("JSON parse error. End of term before start: " + m_value);
		std::string term = m_value.substr(startOfTerm + 1, endOfTerm - startOfTerm - 1);
		std::size_t colon = term.find(':');
		return JSONElement(term.substr(0, colon - 1), term.substr(colon + 1));
	}

	JSONElement JSONElement::operator[](int index) const {
		return this->operator[](static_cast<std::size_t>(index));
	}


	/*
	* Valid possibilities we need to deal with:
	* "a":1
	* "a":[]
	* "a":[1]
	* "a":[1,2,3]
	* "a":[{"b":1}]
	* "a":[{"b":1,"c":2}]
	* "a":[{"b":1,"c":2},{"b":3,"c":4}]
	* "":{"b":2,"c":3}						//Result of intermediate process or representation of the whole list.
	* "":{"b":2}
	*/
	JSONElement JSONElement::operator[](const std::string& index) const {
		if (!m_valid) return JSONElement(false);

		//First things first, if the given key doesn't exist at all in our value, then it's evidently not in the list
		//Note the adding quotes - an index of apple should not correspond to a key of "apple pie"
		std::size_t KeyPos = m_value.find('"' + index + '"');
		if (KeyPos == std::string::npos) return JSONElement(false);

		/*At this stage we know the key is somewhere in the value, but that's not the same as knowing it's a valid key. Consider the JSON "a": [{"b":1,"c":2}, {"b":3,"c":4}]
		* JSON["b"] should return an invalid element because "b" is only a key to a subelement of a list inside the current value, not an element of the current value.
		* From this we can deduce that there are only these possibilities where this operator access is valid:
		* "":{"b":2,"c":3}
		* "":{"b":2}
		* And these are distinguished from the rest by being surrounded by curly brackets
		*/
		//Wouldn't it be nice if we had regex, but we're trapped in C++03 

		//If the first and last non-WS characters are { and } respectively
		bool surroundedByCurly = (m_value.find_first_not_of(" \n\t\r\b") == m_value.find('{') && m_value.find_last_not_of(" \n\t\r\b") == m_value.rfind('}'));
		if (!surroundedByCurly) return JSONElement(false);
		/*
		* This leaves us with
		* "":{"b":2,"c":3}						//Result of intermediate process or representation of the whole list.
		* "":{"b":2}
		* "":{"b":3, "c":[{"d":3,"e":4}]}
		* The third of those shows our dilemma - in that case only "b" and "c" are valid and we need to filter out "d" and "e"
		*/
		//If the given key is inside a nested list, it will encounter a closing ] which is not "cancelled out" by an opening one.
		int braceDepth = 0;
		for (std::size_t i = KeyPos; i < m_value.length(); ++i) {
			if (m_value[i] == '[') ++braceDepth;					//This looks similar to the below loop but they can't be combined
			else if (m_value[i] == ']') --braceDepth;				//The below loop assumes we're at the "top level", this loop tests if it's true
		}
		if (braceDepth != 0) return JSONElement(false);

		//By now we know we're of the correct form, we need the value - everything after the colon through to the next top level comma or closing brace.
		std::size_t endOfTerm = 0;
		for (std::size_t i = KeyPos; i < m_value.length(); ++i) {
			if (m_value[i] == '[') ++braceDepth;
			else if (m_value[i] == ']') --braceDepth;
			else if ((m_value[i] == ',' || m_value[i] == '}') && braceDepth == 0) {
				endOfTerm = i;
				break;
			}
		}

		std::size_t colonPos = m_value.find(':', KeyPos);

		return JSONElement(trim(index), trim(m_value.substr(colonPos + 1, endOfTerm - colonPos - 1)));

	}

	JSONElement JSONElement::createFromString(const std::string& in) {
		return JSONElement("", in);
	}

#ifdef __TCPLUSPLUS__
	JSONElement JSONElement::createFromString(const UnicodeString& in) {
		//This is a silly dance we have to do. Thanks, Delphi.
		AnsiString as = in;
		std::string str = as.c_str();
		return createFromString(str);
	}
#endif

	JSONElement JSONElement::createFromFile(const std::string& filePathAndName) {
		std::ifstream fs(filePathAndName.c_str());
		if (!fs) throw std::runtime_error("Error: Could not open file " + filePathAndName);
		//Double brackets to avoid most vexing parse.
		std::string fileContent((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
		return JSONElement("", fileContent);
	}

	std::string JSONElement::getString() const {
		std::string out = "";
		//For a nonempty key
		if (!(m_key.empty() || m_key == "\"\"")) out += m_key + ":";
		out += m_value;
		return out;
	}

	void JSONElement::putKeyInQuotes(const std::string& key) {
		//Make sure the key (a string) is always in quotes.
		if (key[0] != '"') m_key = '"';
		m_key += key;
		if (m_key.length() == 1 || m_key[m_key.length() - 1] != '"') m_key += '"';
	}

	bool JSONElement::isNotSimpleValue(const std::string& in) {
		std::size_t firstChar = in.find_first_not_of(" \t\n\r\b");
		if (firstChar == std::string::npos) return false;

		return (in[firstChar] == '[' || in[firstChar] == '{');
	}

}