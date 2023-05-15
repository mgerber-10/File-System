#include "Wad.h"

/*___________________
	CONSTRUCTOR
  ___________________
*/

// Initializes rootNode of file tree with name = "/", offset = 0, length = 0, and parent = nullptr
Wad::Wad() 
{
	root = new treeNode("/", 0, 0, nullptr);
}


/*______________________________
	HELPER FUNCTION DEFINITIONS
  ______________________________
*/

// Helper function to load WAD file from specified path & set magic variable to specified value
void Wad::Load(string magic, const string& path) 
{
	this->magic = magic;
	this->wadFile = path;
}

// Helper function to load contents of each node in a tree from a binary file
void Wad::loadNodes(treeNode* currNode) 
{
	if (currNode->length != 0) {
		ifstream inFile(wadFile, ios::binary);	// open binary file for reading
		if (inFile.is_open()) {
			// If file is open, seek to offset specified by currNode and read node's contents from file
			inFile.seekg(currNode->offset);
			inFile.read(currNode->contents, currNode->length);
		}
		inFile.close();
	}
	
	// Recursively load contents of each child node in the tree
	for (int i = 0; i < currNode->children.size(); i += 1) {
		loadNodes(currNode->children[i]);
	}
}

// Helper function to recursively delete all nodes and children in the file tree, starting from "currNode"
void Wad::clearNodes(treeNode* currNode) 
{
	if (currNode != nullptr) {
		vector<treeNode*> children = currNode->children;
		for (int i = 0; i < children.size(); i += 1) {
			clearNodes(children.at(i));
		}
		delete[] currNode->contents;
		delete currNode;
	}
}

// Helper function to add a "new" file node to the file tree as child of specified "directory" node
void Wad::addFileNode(treeNode* directory, string name, int offset, int length) 
{
	if (directory != nullptr) {
		treeNode* newFile = new treeNode(name, offset, length, directory);
		directory->children.push_back(newFile);
	}
}

// Helper function to split filepath string into individual files/directories and storing them as separate verctors
void Wad::splitPath(const string& path, vector<string>& rawPath) 
{
	int i = 0;
	rawPath.push_back("/");
	while (i < path.size()) {
		string temp;
		while (i < path.size() && path.at(i) != '/') {
			temp.push_back(path.at(i));
			i += 1;
		}
		if (temp.size() > 0) {
			rawPath.push_back(temp);
		}
		i += 1;
	}
}

// Helper function to recursively search the tree for a file or directory node with the specified "filename" starting at node "file"
Wad::treeNode* Wad::searchDirectory(string fileName, treeNode* file) 
{
	if (file->name == fileName) {
		return file;
	}
	else {
		vector<treeNode*> children = file->children;
		treeNode* tempNode;
		for (int i = 0; i < children.size(); i += 1) {
			string currFile = children.at(i)->name;
			if (currFile == fileName) {
				return children.at(i);
			}
			else {
				tempNode = searchDirectory(fileName, children.at(i));
				if (tempNode != nullptr) {
					return tempNode;
				}
			}
		}
	}
	return nullptr;
}

// Helper function to return root of the file tree
Wad::treeNode* Wad::getRoot() 
{
	return root;
}

// Helper function to extract the name of the directory from file string
string Wad::getDirectoryName(string& file) 
{
	int i = 0;
	string name;
	while (file.at(i) != '_') {
		name.push_back(file.at(i));
		i += 1;
	}
	return name;
}

// Helper function to check if the file represents a map marker (strints of form "E#M#"")
bool Wad::isMapMarker(string& file) 
{
	if (file.at(0) == 'E' && isdigit(file.at(1)) && file.at(2) == 'M' && isdigit(file.at(3))){ return true; }
	else{ return false;}
}

// Helper function to check if the current file is the start of the directory
bool Wad::isDirectoryStart(string& file) 
{
	string startString = "_START";
	size_t found = file.find(startString);
	if (found != string::npos && found == file.size()-6) {return true;}
	else {return false;}
}

// Helper function to check if the current file is the end of the directory
bool Wad::isDirectoryEnd(string& file) 
{
	string endString = "_END";
	size_t found = file.find(endString);
	if (found != string::npos && found == file.size()-4) {return true;}
	else {return false;}
}


/*_________________________________________
	REQUIRED LIBRARY FUNCTION DEFINITIONS
  _________________________________________
*/

