#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <tclap/CmdLine.h>

using namespace std;
namespace fs = std::filesystem;

enum CmdOptions {
	DIRECTORY,
	INPUT,
	OUTPUT,
	SORT_BY_ARTIST,
	SORT_BY_ALBUM
};

map<int, string> parseCmd(int argc, char** argv);
vector<string*> readGivenData(string input);
vector<string*> indexDirectory(string path);
void writeFoundData(string path, vector<string*> foundData);
bool sortByArtist(string* i, string* j) { return i[0] < j[0]; }
bool sortByAlbum(string* i, string* j) { return i[1] < j[1]; }

// TODO exception handling
int main(int argc, char** argv)
{
	vector<string*> givenData;		// A vector containing the data held in the input file
	map<int, string> givenOptions;	// A map of the command line options given by the user
	vector<string*> foundData;		// A vector containing all the data found in the file tree
	vector<string*> newData;		// A vector containing all the new data found in the file tree
	vector<string*> missingData;	// A vector containing all the read in data that was not found in the file tree

	givenOptions = parseCmd(argc, argv);

	// If the user provided an input file
	if (givenOptions[INPUT] != "")
	{
		givenData = readGivenData(givenOptions[INPUT]);
	}

	foundData = indexDirectory(givenOptions[DIRECTORY]);

	// Sort the data first
	if (givenOptions[SORT_BY_ARTIST] != "")
	{
		if (givenOptions[INPUT] != "")
		{
			sort(givenData.begin(), givenData.end(), sortByArtist);
		}

		sort(foundData.begin(), foundData.end(), sortByArtist);
	}

	else if (givenOptions[SORT_BY_ALBUM] != "")
	{
		if (givenOptions[INPUT] != "")
		{
			sort(givenData.begin(), givenData.end(), sortByAlbum);
		}

		sort(foundData.begin(), foundData.end(), sortByAlbum);
	}

	writeFoundData(givenOptions[OUTPUT], foundData);

	// If the user provided an input file
	if (givenOptions[INPUT] != "")
	{
		for (string* foundE : foundData)
		{
			// Boolean for determining if a matching artist-album combination was found in the read in data
			bool matchFound = false;

			// Loop through every element of the read in data
			for (string* s : givenData)
			{
				// If both the artist and album match
				if (s[0] == foundE[0] && s[1] == foundE[1])
				{
					// A match was found
					matchFound = true;
				}
			}

			// If no match was found
			if (!matchFound)
			{
				newData.push_back(foundE);
			}
		}

		// Print the new data to stdout
		// Header
		cout << "=====================NEW-DATA=====================\n";

		for (string* s : newData)
		{
			cout << "\"" << s[0] << "\"" << ',' << "\"" << s[1] << "\"" << endl;
		}

		cout << endl;

		// Find missing data
		// Loop through the given data
		for (string* givenE : givenData)
		{
			bool givenFound = false;

			for (string* foundE : foundData)
			{
				if (givenE[0] == foundE[0] && givenE[1] == foundE[1])
				{
					givenFound = true;
				}
			}

			if (!givenFound)
			{
				missingData.push_back(givenE);
			}
		}

		// Print the missing data to stdout
		// Header
		cout << "===================MISSING-DATA===================\n";

		for (string* s : missingData)
		{
			cout << "\"" << s[0] << "\"" << ',' << "\"" << s[1] << "\"" << endl;
		}

		cout << endl;
	}

	// Clear the used memory
	for (unsigned long i = 0; i < givenData.size(); i++)
	{
		delete[] givenData[i];
	}

	// Clear the used memory
	for (unsigned long i = 0; i < foundData.size(); i++)
	{
		delete[] foundData[i];
	}

	// We're done
	return 0;
}

