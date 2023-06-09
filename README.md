# JSON-Parser
A recent project for a client involved retrofitting a new service to existing older code, which required sending and receiving data over the web in JSON format. The code was written in the C++03 standard and the client didn't have a preexisting solution to write and parse JSON files, so this code was written as part of the project, tailored to that project's particular needs. 

## Usage
The JSONElement class represents a piece of JSON data. This can be an entire JSON in itself, or any arbitrary subdivision of it. As any entry in a JSON can itself be an array, which can itself contain arrays of arbitrary depth, `operator[]` can be used and repeatedly chained to access deeper levels of of the data, with the underlying data accessible with the templated `as()` member function. It can be constructed from a key-value pair, or from an existing string or file.

The JSONWriter class allows the user to construct valid JSON data from inputs of various types, and write them to a string within the program to be used for other processes. Items and arrays are added in small pieces as the code goes along, and written to the desired output in one fell swoop by calling the appropriate member function.

A sample code snippet for this code follows:
```cpp
json::JSONElement js = json::JSONElement::createFromFile("MyFile.json);

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

The specification for this project took a somewhat soft approach on error handling - in the event of certain logically impossible JSON format, certain accessors will throw. However, out-of-bounds access and other program-level user errors such as trying to divide up some JSON which is not a list will result in an object in a well-defined "invalid" state, which can be queried with the `valid()` member function. It can also be queried via `if(!JSON)` in a similar syntax to checking the validity of pointers. Note, this is achieved via `operator!()` and not an implicit conversion to `bool`. This was designed primarily to avoid ambiguity between the designed `operator[](std::string)`, and the built-in `[]` operator attempting to do pointer math by implicit conversion around the base int types. As `explicit` type conversions are a C++11 feature, this ambiguity is largely unavoidable for conversions to built-in types, with all the implicit conversions they permit between themselves; however the use of `operator!` does also leave the design space open if some future update on a (relative to C++03) future standard wants to implement it.

## Notes on the code
This code is entirely conforming to the C++03 standard with no additional dependencies. This does unfortuantely mean that some elements of the code do feel the lack of certain features which were added in later standards, and as such a handful of methods take a more heavy-handed or inefficient approach than they ideally would. This code was written with forward compatibility in mind, and makes no use of any features which are deprecated or removed in later standards (up to C++20, in any case). One of the target platforms for this code was Embarcadero C++Builder, which uses Delphi-esque string types AnsiString and UnicodeString. These are supported for that platform, and conditionally included via preprocessor macro. There are some additional specifics to mention:

The original intention was to allow the JSON objects to be able to modify entries within the data, with a simple and idiomatic `JSON[a][b] = newData;`. However, during testing a compiler bug was revealed in one of the environments this software was written for (Embarcadero's C++Builder "classic" compiler) which meant that such design would be impossible without opening the door to undefined behaviour which could not be properly detected at compile time or runtime. As such, the safest workaround necessitated the JSONWriter class and an interface which is clunkier than I would like.
