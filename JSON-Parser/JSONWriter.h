//---------------------------------------------------------------------------

#ifndef JSON_03_WRITER
#define JSON_03_WRITER
//---------------------------------------------------------------------------
#include <vector>
#include <string>
#include <ios>
#include <sstream>

#include "JSONElement.h"
#include "Tags.h"

namespace json {

	class JSONWriter {

	public:
		JSONWriter() : m_arrayDepth(0), m_valid(true) {};

		//Templated to allow non-string types to make it into the JSONriter
		template<typename T>
		void add(const std::string& key, const T& value) {
			m_data.push_back("\"" + key + "\":" + detail::add_helper<T>::get(value, instance_of<T>()));
		}

		void add(const JSONElement& newElement);
		//Start a full array, i.e. insert a [ or ] into the current file
		void startArray(const std::string& key);
		void endArray();

		//For "simple" arrays, e.g. "Aliases" : ["Theta Sigma", "Brother Lungbarrow"] - call startArray then add each item through this
		//void addSimpleArrayItem(const std::string& input);
		template<typename T>
		void addSimpleArrayItem(const T& newItem) {
			if (m_arrayDepth == 0 || m_data.empty()) return;

			std::string& mostRecentTerm = m_data[m_data.size() - 1];
			std::size_t lastTokenIndex = mostRecentTerm.find_last_not_of(" \t\n\r\b");
			if (lastTokenIndex == std::string::npos) return;
			if (mostRecentTerm[lastTokenIndex] != '[') mostRecentTerm += ",";

			mostRecentTerm += detail::add_helper<T>::get(newItem, instance_of<T>());

		}


		//For compound array items, e.g. "Products" : [{"Name" : "SomeProduct", "Price" : 5},{"Name" : "SomeOtherProduct", "Price" : 6}]
		//The within these blocks, calls to add() should be made, not addSimpleArrayItem
		void startArrayItem();
		void endArrayItem();

		void writeToFile(const std::string& filePathAndName, std::ios_base::openmode openArgs = std::ios_base::out);
		std::string getString(bool removeWS = false);

		bool valid();
		bool operator!();


	private:

		std::string processData();

		bool                    m_valid;
		std::size_t             m_arrayDepth;
		std::vector<std::string> 	m_data;



	};

}
#endif