map<int, string> parseCmd(int argc, char** argv)
{
	map<int, string> parsedCmd;

	// Try to use TCLAP to parse the command line
	try
	{
		// Main command line object for parsing
		TCLAP::CmdLine cmd("Music Directory Indexer", ' ', "1.0");

		// The output file flag
		TCLAP::ValueArg<string> outfile("o", "outfile", "The file to place the data into", false, "music-data.csv", "output CSV");

		// Add the output file flag to the main cmd object
		cmd.add(outfile);

		// The input file flag
		TCLAP::ValueArg<string> infile("i", "infile", "A CSV file to compare against when indexing the directory", false, "blank", "input CSV");

		// Add the input file flag to the main cmd object
		cmd.add(infile);

		// The directory to search
		TCLAP::UnlabeledValueArg<string> directory("directory", "The directory to index the folders of", false, ".", "directory path");

		// Add the directory to the main cmd object
		cmd.add(directory);

		// Whether to sort by artist
		TCLAP::SwitchArg sortArtist("r", "sort-artist", "Sort all output by artist", false);

		// Add it to the main cmd object
		cmd.add(sortArtist);

		// Whether to sort by album
		TCLAP::SwitchArg sortAlbum("l", "sort-album", "Sort all output by album", false);

		// Add it to the main cmd object
		cmd.add(sortAlbum);

		// Parse the command line
		cmd.parse(argc, argv);

		// Retrieve the proper values from the respective TCLAP object
		parsedCmd[DIRECTORY] = directory.getValue();
		parsedCmd[OUTPUT] = outfile.getValue();

		// If the user provided an input file
		if (infile.isSet())
		{
			// Get the name of the file
			parsedCmd[INPUT] = infile.getValue();
		}

		// If the user did not provide an input file
		else
		{
			// Set the file name to a null string
			parsedCmd[INPUT] = "";
		}

		if (sortArtist.getValue() && sortAlbum.getValue())
		{
			// Print an error message describing the issue                                                                                                                                                                                          
			cerr << "Invalid options given, can only sort by album OR artists, not both." << endl;

			// Print the usage statement                                                                                                                                                                                                            
			cmd.getOutput()->usage(cmd);

			throw exception();
		}

		if (sortArtist.getValue())
		{
			parsedCmd[SORT_BY_ARTIST] = "true";
		}

		else if (sortAlbum.getValue())
		{
			parsedCmd[SORT_BY_ALBUM] = "true";
		}
	}

	// Catch any exceptions TCLAP may throw
	catch(TCLAP::ArgException& e)
	{
		cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
		throw e;
	}

	// Return the parsed command line options
	return parsedCmd;
}

vector<string*> readGivenData(string path)
{
	vector<string*> readData;

	// The file stream for reading the input file
	ifstream inFile(path);

	// If the file could not be opened
	if (!inFile)
	{
		// Let the user know
		cerr << "Failed to open " << path << "." << endl;
		throw exception();
	}

	// String for reading in each line of the input file
	string workingLine;

	// Read in the first line of the file
	getline(inFile, workingLine);
	
	// Loop through the file until the end is reached or a blank line is reached
	while (!inFile.eof() && workingLine.compare("") != 0)
	{
		// Create a new string array for holding the read in values
		string* temp = new string[2];

		// String stream for parsing the read in line
		stringstream workingStream(workingLine);

		// Get the first character of the line
		char workingChar = workingStream.get();

		// If the character was a quotation mark
		if (workingChar == '"')
		{
			// Read in the next character
			workingChar = workingStream.get();

			// Loop until a second quotation mark is reached
			while (workingChar != '"')
			{
				// Add the current character to the first string of the array
				temp[0] += workingChar;
				// Read in the next character
				workingChar = workingStream.get();
			}

			// Burn the comma after the last quotation mark
			workingStream.get();
		}

		// If the read in character was not a quotation mark
		else
		{
			// Read the rest of the line into the first string of the array
			getline(workingStream, temp[0], ',');

			// Put the read in character back at the beginning of the string
			temp[0] = workingChar + temp[0];
		}

		// Read in the next character of the line
		workingChar = workingStream.get();

		// If the character was a quotation mark
		if (workingChar == '"')
		{
			// Read in the next character
			workingChar = workingStream.get();

			// Loop until a second quotation mark is reached
			while (workingChar != '"')
			{
				// Add the current character to the second string of the array
				temp[1] += workingChar;
				// Read in the next character
				workingChar = workingStream.get();
			}

			// Burn the comma after the last quotation mark if it exists
			workingStream.get();
		}

		// If the read in character was not a quotation mark
		else
		{
			// Read the rest of the line into the first string of the array
			getline(workingStream, temp[1], ',');

			// Put the read in character back at the beginning of the string
			temp[1] = temp[1].substr(1, temp[1].length() - 2);
		}

		// Add the read in line to the data vector
		readData.push_back(temp);

		// Read the next line from the file
		getline(inFile, workingLine);
	}

	// Close the input file
	inFile.close();

	return readData;
}


vector<string*> indexDirectory(string path)
{
	vector<string*> foundData;

	// Loop through every artist folder
	for (const auto & artist : fs::directory_iterator(path))
	{
		// Make sure that the current item is a folder
		if (is_directory(artist.path()))
		{
			// Loop through every album folder
			for (const auto & album : fs::directory_iterator(artist))
			{
				// Make sure that the current item is a folder
				if (is_directory(album.path()))
				{
					string* temp1 = new string[2];
					temp1[0] = artist.path().filename().string();
					temp1[1] = album.path().filename().string();
					foundData.push_back(temp1);
				}
			}
		}
	}

	return foundData;
}

void writeFoundData(string path, vector<string*> foundData)
{
	// The output file stream for writing the indexed data
	ofstream outFile(path);

	// If the output file could not be opened
	if (!outFile)
	{
		// Let the user know
		cerr << "Failed to open " << path << "." << endl;
		throw exception();
	}

	for (string* s : foundData)
	{
		// Write the artist and album as an entry in the output file
		outFile << "\"" << s[0] << "\"" << ',' << "\"" << s[1] << "\"" << endl;
	}

	// Close the output file
	outFile.close();
}
