#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include "tclap/CmdLine.h"

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char** argv)
{
	string dir;					// The directory to index
	string output;				// The name of the output file
	string input;				// The name of the input file (if provided)
	bool infileGiven = false;	// Whether or not an input file was given
	vector<string*> givenData;	// A vector containing the data held in the input file

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

		// Parse the command line
		cmd.parse(argc, argv);

		// Retrieve the proper values from the respective TCLAP object
		dir = directory.getValue();
		output = outfile.getValue();
		infileGiven = infile.isSet();

		// If the user provided an input file
		if (infileGiven)
		{
			// Get the name of the file
			input = infile.getValue();
		}
	}

	// Catch any exceptions TCLAP may throw
	catch(TCLAP::ArgException& e)
	{
		cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
		return 1;
	}

	// If the user provided an input file
	if (infileGiven)
	{
		// The file stream for reading the input file
		ifstream inFile(input);

		// If the file could not be opened
		if (!inFile)
		{
			// Let the user know
			cerr << "Failed to open " << input << "." << endl;
			return 1;
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
			givenData.push_back(temp);

			// Read the next line from the file
			getline(inFile, workingLine);
		}

		// Close the input file
		inFile.close();
	}

	// The output file stream for writing the indexed data
	ofstream outFile(output);

	// If the output file could not be opened
	if (!outFile)
	{
		// Let the user know
		cerr << "Failed to open " << output << "." << endl;
		return 1;
	}

	// Loop through every artist folder
	for (const auto & artist : fs::directory_iterator(dir))
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
					// Write the artist and album as an entry in the output file
					outFile << artist.path().filename() << ',' << album.path().filename() << endl;

					// If the user provided an input file
					if (infileGiven)
					{
						// Boolean for determining if a matching artist-album combination was found in the read in data
						bool matchFound = false;

						// Loop through every element of the read in data
						for (string* s : givenData)
						{
							// If both the artist and album match
							if (s[0] == artist.path().filename().string() && s[1] == album.path().filename().string())
							{
								// A match was found
								matchFound = true;
							}
						}

						// If no match was found
						if (!matchFound)
						{
							// Print the unmatched artist and album entry as it would appear in the output file
							cout << artist.path().filename() << ',' << album.path().filename() << endl;
						}
					}
				}
			}
		}
	}

	// Close the output file
	outFile.close();

	// Clear the used memory
	for (unsigned long i = 0; i < givenData.size(); i++)
	{
		delete[] givenData[i];
	}

	// We're done
	return 0;
}
