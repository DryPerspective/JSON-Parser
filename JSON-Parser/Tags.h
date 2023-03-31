#ifndef TMP_03_TAGS
#define TMP_03_TAGS

#include <string>

#ifdef __TCPLUSPLUS__
#include <vcl.h>
#endif


/*
* The core of our TMP bag of tricks. This lightweight struct allows us to represent a concrete type without needing to construct it.
*/
template<typename T>
struct instance_of {
	typedef T type;
};

/*
* Tag classes. When the same behaviour is required in the same "family" of types, these allow us to categorise a generic type
* e.g. a function with an overload which accepts a tag_signed_int, and a call which uses instance_of<int> will match
* Don't allow overlap. E.g. one overload which accepts a tag_std_string and one which accepts tag_any_string will be ambiguous
*/
struct tag_signed_int {
	tag_signed_int() {}
	tag_signed_int(instance_of<short>) {}
	tag_signed_int(instance_of<int>) {}
	tag_signed_int(instance_of<long>) {}
};

struct tag_unsigned_int {
	tag_unsigned_int() {}
	tag_unsigned_int(instance_of<unsigned short>) {}
	tag_unsigned_int(instance_of<unsigned int>) {}
	tag_unsigned_int(instance_of<unsigned long>) {}
};

struct tag_any_int {
	tag_any_int() {}
	tag_any_int(instance_of<short>) {}
	tag_any_int(instance_of<int>) {}
	tag_any_int(instance_of<long>) {}
	tag_any_int(instance_of<unsigned short>) {}
	tag_any_int(instance_of<unsigned int>) {}
	tag_any_int(instance_of<unsigned long>) {}

};

struct tag_floating_point {
	tag_floating_point() {}
	tag_floating_point(instance_of<float>) {}
	tag_floating_point(instance_of<double>) {}
	tag_floating_point(instance_of<long double>) {}
};

struct tag_std_string {
	tag_std_string() {}
	tag_std_string(instance_of<std::string>) {}
	tag_std_string(instance_of<std::wstring>) {}
};

#ifdef __TCPLUSPLUS__
struct tag_delphi_string {
	tag_delphi_string() {}
	tag_delphi_string(instance_of<AnsiString>) {}
	tag_delphi_string(instance_of<UnicodeString>) {}
};
#endif

struct tag_any_string {
	tag_any_string() {}
	tag_any_string(instance_of<std::string>) {}
	tag_any_string(instance_of<std::wstring>) {}
#ifdef __TCPLUSPLUS__
	tag_any_string(instance_of<AnsiString>) {}
	tag_any_string(instance_of<UnicodeString>) {}
#endif
};

struct tag_char {
	tag_char() {}
	tag_char(instance_of<char>) {}
	tag_char(instance_of<wchar_t>) {}
};




#endif