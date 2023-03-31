#ifndef JSON_03_READER
#define JSON_03_READER


#include <string>
#include <vector>


#include "JSONEntry.h"

/*
*   A class to provide *read only* access to JSON data from a file, or from a string.
*	Access to each element is provided by operator[] and can itself be chained
*	e.g. Reader["Products"][0]["Product Code"].as<std::string>()
*	will return a std::string containing the product code of the first element of the Products array
*/


class JSONReader {
public:

	//"Normal" construction will be to read from an existing JSON file, so while this shares functionality with one of the
	//factory functions, a simple and idiomatic way to create these objects is still preferable to have.
	JSONReader(const std::string& filePathAndName);

	//As in the entry, we provide a "natural" type to index over with a specific overload of the most common use-case: literals.
	//Returning by const value is intentional - operator[] can be chained repeatedly but no element which starts off const should be
	//assignable
	const JSONEntry operator[](std::size_t index) const;
	const JSONEntry operator[](int index) const;

	//Conversely to how the Entry works internally, here we have to eat the cost of allocating a string to allow the algorithm to work
	const JSONEntry operator[](const std::string& index) const;

	template<std::size_t N>
	const JSONEntry operator[](const char(&index)[N]) const {
		return (*this)[std::string(index)];
	}

	bool valid() const;
	bool operator!() const;

	//Factory functions to create from different input
	static JSONReader createFromFile(const std::string& filePathAndName);
	static JSONReader createFromString(const std::string& stringData);


private:

	/*
	*  Potentially counter-intuitively, we use a sorted std::vector of elements to store our data.
	*  This is for a few reasons - firstly, JSONEntry needs to be aware of the key values used in order to
	*  facilitate arrays, so a map-like container would require duplication of data.
	*  Secondly, as data will not be added to the vector after construction, a well-designed sorting and retrieval
	*  setup can make up for, and potentially outperform, a tree-based container.
	*  Additionally, it allows O(1) lookup via operator[](std::size_t).
	*/
	std::vector<JSONEntry> m_data;
	bool                   m_valid;

	//Shared setup for all ctors
	void setup(const std::string& data);

	//We want a private default ctor for two reasons:
	//1. A default constructed instance would be meaningless as all data is read on construction
	//2. It allows internal processing from the factory functions to start with a blank slate
	JSONReader() : m_valid(true) {}

};
#endif