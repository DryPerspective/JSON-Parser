//---------------------------------------------------------------------------

#ifndef JSON_03_WRITER
#define JSON_03_WRITER
//---------------------------------------------------------------------------
#include <vector>
#include <string>
#include <ios>
#include <sstream>

#include "JSONEntry.h"
#include "Tags.h"

class JSONWriter{

public:
	 JSONWriter() : m_arrayDepth(0), m_valid(true) {};

     //Templated to allow non-string types to make it into the JSON
	 template<typename T>
	 void add(const std::string& key, const T& value){
		m_data.push_back(JSONEntry("\"" + key + "\":" + add_helper<T>::get(value, instance_of<T>())));
	 }

	 void add(const JSONEntry& newElement);
	 //Start a full array, i.e. insert a [ or ] into the current file
	 void startArray(const std::string& key);
	 void endArray();

	 //For "simple" arrays, e.g. "Aliases" : ["Theta Sigma", "Brother Lungbarrow"]
	 //void addSimpleArrayItem(const std::string& input);
	 template<typename T>
	 void addSimpleArrayItem(const T& newItem){
		if(m_arrayDepth == 0 || m_data.empty()) return;

		std::string& mostRecentTerm = m_data[m_data.size() - 1].m_data;
		std::size_t lastTokenIndex = mostRecentTerm.find_last_not_of(" \t\n\r\b");
		if(lastTokenIndex == std::string::npos) return;
		if(mostRecentTerm[lastTokenIndex] != '[') mostRecentTerm += ",";

		mostRecentTerm+= add_helper<T>::get(newItem, instance_of<T>());

	 }


	 //For compound array items, e.g. "Products" : [{"Name" : "SomeProduct", "Price" : 5},{"Name" : "SomeOtherProduct", "Price" : 6}]
	 //The within these blocks, calls to add() should be made, not addSimpleArrayItem
	 void startArrayItem();
	 void endArrayItem();

	 JSONEntry& operator[](std::size_t index);
	 JSONEntry& operator[](int index);
	 JSONEntry& operator[](const std::string& index);

	 void writeToFile(const std::string& filePathAndName, std::ios_base::openmode openArgs = std::ios_base::out);
	 std::string getString(bool removeWS = false);

	 bool valid();
     bool operator!();


private:

	std::string processData();

	bool                    m_valid;
	std::size_t             m_arrayDepth;
	std::vector<JSONEntry> 	m_data;

	template<typename T>
	struct add_helper{
		//String types;
		static inline std::string get(const T& in, tag_std_string){
			return "\"" + std::string(in.begin(),in.end()) + "\"";
		}
		#ifdef __TCPLUSPLUS__
		static inline std::string get(const T& in, tag_delphi_string){
			return "\"" + std::string(in.begin(),in.end()) + "\"";
		}
		#endif
		template<std::size_t N>
		static inline std::string get(const T& in, instance_of<char[N]>){
            return std::string("\"") + in + "\"";
		}

		//Numerical types
		static inline std::string get(const T& in, tag_any_int){
			std::stringstream sstr;
			sstr << in;
			std::string out;
			sstr >> out;
			return  out;
		}
		static inline std::string get(const T& in, tag_floating_point){
			std::stringstream sstr;
			sstr << in;
			std::string out;
			sstr >> out;
			return out;
		}

		//Misc types
		static inline std::string get(const T& in, instance_of<char>){
			return std::string("\"") + in + "\"";
		}
		static inline std::string get(const T& in, instance_of<bool>){
			if(in) return "true";
			else return "false";
		}

	};

};
#endif