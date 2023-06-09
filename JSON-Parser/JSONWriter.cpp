


//---------------------------------------------------------------------------
#include <fstream>
#include <sstream>


#include "JSONWriter.h"

namespace json {

	void JSONWriter::add(const JSONElement& input) {
		if (input.key() != "\"\"") m_data.push_back(input.key() + ":");
		m_data.push_back(input.val());
	}

	void JSONWriter::startArray(const std::string& key) {
		m_data.push_back("\"" + key + "\":[");
		++m_arrayDepth;
	}

	void JSONWriter::endArray() {
		if (m_arrayDepth == 0 || m_data.empty()) return;

		std::string& mostRecentTerm = m_data[m_data.size() - 1];
		std::size_t lastTokenIndex = mostRecentTerm.find_last_not_of(" \t\n\r\b");

		if (lastTokenIndex == std::string::npos) return;
		if (mostRecentTerm[lastTokenIndex] != '}') mostRecentTerm += "]";
		else m_data.push_back("]");

		--m_arrayDepth;
	}

	void JSONWriter::startArrayItem() {
		m_data.push_back("{");
	}

	void JSONWriter::endArrayItem() {
		m_data.push_back("}");
	}




	void JSONWriter::writeToFile(const std::string& fileName, std::ios_base::openmode openArgs) {
		if (!m_valid) return;

		std::ofstream out(fileName.c_str(), openArgs);
		if (!out) {
			m_valid = false;
			return;
		}

		out << processData();
		out.close();

	}

	//Note this is a non-member function which exists only in this TU.
	inline bool removeMeaninglessChars(char c) {
		if (c == '\n' || c == '\t' || c == '\r' || c == '\b') return true;
		return false;
	}

	std::string JSONWriter::getString(bool removeWS) {
		if (!m_valid) return "";

		std::string data = processData();
		if (removeWS) {
			//NB: We don't remove white spaces with isspace as it may remove spaces from data made up of multiple words
			data.erase(std::remove_if(data.begin(), data.end(), removeMeaninglessChars), data.end());
		}
		return data;

	}

	std::string JSONWriter::processData() {

		std::stringstream out;
		out << "{\n";

		std::size_t braceDepth = 0;
		//We add a dummy closing element as we need to compare against the "next" term in all cases except the end.
		m_data.push_back("}");
		for (std::size_t i = 0; i < m_data.size() - 1; ++i) {

			if (i > 0) {
				if (m_data[i - 1].find_first_of("{[") != std::string::npos) ++braceDepth;
				if (m_data[i - 1].find_first_of("}]") != std::string::npos) braceDepth = (braceDepth == 0) ? 0 : braceDepth - 1;
			}

			for (std::size_t j = 0; j < braceDepth; ++j) {
				out << '\t';
			}

			out << m_data[i];

			//We add a comma at the end of a row if it is a data row (i.e. not an opening/closing brace) and if it is not at the end of
			//an array (i.e. the first non-ws character of the next term is not a closing brace or closing square bracket
			std::size_t thisTermData = m_data[i].find_last_not_of(" \t\n\r\b");
			std::size_t nextTermData = m_data[i + 1].find_first_not_of(" \t\n\r\b");
			if (nextTermData != std::string::npos
				&& thisTermData != std::string::npos
				&& m_data[i + 1][nextTermData] != '}'
				&& m_data[i + 1][nextTermData] != ']'
				&& m_data[i][thisTermData] != '{'
				&& m_data[i][thisTermData] != '['
				) out << ',';

			out << '\n';

		}

		out << "}\n";

		//And we don't forget to pop our dummy ending off
		m_data.pop_back();

		return out.str();
	}

	bool JSONWriter::valid() {
		return m_valid;
	}
	bool JSONWriter::operator !() {
		return !this->valid();
	}

}
