#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

using namespace std;


//my version of string to int converter to make compiling on mauler possible
int my_stoi(string input){
	int answer;
	istringstream convert(input);

	if( !(convert >> answer))
		answer = 0;

	return answer;

}

// my string to int converter
string my_to_string(int input){

	string answer;
	ostringstream convert;
	convert << input;
	answer = convert.str();

	return answer;
}

// checks if string x already exists in a string vector
bool alreadyExists(vector<string> input, string x){
	for(int i = 0; i < input.size(); i++){
		if(input[i] == x)
			return true;
	}
	return false;
}

// gets the appropriate symbol adjustment from the symbol table using a random string
int retrieveSymbol(vector<string> symbolTable, string x){

	for(int i = 0; i < symbolTable.size(); i+=2){
		if(symbolTable[i] == x){
				int answer = my_stoi(symbolTable[i+1]);
			
			return answer;
		}
	}
	return 0;
}

// determines which module a symbol is defined in
int whichModule(vector<string> input, string symbol){
	
	int module = 0;
	int flag = 0;

	int i = 0;
	while(i < input.size()){

		// iterate to end of line 3 times until you modify module number
		int temp = my_stoi(input[i]);
		for(int j = 0; j <= (2 * temp); j++){
			if(flag == 0 && input[i] == symbol){
				return module;
			}
			i++;
		}

		flag++;
		if(flag >2){
			module++;
			flag = 0;
		}
	}

	return module;

}

// prints and returns a symbol table given input
vector<string> getSymbolTable(vector<string> input){
	
	int flag = 1, module = 0, moduleCount = 0;

	vector<string> definition, text, symbolTable, symbols;

	int i = 0;
	while(i < input.size()){

		// gather info for each module
		int temp = my_stoi(input[i]);
		for(int j = 0; j <= (2 * temp); j++){

			if(flag == 1){
				if(j > 0)
					definition.push_back(input[i]);	
			}

			if(flag > 2){
				if(j > 0)
					text.push_back(input[i]);

				if(j == 2*temp)
					flag = 0;
			}
			i++;
		}

		// check to see if we have reached end of module based on which line we are on
		if (flag == 0){

			for(int z = 0; z < definition.size(); z+=2){

				int multipleFlag = 0;
				int outFlag = 0;

				// first check if any definition address exceeds the module size, use value 0 instead
				int temp = my_stoi(definition[z+1]);
				if(temp + 1 > text.size() / 2){
					outFlag = 1;
					definition[z+1] = "0";

				}

				// determine appropriate absolute address for the current symbol (string to int, int to string)
				int tempNum = my_stoi(definition[z+1]) + moduleCount;
				string number = my_to_string(tempNum);
				definition[z+1] = number;

				// check if symbol has already been defined and record current symbol if it has not
				if(alreadyExists(symbols, definition[z])){
					multipleFlag = 1;
				}
				else{
					symbols.push_back(definition[z]);
				}

				// printing symbol table and errors
				if(multipleFlag == 0 && outFlag == 0){
					cout << definition[z] << "=" << definition[z+1] << endl;
				}
				if(multipleFlag == 1){
					cout << definition[z] << " Error: This variable has already been defined; first value used" << endl;
				}
				if(outFlag == 1 && multipleFlag != 1){
					ostringstream oss;
					oss << " Error: The value of " << definition[z] << " is outside module " << module << "; zero (relative) used";
					string error = oss.str();

					cout << definition[z] << "=" << definition[z+1] << error << endl;
				}

				// load up symbol table with appropriate 
				if(multipleFlag == 0){
					symbolTable.push_back(definition[z]);
					symbolTable.push_back(definition[z+1]);
				}

			}
			// adjust number of modules passed
			moduleCount += text.size() / 2;

			// new module, so clear definition and text lists
			definition.clear();
			text.clear();
		}
		// keep track of module number for error printing
		if (flag == 0)
			module++;
		
		// keep track of which line we are on in the module
		flag++;
	}

	return symbolTable;
}

