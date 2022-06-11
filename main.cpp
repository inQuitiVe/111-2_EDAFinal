// main.cpp for EDA final project
// author: Eric, Cody, Joey
// date: 1989/06/04
// usage: ./DMP caseOO.v caseOO.lef caseOO.def caseOO.mlist caseOO.txt caseOO.dmp
// argv       0        1          2          3            4          5          6

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

enum orientation{
    North,
    FlipNorth,
    South,
    FlipSouth
};

class Macro_type{
    public:
    float width_x, width_y;
    // PIN ;
};

class Macro{
    public:
        string name; // only for implementing operator== function
        float pos_x, pos_y;
        bool movable;
        orientation orient;
        string macro_type;
        Macro(); // default constructor
        Macro(string na, float px, float py, bool mov, orientation ori, string mtype);
        bool operator==(const Macro & rhs){
            return this->name == rhs.name;
        }
        ~Macro();
};
    
Macro::Macro(){
    name = "none";
    pos_x = 0.0;
    pos_y = 0.0;
    movable = false;
    orient = North;
    macro_type = "none";
}

Macro::Macro(string na, float px, float py, bool mov, orientation ori, string mtype){
    name = na;
    pos_x = px;
    pos_y = py;
    movable = mov;
    orient = ori;
    macro_type = mtype;
}

Macro::~Macro(){}
    
class Pin{
  public:
    int pos_x, pos_y;
    // net?   
};

struct Rect{ // a rectangel from (init_x, init_y) to (end_x, end_y)
    int init_x, init_y, end_x, end_y;
};

// code ref: https://java2blog.com/split-string-space-cpp/#Using_find_and_substr_methods
void splitStringToWords(string str, vector<string>& words,  string delimiter = " "){
    int start = 0;
    int end = str.find(delimiter);
    words.clear();
    while (end != -1) {
        words.push_back( str.substr(start, end - start) );
        start = end + delimiter.size();
        end = str.find(delimiter, start);
    }
    words.push_back( str.substr(start, end - start) );
}

int main(int argc, char* argv[]){
    if(argc != 7){
        cout << "Usage: ./DMP caseOO.v caseOO.lef caseOO.def caseOO.mlist caseOO.txt caseOO.dmp\n";
        return 0;
    }

    /********** read input files **********/
    int MAX_DISPLACEMENT = 0;
    const int MAX_SPACING = 0;
    const int MACRO_HOLO = 0;
    int num_macros = 0;
    int num_pins = 0;
    int num_components = 0;
    ifstream lef(argv[2]);
    ifstream def(argv[3]);
    ifstream mlist(argv[4]);
    ifstream txt(argv[5]); 
    string line;
    string word;
    unordered_map<string, Macro> macro_dict; 
    vector<string> movable_macro;

    if(txt.is_open()){
        getline(txt, line, ' ');
        getline(txt, line, ' ');
        MAX_DISPLACEMENT = stoi(line);
        txt.close();
    }
    
    if(def.is_open()){
        string cur_state = "INIT";
        vector<string> words, sec_words;
        string first_line, second_line;
        while(getline(def, line)){
            word = line.substr(0, line.find(" ")); // first word of the sentense
            // switching cur_state
            if(cur_state == "INIT" && word == "COMPONENTS")
                cur_state = "COMPONENTS";
            else if(cur_state == "COMPONENTS" && word == "PINS")
                cur_state = "PINS";
            else if(cur_state == "PINS" && word == "END")
                cur_state = "END";
            
            // do different things according to cur_state
            if(cur_state == "INIT"){
                continue; // do nothing
            }else if(cur_state == "COMPONENTS"){
                splitStringToWords(line, words);
                num_macros = stoi(words[1]);
                for(int i=0; i<num_macros; ++i){
                    getline(def, first_line);
                    getline(def, second_line);
                    splitStringToWords(first_line, words);
                    splitStringToWords(second_line, sec_words);
                    Macro mac(words[1], stof(sec_words[3]), stof(sec_words[4]), false, North, words[2]);
                    macro_dict[words[1]] = mac;
                }
                getline(def, line); // END COMPONENTS
                getline(def, line); // empty lines
            }else if(cur_state == "PINS"){
                splitStringToWords(line, words);
                num_pins = stoi(words[1]);
                for(int i=0; i<num_pins; ++i){
                    getline(def, first_line);
                    getline(def, second_line);
                    splitStringToWords(first_line, words);
                    splitStringToWords(second_line, sec_words);

                    // TODO: unfinished part
                    break;
                    
                }
                // TODO: unfinished part
                break; 
            }else{ // cur_state == "END"
                break;
            }
        }
        def.close();
    }

    if(mlist.is_open()){
        string cur_state = "INIT";
        vector<string> words;
        string first_line, second_line;
        while(getline(mlist, line)){
            word = line.substr(0, line.find(" ")); // first word of the sentense
            // switching cur_state
            if(cur_state == "INIT" && word == "COMPONENTS")
                cur_state = "COMPONENTS";
            
            // do different things according to cur_state
            if(cur_state == "INIT"){
                continue; // do nothing
            }else if(cur_state == "COMPONENTS"){
                splitStringToWords(line, words);
                num_components = stoi(words[1]);
                movable_macro.reserve(num_components); // reserving space for # movable macros
                for(int i=0; i<num_components; ++i){
                    getline(mlist, first_line);
                    getline(mlist, second_line); // no need to use second_line's information
                    splitStringToWords(first_line, words);
                    macro_dict[words[4]].movable = true; // update bool movable from false to true
                    movable_macro.push_back(words[4]);
                }
                break; // end of reading mlist
            }
        }
        mlist.close();
    }
    

    /********** force-based approach **********/
    
    /********** determine final Macro location **********/

    /********** write output file **********/

    return 0;
}
