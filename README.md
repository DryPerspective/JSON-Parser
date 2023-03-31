# JSON-Parser
A recent project for a client involved retrofitting a new service to existing older code, which required sending and receiving data over the web in JSON format. The code was written in the C++03 standard and the client didn't have a preexisting solution to write and parse JSON files, so this code was written as part of the project, tailored to that project's particular needs. 

## Usage
The JSONEntry class is the core element here, representing a single "data point" in a JSON, as a kind of proxy class. As any entry in a JSON can itself be an array, which can itself contain arrays of arbitrary depth, `operator[]` can be used and repeatedly chained to access deeper levels of of the data, with the underlying data accessible with the templated `as()` member function.

The JSONReader class provides *read-only* access to a particular JSON file, primarily from reading a file in the user's filesystem, but can also construct a JSON from a provided string. As the most common use-case is constructing from a file, this is the decision taken by the primary constructor for the class for simpler idiomatic use, however a factory function for constructing from a string is provided (with a matching function for files to maintain symmetry). As with the JSON entry class, `operator[]` is used to access data within the file.

The JSONWriter class allows the user to construct valid JSON data from inputs of various types, and write them to a string within the program to be used for other processes. Items and arrays are added in small pieces as the code goes along, and written to the desired output in one fell swoop by calling the appropriate member function.

A sample code snippet for this code follows:
```cpp
JSONReader js("MyFile.json);

//Read a data within the file
int LifeNumber = js["Puss in Boots"]["Number of Lives"].as<int>();

...

JSONWriter out;
out.add("Name","Puss in Boots");
out.startArray("Aliases");
out.addSimpleArrayItem("The Stabby Tabby");
out.addSimpleArrayItem("El Macho Gato");
out.addSimpleArrayItem("The Leche Whisperer);
out.endArray();
std::string fullJSON = out.getString();
```

The specification for this project took a soft approach on error handling - in the event of invalid data, either from an invalid index or invalid data in the file, the JSONEntry object returned will be in a well-defined "invalid" state, which can be queried with the `valid()` member function. It can also be queried via `if(!JSON)` in a similar syntax to checking the validity of pointers. Note, this is achieved via `operator!()` and not an implicit conversion to `bool`. This was designed primarily to avoid ambiguity between the designed `operator[](std::string)`, and the built-in `[]` operator attempting to do pointer math by implicit conversion around the base int types. As `explicit` type conversions are a C++11 feature, this ambiguity is largely unavoidable for conversions to built-in types, with all the implicit conversions they permit between themselves; however the use of `operator!` does also leave the design space open if some future update on a (relative to C++03) future standard wants to implement it.

## Notes on the code
This code is entirely conforming to the C++03 standard with no additional dependencies. This does unfortuantely mean that some elements of the code do feel the lack of certain features which were added in later standards, and as such a handful of methods take a more heavy-handed or inefficient approach than they ideally would. This code was written with forward compatibility in mind, and makes no use of any features which are deprecated or removed in later standards (up to C++20, in any case). One of the target platforms for this code was Embarcadero C++Builder, which uses Delphi-esque string types AnsiString and UnicodeString. These are supported for that platform, and conditionally included via preprocessor macro. There are some additional specifics to mention:

Some design choices for this code may seem unconventional - there is method to the apparent madness, such as some functions returning by `const` value. This serves a purpose - with C++03 being unable to distinguish between lvalue and rvalue qualification in member functions and the repeated chaining of `operator[]`, const qualification was the simplest and best approach to prevent writing to a temporary which seems to represent data within a persistent object. Or rather, that the potential line `myReader["People"][0]["Name"] = "Pickles";` should be explicitly compiler-forbidden rather than allowed but meaningless at runtime. 

The original intention was to allow the JSONWriter class (and, potentially JSONReader) to be able to modify entries within the data, with a simple and idiomatic `JSON[a][b] = newData;`. However, during development a particular compiler bug emerged in one of the platforms on which this code would be run on, where it was improperly unable to disambiguate `const` and non-`const` overloads. Being unable to work around this bug, as well as changes to the specification and simple time constraints, led JSONWriter to have a slightly clunkier interface than originally intended. This is unfortunate, but the groundwork is there within the class to build up to this interface design in a future update, if needed.

There is a limitation within this code - namely that JSON files which consist entirely of an unnamed array will not be properly parsed. This is earmarked as a known issue, however it was deemed sufficiently unlikely for the greater project at large to not be worth interrupting further project development to fix. Perhaps a future update will address this.
