#include <vector>

#include "JSONEntry.h"




//Our primary lookup will be via string, since the underlying data structure is all in terms of strings.
//However as a single entry may contain an array with its own subentries, we also need to be able to parse and generate them
//Here, all we need are top level commas, i.e. commas that are not inside a block of {.....}
const JSONEntry JSONEntry::operator[](std::size_t index) const {
	std::size_t braceDepth = 0;
	std::vector<std::size_t> commaPos;
	bool foundFirstData = false;

	for (std::size_t i = 0; i < m_data.length(); ++i) {
		if (m_data[i] == '{') ++braceDepth;
		else if (m_data[i] == '}') --braceDepth;
		else if (braceDepth == 0 && m_data[i] == ',') commaPos.push_back(i);
	}
	//In the event that the user input an invalid index or invalid JSON format, we return an error case
	if (index > commaPos.size() || commaPos.size() == 0) {
		return JSONEntry(false);
	}

	//Otherwise we return the data between the indexth and (index + 1)th top level commas, accounting for each end
	//If they want the beginning
	std::string newData;
	if (index == 0) {
		//If at the beginning, it is possible that there are some leading symbols we want to trim,
		//e.g. "Key" : [{"A":1,"B":2}, -> "A":1,"B":2
		newData = m_data.substr(0, commaPos[0]);

		std::size_t goFrom = 0;
		//Our logic is simple - if there is an opening [ after the first colon, we trim to the [. Otherwise we assume we are
		//not in this specific situation
		std::size_t openingBrace = newData.find("[");
		if (openingBrace != std::string::npos && openingBrace > newData.find(":")) goFrom = openingBrace;
		newData.erase(0, goFrom);

		trim(newData, " \n\r\t\b,{}[]:");
	}
	//If they want the end.
	else if (index == commaPos.size()) {
		newData = m_data.substr(commaPos[commaPos.size() - 1]);
		trim(newData, " \n\r\t\b,{}[]:");
	}
	//Otherwise
	else {
		newData = m_data.substr(commaPos[index - 1], commaPos[index] - commaPos[index - 1]);
		trim(newData, " \n\r\t\b,{}[]:");
	}
	//It's possible that the trimming removed a trailing array at the end of the block, e.g.
	// "ListedTimes : []" ->  "ListedTimes : ".
	//In this case, we add our array back in.
	if (std::count(newData.begin(), newData.end(), ':') == 0 && m_data.find("[]") != std::string::npos) {
		newData += " : []";
	}
	return JSONEntry(newData);
}



const JSONEntry JSONEntry::operator [](int index) const {
	return this->operator[](static_cast<std::size_t>(index));
}

const JSONEntry JSONEntry::operator [](const std::string& index) const {
	if (index.length() > 1020) return JSONEntry(false);
	char newIndex[1024];

	//Warning suppression for MSVC. Since this is a C++03 project, and we have ensured that the buffer is large enough to contain the data we copy into it,
	//we know this is a safe line of code.
#pragma warning(suppress : 4996)
	std::strncpy(newIndex, index.c_str(), index.length() + 1);
	
	return this->operator [](newIndex);
}

//Avoid duplication by having the non-const overloads call the const ones
JSONEntry JSONEntry::operator[](std::size_t index) {
	return static_cast<const JSONEntry>(*this)[index];
}
JSONEntry JSONEntry::operator[](int index) {
	return static_cast<const JSONEntry>(*this)[static_cast<std::size_t>(index)];
}
JSONEntry JSONEntry::operator[](const std::string& index) {
	return static_cast<const JSONEntry>(*this)[index.c_str()];
}

std::string& trim(std::string& toTrim, const char* charsToTrim) {
	toTrim.erase(0, toTrim.find_first_not_of(charsToTrim));
	toTrim.erase(toTrim.find_last_not_of(charsToTrim) + 1, toTrim.length());
	return toTrim;

}
std::string trim(const std::string& toTrim, const char* charsToTrim) {
	std::string copy = toTrim;
	return trim(copy, charsToTrim);
}

bool operator==(const JSONEntry& lhs, const JSONEntry& rhs) {
	if (&lhs == &rhs) return true;

	//All invalid items are equally invalid
	if (!lhs && !rhs) return true;
	else if (!lhs || !rhs) return false;

	return lhs.as<std::string>() == rhs.as<std::string>();
}

bool operator!=(const JSONEntry& lhs, const JSONEntry& rhs) {
	return !(lhs == rhs);
}

bool JSONEntry::valid() const {
	return m_valid;
}

bool JSONEntry::operator!() const {
	return !this->valid();
}

std::pair<std::string::const_iterator, std::string::const_iterator> JSONEntry::key() const {
	std::string::const_iterator it = m_data.begin();
	std::string::const_iterator itfirst;
	bool firstQuotes = false;

	//Typical use will be much less than a full O(n)
	while (it != m_data.end()) {
		if (*it == '\"') {
			if (!firstQuotes) {
				firstQuotes = true;
				itfirst = it + 1;
			}
			else {
				//--it;
				break;
			}
		}
		++it;
	}
	return std::make_pair(itfirst, it);
}



std::size_t findEndOfTerm(const std::string& data, std::size_t startOfTerm) {
	//We want the position of the first "top level" comma or closing brace.
	std::size_t braceCount = 0;
	for (std::size_t i = startOfTerm; i < data.length(); ++i) {
		if (data[i] == '{' || data[i] == '[') ++braceCount;
		else if ((data[i] == '}' || data[i] == ']') && braceCount > 0) --braceCount;
		else if (braceCount == 0 && (data[i] == ',' || data[i] == '}')) return i;
	}
	return data.length() - 1;
}