// prints memory map along with any errors
void printMemoryMap(vector<string> input, vector<string> symbolTable){


	int flag = 1, moduleCount = 0, m = 0, usedNotDefinedFlag = 0, outOfRangeFlag = 0;
	string notDefined;
	vector<string> notTypeE, useChain, use, text, allUses;

	int i = 0;
	while(i < input.size()){

		// same iteration as getSymbolTable
		int temp = my_stoi(input[i]);
		for(int j = 0; j <= (2 * temp); j++){

			if(flag == 2){
				if(j > 0)
					use.push_back(input[i]);	
			}

			if(flag > 2){
				if(j > 0)
					text.push_back(input[i]);

				if(j == 2*temp)
					flag = 0;
			}
			i++;
		}

		// once all information for a module has been gathered, make adjustments
		if (flag == 0){

			for(int z = 0; z < use.size(); z+=2){

				// error checks
				usedNotDefinedFlag = 0;
				outOfRangeFlag = 0;

				// symbol is used but not defined
				if(!alreadyExists(symbolTable, use[z])){
					usedNotDefinedFlag = 1;
					notDefined = use[z];
				}

				// perform necessary adjustments
				int moduleIndex = my_stoi(use[z+1]);
				int moduleAddress = my_stoi(text[moduleIndex*2+1]);

				while(moduleAddress % 1000 != 777){

					string x = my_to_string(moduleIndex*2);
					if(text[moduleIndex*2] != "E"){
						notTypeE.push_back(x); 
						notTypeE.push_back(text[moduleIndex*2]);
					}
					useChain.push_back(x);
					useChain.push_back(text[moduleIndex*2]);
					

					// keep track of next use index in the chain
					int nextIndex = moduleAddress % 1000;
					int nextAddress = my_stoi(text[nextIndex*2+1]);

					// adjust address
					if(nextIndex != 777){
						int x = retrieveSymbol(symbolTable, use[z]);
						moduleAddress = (moduleAddress / 1000 * 1000) + x;
						string temp = my_to_string(moduleAddress);
						text[moduleIndex*2+1] = temp;
					}

					// if next address is out of range
					if( (nextAddress % 1000)  > text.size() / 2 && (nextAddress % 1000) != 777 ) {
						moduleAddress = nextAddress;
						moduleIndex = nextIndex;
						outOfRangeFlag = 1;
						break;
					}

					moduleAddress = nextAddress;
					moduleIndex = nextIndex;

				}
				// get type info for last address
				string xy = my_to_string(moduleIndex*2);
				if(text[moduleIndex*2] != "E"){
					notTypeE.push_back(xy); 
					notTypeE.push_back(text[moduleIndex*2]);
				}
				useChain.push_back(xy);
				useChain.push_back(text[moduleIndex*2]);

				// finally, adjust the last address in the list (777 or out of bounds)
				int x = retrieveSymbol(symbolTable, use[z]);
				moduleAddress = (moduleAddress / 1000 * 1000) + x;
				string temp = my_to_string(moduleAddress);
				text[moduleIndex*2+1] = temp;

				// to check if any symbol is defined but not used, keep track of all uses
				allUses.push_back(use[z]);
			}

			// print memory map and errors loop
			for(int n = 0; n < text.size(); n+=2){

				if(text[n] == "R"){
					int temp = my_stoi(text[n+1]) + moduleCount;
					string newAddress = my_to_string(temp);
					text[n+1] = newAddress;
				}

				cout << setw(2) << m << ":  " << text[n+1];

				for(int i = 0; i < notTypeE.size(); i+=2){
					int temp = my_stoi(notTypeE[i]);
					if(temp == n){
						cout << " Error: " << notTypeE[i+1] << " type address on use chain; treated as E type.";
					}
				}

				if(outOfRangeFlag == 1 ){
					cout << " Error: Pointer in use chain exceeds module size; chain terminated";
					outOfRangeFlag = 0;
				}


				if(text[n] == "E"){
					string x = my_to_string(n);
					if(!alreadyExists(useChain, x) ){
						cout << " Error: E type address not on use chain; treated as I type.";
					}
				}


				if(usedNotDefinedFlag == 1){
					cout << " Error: " << notDefined << " is not defined; zero used"; 
				}

				cout << endl;

				m++;
			}
			usedNotDefinedFlag = 0;
			outOfRangeFlag = 0;

			moduleCount += text.size() / 2;

			// reset lists for a new module
			use.clear();
			text.clear();
			notTypeE.clear();
			useChain.clear();
		}
		// keeps track of changing lines
		flag++;
	}


	cout << endl;
	// once done, use allUses list to make sure no symbol defined was not used
	for(int i = 0; i < symbolTable.size(); i+=2){
		if(!alreadyExists(allUses,symbolTable[i])){
			int module = whichModule(input, symbolTable[i]);
			cout << "Warning: " << symbolTable[i] << " was defined in module " << module << " but never used." << endl;
		}
	}
}

// take in standard input and get symbol table and print memory map
int main() {

	// create temporary string and string vector to hold the input
	string x;
	vector<string> input;
	
	// until input reaches end, add each piece of the 
	// input (delimited by spaces) to the vector
	while (cin >> x ) {
		input.push_back(x);
    }

    cout << "Symbol Table" << endl;
    // get symbol table from input vector and print it out
    vector<string> symbolTable = getSymbolTable(input);

    cout << endl;

    cout << "Memory Map" << endl;
    // use symbol table to get memory map
    printMemoryMap(input, symbolTable);


	return 0;
}
