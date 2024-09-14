#include <app.h>
#include <iostream>
#include <fstream>

using namespace std;

std::string html;

char* getHtml(char* filePath) {
/*
    while (getline(filePath, html)) {
    cout << html;
  }

  char* htmlChar = &html[0]; 
  std::cout << htmlChar;   

  filePath.close();

  return htmlChar;
*/

  ///////////////////////////////////////////////////////////
    ifstream inputFile(filePath); 
  
    // Variable to store each line from the file 
    string line; 
  
    // Read each line from the file and print it 
    while (getline(inputFile, line)) { 
        // Process each line as needed 
        cout << line << endl; 
    } 
  
    // Always close the file when done 
    inputFile.close(); 

    return const_cast<char*>(line.c_str());
}

