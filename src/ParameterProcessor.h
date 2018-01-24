/*
 * ParameterProcessor.h
 *
 *  Created on: Dec 5, 2014
 *      Author: balyo
 */

#ifndef PARAMETERPROCESSOR_H_
#define PARAMETERPROCESSOR_H_

#include "string.h"
#include <map>
#include <string>
#include <iostream>
#include "stdlib.h"
using namespace std;


/*
Example: ./program -t=5 input.sas -d -h=fast

int main(char** argv, int argc) {
	ParameterProcessor pp;
	pp.init(argc, argv);


	pp.getFilename() -> "input.sas"
	pp.isSet("d") -> true
	pp.isSet("x") -> false
	pp.getIntParam("t", 7) -> 5
	pp.getIntParam("u", 10) -> 10
}
*/
class ParameterProcessor {
private:
	map<string, string> params;
	char* filename;
public:
	ParameterProcessor() {
		filename = nullptr;
	}
	void init(int argc, char** argv) {
		for (int i = 1; i < argc; i++) {
			char* arg = argv[i];
			if (arg[0] != '-') {
				filename = arg;
				continue;
			}
			char* eq = strchr(arg, '=');
			if (eq == nullptr) {
				params[arg+1];
			} else {
				*eq = 0;
				char* left = arg+1;
				char* right = eq+1;
				params[left] = right;
			}
		}
	}

	const char* getFilename() {
		return filename;
	}

	void printParams() {
		for (map<string,string>::iterator it = params.begin(); it != params.end(); it++) {
			if (it->second.empty()) {
				cout << it->first << ", ";
			} else {
				cout << it->first << "=" << it->second << ", ";
			}
		}
		cout << "\n";
	}

	void setParam(const char* name) {
		params[name];
	}

	void setParam(const char* name, const char* value) {
		params[name] = value;
	}

	bool isSet(const string& name) {
		return params.find(name) != params.end();
	}

	string getParam(const string& name, const string& defaultValue) {
		if (isSet(name)) {
			return params[name];
		} else {
			return defaultValue;
		}
	}

	string getParam(const string& name) {
		return getParam(name, "ndef");
	}

	int getIntParam(const string& name, int defaultValue) {
		if (isSet(name)) {
			return atoi(params[name].c_str());
		} else {
			return defaultValue;
		}
	}

};

#endif /* PARAMETERPROCESSOR_H_ */
