//---------------------------------------------------------------------------

#ifndef JSON_03_ENTRY
#define JSON_03_ENTRY
//---------------------------------------------------------------------------

#include <string>

#include <cstdlib>
#include <algorithm>

#include "Tags.h"

/*
*  A class representing an entry in the JSON, which acts as a kind of proxy object (if we slightly loosen the definition of that term)
*  This indirection allows us a simple and idiomatic way to account for arrays in the JSON, allow multidimensional access (e.g. JSON[0][1])
*  As well as use in-built conversions to return the data in the desired format.
*/


//Trim leading and trailing undesired characters off a string.
std::string& trim(std::string& toTrim, const char* charsToTrim = " \t\r\n\b,{}[]");
std::string trim(const std::string& toTrim, const char* charsToTrim = " \t\r\n\b,{}[]");


std::size_t findEndOfTerm(const std::string& data, std::size_t startOfTerm = 0);



class JSONEntry {


public:

	friend bool operator==(const JSONEntry& lhs, const JSONEntry& rhs);
	friend bool operator!=(const JSONEntry& lhs, const JSONEntry& rhs);

	bool valid() const;
	bool operator!() const;


	//Returning by const value is intentional - operator[] can be chained repeatedly but no element which starts off const should be
	//assignable at a later date
	const JSONEntry operator[](std::size_t index) const;
	JSONEntry operator[](std::size_t index);

	//For accessing a particular key
	//In terms of error handling, per the spec, in the event that the user enters an invalid index we return a dud error case.
	//In the event that this object represents an array with multiple keys, we return the first matching one.
	//Note we use const char* as the overwhelming majority of use-cases will be from some string literal in the code,
	//and taking that literal as directly as possible, rather than needing to construct a string around it every time,
	//seemed like the optimal approach. An overload for std::string is provided.
	//For accessing a particular key
	//In terms of error handling, per the spec, in the event that the user enters an invalid index we return a dud error case.
	//In the event that this object represents an array with multiple keys, we return the first matching one.
	//Note we use const char* as the overwhelming majority of use-cases will be from some string literal in the code,
	//and taking that literal as directly as possible, rather than needing to construct a string around it every time,
	//seemed like the optimal approach. An overload for std::string is provided.
	template<std::size_t N>
	const JSONEntry operator[](const char(&index)[N]) const {
		std::size_t indexPos = m_data.find(index);
		if (indexPos == std::string::npos) return JSONEntry(false);

		std::size_t nextComma = findEndOfTerm(m_data, indexPos);
		//This one is not an error case. Simple entries in the file e.g. "Name":"John Smith" will be stored often.
		//In this case, there will be no comma.
		if (nextComma == std::string::npos) return *this;


		//We subtract one to prevent us trimming off the first quote mark, e.g. JSONReader["B"] -> B":2 rather than "B":2
		//We account for the possibility of the user manually entering the quote marks in the key using the trim.
		--indexPos;
		std::string newData = m_data.substr(indexPos, nextComma - indexPos + 1);
		return JSONEntry(trim(newData, " \t\r\n\b,{}[:"));
	}
	template<std::size_t N>
	JSONEntry operator[](const char(&index)[N]) {
		return static_cast<const JSONEntry>(*this)[index];
	}



	//This overload is to account for the fact that while std::size_t is a more "natural" type to index over, the majority
	//of use-cases will be for integer literals within the code, which are parsed as int.
	//An exact type match may reduce ambiguities in any future changes to this code.
	const JSONEntry operator[](int index) const;
	JSONEntry operator[](int index);

	//Support for full std::strings, even if const char* will be more common.
	const JSONEntry operator[](const std::string& index) const;
	JSONEntry operator[](const std::string& index);




	//Our main function to get the value from an entry, templated to allow quick and easy conversion
	template<typename T>
	T as() const {
		if (!m_valid) return as_helper<T>::get(m_data, instance_of<T>());

		std::size_t colonPos = m_data.find(':');
		std::string value;
		if (colonPos != std::string::npos) value = m_data.substr(colonPos, findEndOfTerm(m_data, colonPos) - colonPos + 1);
		else value = m_data;
		/*
		* In case you are unfamiliar with C++03 TMP techniques:
		* To elaborate on the trick here - passing an instance_of<T> forces the compiler to look for an appropriate get()
		* function in the at_helper struct. Since the overloads use tags which are constructble from specific instance_of
		* types, it becomes a game of matching the instance_of<T> to a constructor of one of those tags.
		* As we have carefully designed are tags and overloads such that a given <T> will match no more than one constructor,
		* the result is either a successful match to the function of the correct type, or a compiler failure if the user asks for
		* an unsupported type.
		*/
		return as_helper<T>::get(trim(value, " \t\r\n\b,{}[]:\""), instance_of<T>());
	}




	//Functor for comparison by key
	struct Compare {
		typedef std::pair<std::string::const_iterator, std::string::const_iterator> itpair;

		inline bool operator()(const JSONEntry& lhs, const JSONEntry& rhs) const {
			itpair lhs_it = lhs.key();
			itpair rhs_it = rhs.key();
			return std::lexicographical_compare(lhs_it.first, lhs_it.second, rhs_it.first, rhs_it.second);
		}


		inline bool operator()(const JSONEntry& lhs, const std::string& str) const {
			itpair lhs_it = lhs.key();
			return std::lexicographical_compare(lhs_it.first, lhs_it.second, str.begin(), str.end());
		}

	};


private:

	std::string 	   	m_data;
	bool        		m_valid;


	//Ctors
	//As this is in many ways a proxy object, we only want it constructible from an object which represents actual
	//JSON data. As such, constructors are private and only accessible to friends.
	//"Invalid state" constructor - for failure cases
	explicit JSONEntry(bool exists) : m_data("N/A"), m_valid(exists) {}

	//Primary constructors for handling data
	explicit JSONEntry(const std::string& inData) : m_data(inData), m_valid(true) {}

	template<std::size_t N>
	explicit JSONEntry(const char(&inData)[N]) : m_data(std::string(inData)), m_valid(true) {}

	friend class JSONReader;
	friend class JSONWriter;


	//Primarily used for comparisons, this function returns iterators to the start and end of the key for this element
	std::pair<std::string::const_iterator, std::string::const_iterator> key() const;




	/*
	* A series of overloads to return data from the JSONEntry in the correct format, with some use of tags to share
	* common functionality.
	* Per the spec, this assumes narrowing conversions are a user error and not for us to clean up.
	*/
	template<typename T>
	struct as_helper {

		static inline T get(const std::string& src, tag_std_string) {
			return T(src.begin(), src.end());
		}

#ifdef __TCPLUSPLUS__
		static inline T get(const std::string& src, tag_delphi_string) {
			return src.c_str();
		}
#endif

		static inline T get(const std::string& src, tag_floating_point) {
			return static_cast<T>(std::atof(src.c_str()));
		}

		static inline T get(const std::string& src, tag_signed_int) {
			return static_cast<T>(std::atol(src.c_str()));
		}

		static inline T get(const std::string& src, tag_unsigned_int) {
			return static_cast<T>(std::strtoul(src.c_str(), NULL, 10));
		}

		static inline T get(const std::string& src, instance_of<bool>) {
			if (src.empty()) return false;
			switch (src[0]) {
			case 't':
			case 'T':
			case '1':
				return true;
			default:
				return false;
			}
		}

		static inline T get(const std::string& src, tag_char) {
			if (src.empty()) return '0';
			return static_cast<T>(src[0]);
		}

	};

};









#endif