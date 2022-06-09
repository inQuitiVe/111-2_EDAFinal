// main.cpp for EDA final project
// author: Eric, Cody, Joey
// date: 1989/06/04
// usage: ./DMP caseOO.v caseOO.lef caseOO.def caseOO.mlist caseOO.txt caseOO.dmp
// argv       0        1          2          3            4          5          6
// input file: caseOO.v caseOO.lef caseOO.def caseOO.mlist caseOO.txt
// output file: caseOO.dmp
#include <iostream>
#include <fstream>
#include <string>
// #include <string.h>
#include <limbo/parsers/lef/adapt/LefDriver.h>
using namespace std;

int main(int argc, char* argv[]){
    if(argc != 7){
        cout << "Usage: ./DMP caseOO.v caseOO.lef caseOO.def caseOO.mlist caseOO.txt caseOO.dmp\n";
        return 0;
    }
    /********** read input files **********/
    int MAX_DISPLACEMENT;
    const int MAX_SPACING = 0;
    const int MACRO_HOLO = 0;
    ifstream constraintfile(argv[5]); // case00.txt
    string line;
    char * token;
    if(constraintfile.is_open()){
        getline(constraintfile, line, ' ');
        getline(constraintfile, line, ' ');
        MAX_DISPLACEMENT = stoi(line);
        // cout << MAX_DISPLACEMENT;
    }
    constraintfile.close();

    /********** force-based approach **********/
    
    /********** determine final Macro location **********/

    /********** write output file **********/

    return 0;
}
class Macro{
    private:
        /* data */
    public:
        int width, length, pos_x, pos_y, id;
        int* connection_list;
        main(/* args */);
        ~main();
};
    
    Macro::Macro(/* args */)
    {
    }
    
    Macro::~Macro()
    {
    }

class Pin{
  public:
    int pos_x, pos_y;
    // net?   
};