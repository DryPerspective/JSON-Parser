#ifndef JSON_ELEMENT_03
#define JSON_ELEMENT_03

#include <string>
#include <sstream>

#include "Tags.h"

namespace json {

	namespace detail {

		/*
		* A struct used to convert an input piece of data (T) into a string which can be used as a value for a JSON
		*/
		template<typename T>
		struct add_helper {
			//String types;
			static inline std::string get(const T& in, tag_std_string) {
				return "\"" + std::string(in.begin(), in.end()) + "\"";
			}
#ifdef __TCPLUSPLUS__
			static inline std::string get(const T& in, tag_delphi_string) {
				return "\"" + std::string(in.begin(), in.end()) + "\"";
			}
#endif
			template<std::size_t N>
			static inline std::string get(const T& in, instance_of<char[N]>) {
				return std::string("\"") + in + "\"";
			}

			//Numerical types
			static inline std::string get(const T& in, tag_any_int) {
				std::stringstream sstr;
				sstr << in;
				std::string out;
				sstr >> out;
				return  out;
			}
			static inline std::string get(const T& in, tag_floating_point) {
				std::stringstream sstr;
				sstr << in;
				std::string out;
				sstr >> out;
				return out;
			}

			//Misc types
			static inline std::string get(const T& in, instance_of<char>) {
				return std::string("\"") + in + "\"";
			}
			static inline std::string get(const T& in, instance_of<bool>) {
				if (in) return "true";
				else return "false";
			}

		};
	}


	//A single element of a JSON. One key and one value
	class JSONElement
	{

	public:


		template<typename T>
		JSONElement(const std::string& key, const T& val) : m_valid(true), m_value(detail::add_helper<T>::get(val, instance_of<T>())) {
			putKeyInQuotes(key);
		}

		template<std::size_t N>
		JSONElement(const std::string& key, const char(&val)[N]) : m_valid(true) {
			putKeyInQuotes(key);
			if (isNotSimpleValue(val)) m_value = val;
			else m_value = '"' + std::string(val) + '"';
		}

		JSONElement(const std::string& key, const std::string& val);


		explicit JSONElement(bool valid) : m_key(""), m_value(""), m_valid(valid) {}

		std::string key() const;
		std::string val() const;
		bool valid() const;
		bool operator!() const;

		bool isList() const;


		//The value may contain a list, so we provide accessors
		JSONElement operator[](std::size_t index) const;
		JSONElement operator[](int index) const;				//Int overload to distinguish integer literals from pointer literals. It just feeds the std::size_t
		JSONElement operator[](const std::string& index) const;

		static JSONElement createFromString(const std::string& input);
#ifdef __TCPLUSPLUS__
		static JSONElement createFromString(const UnicodeString& input);
#endif
		static JSONElement createFromFile(const std::string& filePathAndName);

		std::string getString() const;


		//Our main function to get the value from an entry, templated to allow quick and easy conversion
		template<typename T>
		T as() const {
			if (!m_valid) return as_helper<T>::get("0", instance_of<T>());
			/*
			* In case you are unfamiliar with C++03 TMP techniques:
			* To elaborate on the trick here - passing an instance_of<T> forces the compiler to look for an appropriate get()
			* function in the as_helper struct. Since the overloads use tags which are constructble from specific instance_of
			* types, it becomes a game of matching the instance_of<T> to a constructor of one of those tags.
			* As we have carefully designed are tags and overloads such that a given <T> will match no more than one constructor,
			* the result is either a successful match to the function of the correct type, or a compiler failure if the user asks for
			* an unsupported type.
			*/
			return as_helper<T>::get(trim(m_value, " \t\r\n\b\""), instance_of<T>());
		}


	private:
		std::string m_key;
		std::string m_value;
		bool		m_valid;

		static std::string trim(std::string in, const char* letters = "\n\t\r\b ");

		//Used for construction to put the key value in quotes if the user did not include them.
		void putKeyInQuotes(const std::string& in);
		//Used to detect if the string provided to the constructor is already some complex JSONElement format, e.g. a list
		bool isNotSimpleValue(const std::string& in);

		/*
		* A series of overloads to return data from the JSONElementEntry in the correct format, with some use of tags to share
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

}
#endif