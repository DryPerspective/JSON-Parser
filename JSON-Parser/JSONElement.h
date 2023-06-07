#ifndef JSON_ELEMENT_03
#define JSON_ELEMENT_03

#include <string>

//A single element of a JSON. One key and one value
class JSONElement
{

	std::string m_key;
	std::string m_value;
	bool		m_valid;

	std::string trim(std::string in, const char* letters = "\n\t\r\b \"") const;

public:

	JSONElement(const std::string& key, const std::string& val) : m_key(key), m_value(val), m_valid(true) {}
	explicit JSONElement(bool valid) : m_key(""), m_value(""), m_valid(valid) {}

	std::string key() const;
	std::string val() const;
	bool valid() const;

	bool isList() const;


	//The value may contain a list, so we provide accessors
	JSONElement operator[](std::size_t index) const;
	JSONElement operator[](const char* index) const;


};
#endif
