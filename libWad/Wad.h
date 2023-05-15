#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
using namespace std;

class Wad 
{
public:

	// Node Struct of File System Tree 
	struct treeNode 
	{
		string name;
		int offset;
		int length;
		char* contents;
		vector<treeNode*> children;		   
		treeNode* parent;				   
		
		// Function to search for a child node with given name
		treeNode* searchChild(string name)
		{
			for (int i = 0; i < children.size(); i += 1) {
				if (children.at(i)->name == name) {return children.at(i);}
			}
			return nullptr;
		};	  
		
		// Function to return pointer to parent of currNode
		treeNode* searchParent(){
			return parent;
		};

		// Constructor
		treeNode(string name, int offset, int length, treeNode* parent){
			this->name = name;
			this->offset = offset;
			this->length = length;
			this->parent = parent;
			contents = new char[length];
		}; 
	};

	// Constructor
	Wad();

	// Helper Functions
	void Load(string magic, const string& path);
	void loadNodes(treeNode* currNode);
	void clearNodes(treeNode* currNode);
	void addFileNode(treeNode* directory, string name, int offset, int length);
	void splitPath(const string& path, vector<string>& rawPath);
	treeNode* searchDirectory(string fileName, treeNode* file);
	treeNode* getRoot();
	string getDirectoryName(string& file);
	bool isMapMarker(string& file);
	bool isDirectoryStart(string& file);
	bool isDirectoryEnd(string& file);
	
	// Required Library Functions
	static Wad* loadWad(const string& path);
	string getMagic();
	bool isContent(const string& path);
	bool isDirectory(const string& path);
	int getSize(const string& path);
	int getContents(const string& path, char* buffer, int length, int offset = 0);
	int getDirectory(const string& path, vector<string>* directory);

private:
	string wadFile;		
	string magic;
	treeNode* root;
};
