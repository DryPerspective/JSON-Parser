#include <iostream>
#include <random>

#include "JSONEntry.h"
#include "JSONWriter.h"
#include "JSONReader.h"

JSONWriter getNamesJSON() {
	JSONWriter out;
	out.add("Name", "The Doctor");
	out.startArray("Aliases");
	out.addSimpleArrayItem("John Smith");
	out.addSimpleArrayItem("Johann Schmidt");
	out.addSimpleArrayItem("Theta Sigma");
	out.addSimpleArrayItem("Brother Lungbarrow");
	out.addSimpleArrayItem("The Other");
	out.addSimpleArrayItem("The Oncoming Storm");
	out.endArray();
	out.add("Age", 1200);
	out.add("Planet of Origin", "Gallifrey");
	out.add("Chapter", "Prydonian");
	out.add("Fugitive", true);

	out.writeToFile("Names.json");
	return out;
}

bool generateNames() {
	return getNamesJSON().valid();
}

bool validateNames() {
	JSONReader Names("Names.json");
	if (!Names) {
		return false;
	}
	bool nameCheck = Names["Name"].as<std::string>() == "The Doctor";
	bool ageCheck = Names["Age"].as<int>() == 1200;

	int randomAlias = std::rand() % 6;
	std::string Aliases[6] = { "John Smith","Johann Schmidt","Theta Sigma","Brother Lungbarrow","The Other","The Oncoming Storm" };


	bool aliasCheck = Names["Aliases"][randomAlias].as<std::string>() == Aliases[randomAlias];

	return nameCheck && ageCheck && aliasCheck;
}

bool deepArrayCheck() {
	JSONReader qz("QuizQuestion.json");
	if (!qz) return false;

	bool check = qz["quiz"]["maths"]["q1"]["options"][2].as<int>() == 12;

	return check;
}

bool copyUsers() {
	JSONReader Users("Users.json");
	if (!Users) return false;


	JSONWriter out;

	out.startArray("users");
	for (int i = 0; i < 5; ++i) {
		out.startArrayItem();
		//Two different ways to make the copy:
		out.add("userId", Users["users"][i]["userId"].as<std::string>());
		//Can also accept an entry in another list directly.
		out.add(Users["users"][i]["firstName"]);
		//Can also validate
		if (!Users["users"][i]["lastName"]) return false;
		out.add(Users["users"][i]["lastName"]);


		out.add(Users["users"][i]["phoneNumber"]);
		out.add("emailAddress", Users["users"][i]["emailAddress"].as<std::string>());
		out.endArrayItem();
	}
	out.endArray();

	out.writeToFile("UsersOut.json");

	if (!out) return false;

	//Validate all data matches
	bool allValid = true;
	JSONReader customUsers("UsersOut.json");
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 5; ++j) {
			if (customUsers["users"][i][j] != Users["users"][i][j]) {
				allValid = false;
				break;
			}
		}
		if (!allValid) break;
	}

	return allValid;
}

bool matchFromString() {

	JSONReader hardCoded = JSONReader::createFromString(getNamesJSON().getString(true));
	JSONReader file = JSONReader::createFromFile("Names.json");

	if (!hardCoded || !file) return false;

	if (file["Name"] != hardCoded["Name"]) return false;
	if (file["Aliases"] != hardCoded["Aliases"]) return false;
	if (file["Age"] != hardCoded["Age"]) return false;
	if (file["Planet of Origin"] != hardCoded["Planet of Origin"]) return false;
	if (file["Chapter"] != hardCoded["Chapter"]) return false;
	if (file["Fugitive"] != hardCoded["Fugitive"]) return false;

	return true;
}

bool testAccessSpecifier() {
	JSONReader qz("QuizQuestion.json");

	std::string sport = "sport";
	std::string question = "question";
	//All of these should return the exact same data
	std::string allLiterals = qz["quiz"]["sport"]["q1"]["question"].as<std::string>();
	std::string stringLookup = qz["quiz"][sport]["q1"][question].as<std::string>();
	std::string mixedLookup = qz[0]["sport"]["q1"][question].as<std::string>();


	bool test = (
		//If this returns valid and all the strings match, then we know that the test succeeded and didn't result in invalid data being equally invalid
		qz[0]["sport"]["q1"][question].valid() &&
		allLiterals == stringLookup &&
		allLiterals == mixedLookup
		);

	return test;	
	
}

std::string getPassFail(bool b) {
	if (b) return "\t\tPASSED\n";
	else return "\t\tFAILED\n";
}

int main() {

	std::cout << "Starting tests:\n";
	std::cout << "Generating names file: " << getPassFail(generateNames());
	std::cout << "Validating names file: " << getPassFail(validateNames());
	std::cout << "Checking deep arrays: " << getPassFail(deepArrayCheck());
	std::cout << "Copying file contents: " << getPassFail(copyUsers());
	std::cout << "Testing creation from string:" << getPassFail(matchFromString());
	std::cout << "Testing mixed access types: " << getPassFail(testAccessSpecifier());




	return 0;
}