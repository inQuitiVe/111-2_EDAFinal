// main.cpp for EDA final project
// author: Eric, Cody, Joey
// date: 2022/06
// usage: ./DMP caseOO.v caseOO.lef caseOO.def caseOO.mlist caseOO.txt caseOO.dmp
// argv       0        1          2          3            4          5          6

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <unordered_map>
#include <limits>

#define ITERATION 10

using namespace std;

enum orientation{
    North,
    FlipNorth,
    South,
    FlipSouth
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
    movable = mov;
    pos_x = px;
    pos_y = py;
    orient = ori;
    macro_type = mtype;
}

Macro::~Macro(){}
    
class Pin{ // for def's PI and PO and lef's macro pin
  public:
    string name;
    float pos_x, pos_y;
    // orientation ? (not sure if all the orientation would be North in all test cases)
    Pin(){
        name = "none";
        pos_x = 0.0;
        pos_y = 0.0;
    }
    Pin(string na, float px, float py){
        name = na;
        pos_x = px;
        pos_y = py;
    }  
    bool operator==(const Pin & rhs){
        return this->name == rhs.name;
    }
    ~Pin(){}
};

class Macro_type{
    public:
    string name;
    float width_x, width_y;
    vector<Pin> pin_list;
    // OBS obstruction, probabaly need it but ignore it for now
    Macro_type(){
        name = "none";
        width_x = 0.0;
        width_y = 0.0;
        pin_list.push_back(Pin());
    }
    Macro_type(string na){
        name = na;
        width_x = 0.0;
        width_y = 0.0;
        // pin_list.clear(); // not sure
    }
    void setWidth(float wx, float wy){
        width_x = wx;
        width_y = wy;
    }
    void addPin(Pin pi){
        pin_list.push_back(pi);
    }
    bool operator==(const Macro_type & rhs){
        return this->name == rhs.name;
    }
    ~Macro_type(){};
};

class Rect{ // a rectangle from (init_x, init_y) to (end_x, end_y)
    public:
        int init_x, init_y, end_x, end_y;
        Rect(){
            set(0, 0, 0, 0);
        }
        Rect(int inx, int iny, int edx, int edy){
            set(inx, iny, edx, edy);
        }
        ~Rect(){};
        void set(int inx, int iny, int edx, int edy){
            init_x = inx;
            init_y = iny;
            end_x = edx;
            end_y = edy;
        }
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

    /********** define variables **********/
    int MAX_DISPLACEMENT;
    const int MAX_SPACING = 0;
    const int MACRO_HOLO = 0;
    int def_scalar = 0; // UNITS DISTANCE MICRONS
    int lef_scalar = 0; // DATABASE MICRONS
    int num_macros = 0;
    int num_pins = 0;
    int num_components = 0;
    ifstream verilog(argv[1]);
    ifstream lef(argv[2]);
    ifstream def(argv[3]);
    ifstream mlist(argv[4]);
    ifstream txt(argv[5]); // case00.txt
    string line;
    string word;
    unordered_map<string, Macro> macro_dict; 
    unordered_map<string, Pin> pin_dict;
    unordered_map<string, Macro_type> mctype_dict;
    vector<string> movable_macro;
    Rect diearea(0, 0, 0, 0);