// 1) Library Function 
// Dynamically creates a WAD object and loads WAD file data from path into memory
Wad* Wad::loadWad(const string& path) 
{
	// Create new instance of Wad class
	Wad* fileSystem = new Wad(); 

	// Get root and initialize a pointer variable to root so that workingDirectory is initially set to the root directory
	treeNode* root = fileSystem->getRoot();
	Wad::treeNode* workingDirectory = root;

	// Open the file
	ifstream inFile(path, ios::binary);

	// Set up variables to store header data
	string magic;
	int numDescriptors;
	int descriptorOffset;

	// Read the Wad file header
	if (inFile.is_open()) {
		for (int i = 0; i < 4; i += 1) {
			char temp;
			inFile.read((char*)&temp, sizeof(temp));
			magic.push_back(temp);
		}
		inFile.read((char*)&numDescriptors, sizeof(numDescriptors));
		inFile.read((char*)&descriptorOffset, sizeof(descriptorOffset));

		// Store the magic of the Wad file and Wad filename 
		fileSystem->Load(magic, path);

		// Seek to the offset given by the Wad file header
		inFile.seekg(descriptorOffset);
		for (int i = 0; i < numDescriptors; i += 1) {
			// Set up variables to store file data
			int elementOffset;
			int elementLength;
			string elementName;
			inFile.read((char*)&elementOffset, sizeof(elementOffset));
			inFile.read((char*)&elementLength, sizeof(elementLength));
			for (int j = 0; j < 8; j += 1) {
				char temp;
				inFile.read((char*)&temp, sizeof(temp));
				if (isalpha(temp) || isdigit(temp) || ispunct(temp)) {
					elementName.push_back(temp);
				}
			}
			
			// If file has suffix '_START' , add file to current directory.
			if (fileSystem->isDirectoryStart(elementName)) {
				string directoryName = fileSystem->getDirectoryName(elementName);
				fileSystem->addFileNode(workingDirectory, directoryName, elementOffset, elementLength);
				workingDirectory = workingDirectory->searchChild(directoryName);
			}

			// If file has suffix 'END', change working directory to parent directory
			else if (fileSystem->isDirectoryEnd(elementName)) {
				workingDirectory = workingDirectory->searchParent();
			}

			// If file has format: 'E#M#', where # is a number 0-9, then this is a map marker. 
			// Add map marker to current directory and change working directory to map marker
			// The next 10 files are added in the map marker directory, then working directory is reverted to parent directory
			else if (fileSystem->isMapMarker(elementName)) {
				fileSystem->addFileNode(workingDirectory, elementName, elementOffset, elementLength);
				workingDirectory = workingDirectory->searchChild(elementName);
				int limit = i + 10;
				for (i; i < limit; i += 1) {
					int markerOffset;
					int markerLength;
					string markerName;
					inFile.read((char*)&markerOffset, sizeof(markerOffset));
					inFile.read((char*)&markerLength, sizeof(markerLength));
					for (int j = 0; j < 8; j += 1) {
						char temp2;
						inFile.read((char*)&temp2, sizeof(temp2));
						if (isalpha(temp2) || isdigit(temp2) || ispunct(temp2)) {
							markerName.push_back(temp2);
						}
					}
					fileSystem->addFileNode(workingDirectory, markerName, markerOffset, markerLength);
				}
				workingDirectory = workingDirectory->searchParent();
			}
			// If file is not directory format, add a file to the current working directory
			else {
				fileSystem->addFileNode(workingDirectory, elementName, elementOffset, elementLength);
			}
			
		}
			// Close the file
			inFile.close();
		}

		// Load contents from nodes into the file system tree
		fileSystem->loadNodes(fileSystem->root);

		// Return the instance of this file system
		return fileSystem;
}

// 2) Library Function 
// Gets the magic string
string Wad::getMagic() 
{
	return magic;
}

// 3) Library Function
// Checks if given path corresponds to a content file (file with non-zero length)
bool Wad::isContent(const string& path) 
{
	vector<string> rawPath;
	splitPath(path, rawPath);
	string fileName = rawPath.at(rawPath.size() - 1);
	treeNode* file = searchDirectory(fileName, root); 
	if (file != nullptr) {
		if (file->length != 0)
			return true;
		else
			return false;
	}
	else {
		return false;
	}
}

// 4) Library Function
// Checks if given path represents a corresponds to a directory (file with zero length)
bool Wad::isDirectory(const string& path) 
{
	vector<string> rawPath;
	splitPath(path, rawPath);
	string fileName = rawPath.at(rawPath.size() - 1);
	treeNode* file = searchDirectory(fileName, root);
	if (file != nullptr) {
		if (file->length == 0)
			return true;
		else
			return false;
	}
	else {
		return false;
	}
}

// 5) Library Function
// Gets the number of bytes of data represented by data in path
int Wad::getSize(const string& path) 
{
	if (isContent(path)) {
		vector<string> rawPath;
		splitPath(path, rawPath);
		string fileName = rawPath.at(rawPath.size() - 1);
		treeNode* file = searchDirectory(fileName, root);
		return file->length;
	}
	else {
		return -1;
	}
}

// 6) Library Function
// Get number of bytes copied into buffer
int Wad::getContents(const string& path, char* buffer, int length, int offset) {
	if (isContent(path)) {
		vector<string> rawPath;
		splitPath(path, rawPath);
		string fileName = rawPath.at(rawPath.size() - 1);
		treeNode* file = searchDirectory(fileName, root);

		if (offset >= file->length) {
			return -1;
		}

		memcpy(buffer, file->contents + offset, length);
		return length;
	}
	else {
		return -1;
	}
}

// 7) Library Function
// Get number of elements in the directory
int Wad::getDirectory(const string& path, vector<string>* directory) {
	if (isDirectory(path)) {
		vector<string> rawPath;
		splitPath(path, rawPath);
		string fileName = rawPath.at(rawPath.size() - 1);
		treeNode* file = searchDirectory(fileName, root);

		for (int ii = 0; ii < file->children.size(); ii += 1) {
			directory->push_back(file->children.at(ii)->name);
		}
		return file->children.size();
	}
	else {
		return -1;
	}
}
