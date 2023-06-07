#include <algorithm>
#include <vector>
#include <stdexcept>

#include "JSONElement.h"


std::string JSONElement::trim(std::string in, const char* letters) const {
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
	//First (obvious) case - you can't divide a non-list up further. This stops "a":1
	if (firstSquareBrace == std::string::npos && m_value.find('{') == std::string::npos) return JSONElement(false);

	//Stop "a":[]
	if (firstSquareBrace < m_value.length() - 1 && m_value[firstSquareBrace + 1] == ']') return JSONElement(false);

	//Second fringe case - a non-empty list with only one element, e.g. "a":[1] or "a":[{"b":1}]
	//Assuming a valid JSON, this is found because it's the only valid possibility which does not feature a comma
	std::size_t commaCount = std::count(m_value.begin(), m_value.end(), ',');
	if (commaCount == 0) {
		//Since it has only one element, only one index is valid
		if (index != 0) return JSONElement(false);

		//Otherwise, differentiate between the two "flavours" and go from there
		std::size_t colonPos = m_value.find(':');
		//"a":[1]
		if (colonPos == std::string::npos) {
			std::size_t openBrace = m_value.find('[');
			std::size_t closeBrace = m_value.find(']', openBrace);
			if (openBrace == std::string::npos || closeBrace == std::string::npos) throw std::runtime_error("Invalid JSON format for single element list: " + m_value);

			return JSONElement("", trim(m_value.substr(openBrace + 1, closeBrace - openBrace - 1)));
		}
		//"a":[{"b":2}] and "":{"b":2}
		else {
			std::size_t startOfKey = m_value.rfind('{',colonPos);
			std::size_t endOfVal = m_value.find('}', colonPos);
			if (startOfKey == std::string::npos || endOfVal == std::string::npos) throw std::runtime_error("Invalid JSON format for single element list: " + m_value);

			return JSONElement(trim(m_value.substr(startOfKey + 1, colonPos - startOfKey - 1)), trim(m_value.substr(colonPos + 1, endOfVal - colonPos - 1)));
		}
	}

	//Again we can account for an invalid case - an out of bounds index
	if (index > commaCount) return JSONElement(false);

	/*
	By this point we are on to multi-element lists. We have these possibilities :
	* "a" : [1,2,3]
	* "a" : [{"b":1,"c":2}]
	* "a" : [{"b":1,"c":2},{"b":3,"c":4}]
	* ""  : {"b":1,"c":2}
	We want to split the terms using a comma as a delimiter, making sure only to get the "top level" commas (e.g so the third example above is split into two terms, not four)

	*/
	std::vector<std::size_t> commaPos;
	commaPos.reserve(commaCount);
	std::size_t braceCount = 0;

	for (std::size_t i = 0; i < m_value.length(); ++i) {
		if (m_value[i] == '{') ++braceCount;
		if (m_value[i] == '}') --braceCount;
		if (braceCount == 0 && m_value[i] == ',') commaPos.push_back(i);
	}

	//These three possibilities all play out the same way - grab the term between




}