    /********** read input files **********/
    // readInputFile(def, def_state);
    if(def.is_open()){
        string cur_state = "INIT";
        vector<string> words, sec_words, third_words;
        string first_line, second_line, third_line, fourth_line;
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
                    getline(def, second_line); // no need to use the information
                    getline(def, third_line);
                    getline(def, fourth_line); // no need to use the information
                    splitStringToWords(first_line, words);
                    splitStringToWords(third_line, third_words);

                    // if need to use orientation info later
                    // orientaion info at third_words[8]
                    Pin pi(words[1], stof(third_words[5]), stof(third_words[6]));
                    pin_dict[words[1]] = pi;
                    
                }
                break; // finishing pin reading
            }else{ // cur_state == "END"
                break;
            }
        }
        // cout << pin_dict["FE_RN_2"].pos_x << " " << pin_dict["FE_RN_2"].pos_y << "\n";

        def.close();
    }

    if(mlist.is_open()){
        string cur_state = "INIT";
        vector<string> words, sec_words;
        string first_line, second_line;
        while(getline(mlist, line)){
            word = line.substr(0, line.find(" ")); // first word of the sentense
            // switching cur_state
            if(cur_state == "INIT" && word == "UNITS")
                cur_state = "UNITS";
            else if(cur_state =="UNITS" && word == "DIEAREA")
                cur_state = "DIEAREA";
            else if(cur_state =="DIEAREA" && word == "COMPONENTS")
                cur_state = "COMPONENTS";
            
            // do different things according to cur_state
            if(cur_state == "INIT"){
                continue; // do nothing
            }else if(cur_state == "UNITS"){
                splitStringToWords(line, words);
                if(words.size() > 1){
                    def_scalar = stoi(words[3]);
                }
            }else if(cur_state == "DIEAREA"){
                splitStringToWords(line, words);
                if(words.size() > 1){
                    diearea.set(stoi(words[2]), stoi(words[3]), stoi(words[6]), stoi(words[7]));
                }
            }else if(cur_state == "COMPONENTS"){
                splitStringToWords(line, words);
                num_components = stoi(words[1]);
                movable_macro.reserve(num_components); // reserving space for # movable macros
                for(int i=0; i<num_components; ++i){
                    getline(mlist, first_line);
                    getline(mlist, second_line);
                    splitStringToWords(first_line, words);
                    splitStringToWords(second_line, sec_words);
                    macro_dict[words[4]].movable = true; // update bool movable from false to true
                    macro_dict[words[4]].pos_x = stof(sec_words[9]);  // update pos_x
                    macro_dict[words[4]].pos_y = stof(sec_words[10]); // update pos_y
                    // need to update the orientation ??
                    movable_macro.push_back(words[4]);
                }
                break; // end of reading mlist
            }
            
        }
        mlist.close();
    }

    if(lef.is_open()){
        string cur_state = "INIT";
        vector<string> words, sec_words;
        string first_line, second_line;
        string macro_name;
        bool units_done = false;
        while(getline(lef, line)){
            word = line.substr(0, line.find(" ")); // first word of the sentense
            // switching cur_state
            if(cur_state == "INIT" && word == "UNITS")
                cur_state = "UNITS";
            else if(cur_state =="UNITS" && word == "MACRO")
                cur_state = "MACRO";
            
            // do different things according to cur_state
            if(cur_state == "INIT"){
                continue; // do nothing
            }else if(cur_state == "UNITS"){
                if(!units_done){ // only do once
                    getline(lef, line);
                    splitStringToWords(line, words);
                    lef_scalar = stoi(words[4]);
                    // cout << lef_scalar << "\n";
                    units_done = true;
                }
            }else if(cur_state == "MACRO"){
                splitStringToWords(line, words);
                string macro_name = words[1];
                Macro_type mtype(macro_name);
                while(getline(lef, line)){ // read inside MACRO ... END MACRO
                    if(line.substr(4, line.find(" ", 4) - 4) == "SIZE"){
                        splitStringToWords(line, words);
                        mtype.setWidth(stof(words[5]), stof(words[7]));
                    }else if(line.substr(4, line.find(" ", 4) - 4) == "PIN"){
                        string pin_name = line.substr(8, line.find(" ", 8) - 8); // not sure
                        while(getline(lef, line)){ // read inside PIN ... END PIN
                            if(line.substr(8, line.find(" ", 8) - 8) == "RECT"){
                                splitStringToWords(line, words);
                                // RECT: init_x=words[9], init_y=words[10], end_x=words[11], end_y=words[12]
                                float mid_x =  (stof(words[9]) + stof(words[11])) / 2.0;
                                float mid_y = (stof(words[10]) + stof(words[12])) / 2.0;
                                Pin pi(pin_name, mid_x, mid_y);
                                // cout << mtype.name << " " << pin_name << " " << mid_x << " " << mid_y << "\n";
                                mtype.addPin(pi);
                            }else if(line.substr(8, line.find(" ", 8) - 8) == "END"){
                                break;
                            }
                        }
                    }else if(line.substr(0, line.find(" ", 0)) == "END"){
                        mctype_dict[macro_name] = mtype;
                        getline(lef, line); // read an empty line
                        break;
                    }  
                }
            }
            
        }
        // testing
        // cout << mctype_dict["TIEH_X1"].width_x << " " <<  mctype_dict["TIEH_X1"].width_y << "\n";
        // vector<Pin> test_list = mctype_dict["TIEH_X1"].pin_list;
        // for(int i=0; i<test_list.size(); ++i){
        //     cout << test_list[i].name << " " << test_list[i].pos_x << " " << test_list[i].pos_y << "\n";
        // }
        lef.close();
    }

    if(txt.is_open()){
        string line;
        getline(txt, line, ' ');
        getline(txt, line, ' ');
        MAX_DISPLACEMENT = stoi(line);
        MAX_DISPLACEMENT *= def_scalar; // change to the same unit
        // cout << MAX_DISPLACEMENT;
        txt.close();
    }
    

/********** force-based approach **********/

/********** determine final Macro location **********/
    for (int iteration=0; iteration<ITERATION; iteration++)
        for (unordered_map<string, Macro> :: iterator macro_itr = macro_dict.begin() ; macro_itr != macro_dict.end() ; macro_itr++){
            int xaccum[4] = {0,0,0,0};
            int yaccum[4] = {0,0,0,0};


            for (int i = 0; i < len(macro_itr->second.pins); i++ ){
                unordered_map<string, Macro> :: iterator pin_itr;
                unordered_map<string, Macro> :: iterator pin_itr;
                pin_itr = Pin.find(macro_itr->second.pins.net);
                xaccum[0]  +=  pin_itr->second.pos_x - macro_itr->second.pins.N.pos_x ;   
                xaccum[1]  +=  pin_itr->second.pos_x - macro_itr->second.pins.FN.pos_x; 
                xaccum[2]  +=  pin_itr->second.pos_x - macro_itr->second.pins.S.pos_x ; 
                xaccum[3]  +=  pin_itr->second.pos_x - macro_itr->second.pins.FS.pos_x; 
                yaccum[0]  +=  pin_itr->second.pos_y - macro_itr->second.pins.N.pos_y ;   
                yaccum[1]  +=  pin_itr->second.pos_y - macro_itr->second.pins.FN.pos_y; 
                yaccum[2]  +=  pin_itr->second.pos_y - macro_itr->second.pins.S.pos_y ; 
                yaccum[3]  +=  pin_itr->second.pos_y - macro_itr->second.pins.FS.pos_y;                                                                                                                                                                    = macro_itr->second.pins[i].pos_x + macro_itr->second.pos_x;
            }

            int optimal_pos_x = INT_MAX;
            int optimal_pos_y = INT_MAX;
            string optimal_orient = "N";

            xaccum[0] /= len(macro_itr->second.pins);
            yaccum[0] /= len(macro_itr->second.pins);
            if ((abs(optimal_pos_x-init_pos_x) + abs(optimal_pos_y-init_pos_y)) > (abs(xaccum[0]-init_pos_x) + abs(yaccum[0]-init_pos_y))){
                optimal_pos_x = xaccum[0];
                optimal_orient = "N";
            }
            xaccum[1] /= len(macro_itr->second.pins);
            yaccum[1] /= len(macro_itr->second.pins);
            if ((abs(optimal_pos_x-init_pos_x) + abs(optimal_pos_y-init_pos_y)) > (abs(xaccum[1]-init_pos_x) + abs(yaccum[1]-init_pos_y))){
                optimal_pos_x = xaccum[1];
                optimal_orient = "FN";
            }
            xaccum[2] /= len(macro_itr->second.pins);
            yaccum[2] /= len(macro_itr->second.pins);
            if ((abs(optimal_pos_x-init_pos_x) + abs(optimal_pos_y-init_pos_y)) > (abs(xaccum[2]-init_pos_x) + abs(yaccum[2]-init_pos_y))){
                optimal_pos_x = xaccum[2];
                optimal_orient = "S";
            }
            xaccum[3] /= len(macro_itr->second.pins);
            yaccum[3] /= len(macro_itr->second.pins);
            if ((abs(optimal_pos_x-init_pos_x) + abs(optimal_pos_y-init_pos_y)) > (abs(xaccum[3]-init_pos_x) + abs(yaccum[3]-init_pos_y))){
                optimal_pos_x = xaccum[3];
                optimal_orient = "FS";
            }
            

            for (int i=0; i<10; i++){
                if (macro_itr->second.pos_x == optimal_pos_x && macro_itr->second.pos_y == optimal_pos_y) break;
                if (abs(optimal_pos_x-macro_itr->second.pos_x) > abs(optimal_pos_x-macro_itr->second.pos_x)){
                    if(optimal_pos_x > macro_itr->second.pos_x) macro_itr->second.pos_x++;
                    else macro_itr->second.pos_x--;
                }
                else{
                    if(optimal_pos_y > macro_itr->second.pos_y) macro_itr->second.pos_y++;
                    else macro_itr->second.pos_y--;
                }
            }      
        }



    /********** write output file **********/
    ifstream mlist2(argv[4]);
    ofstream dmp(argv[6]);
    if(mlist2.is_open() && dmp.is_open()){
        dmp << fixed << setprecision(0);
        string cur_state = "INIT";
        vector<string> words, sec_words;
        string first_line, second_line;
        while(getline(mlist2, line)){
            word = line.substr(0, line.find(" ")); // first word of the sentense
            // switching cur_state
            if(cur_state == "INIT" && word == "COMPONENTS")
                cur_state = "COMPONENTS";
            
            // do different things according to cur_state
            if(cur_state == "INIT"){
                dmp << line << "\n";
            }else if(cur_state == "COMPONENTS"){
                dmp << line << "\n"; // COMPONENTS 175 ;
                for(int i=0; i<num_components; ++i){ // num_components is calculated previously
                    getline(mlist2, first_line);  // directly wirte same first line to dmp file
                    dmp << first_line << "\n";
                    getline(mlist2, second_line); // change position to new calculated and write to dmp file
                    splitStringToWords(second_line, sec_words);
                    
                    // for(int i=0; i<words.size();++i){
                    //     cout << i << ": " << words[i] << '\n';
                    // }

                    // pos_x is at sec_words[9], pos_y is at sec_words[10]
                    // orientation is at sec_words[12]
                    for(int j=0; j<5; ++j) // 5 spaces
                        dmp << " ";
                    for(int j=6; j<14; ++j){ // rest of the sentence
                        dmp << " ";
                        if(j==9)
                            dmp << macro_dict[movable_macro[i]].pos_x; // pos_x
                        else if(j==10)
                            dmp << macro_dict[movable_macro[i]].pos_y; // pos_y
                        else if(j==12){ // orientation
                            orientation ori = macro_dict[movable_macro[i]].orient;
                            if(ori == North)
                                dmp << "N";
                            else if(ori == FlipNorth)
                                dmp << "FN";
                            else if(ori == South)
                                dmp << "S";
                            else if(ori == FlipSouth)
                                dmp << "FS";
                        }
                        else
                            dmp << sec_words[j];
                    }
                    dmp << "\n";
                }
                break;
            }
        }
        // writing last few lines
        while(getline(mlist2, line)){
            dmp << line << "\n";
        }
        mlist2.close();
        dmp.close();
    }

    return 0;
}
