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
#include <ctype.h> // for isalpha()

#define ITERATION 10

using namespace std;

enum orientation{
    North,
    FlipNorth,
    South,
    FlipSouth
};

class Component{
    public:
        string name; // only for implementing operator == function
        float pos_x, pos_y;
        bool movable;
        orientation orient;
        string component_type;
        Component(){// default constructor
            name = "none";
            pos_x = 0.0;
            pos_y = 0.0;
            movable = false;
            orient = North;
            component_type = "none";
        }
        Component(string na, float px, float py, bool mov, orientation ori, string mtype){
            name = na;
            pos_x = px;
            pos_y = py;
            movable = mov;
            orient = ori;
            component_type = mtype;
        }
        bool operator==(const Component & rhs){
            return this->name == rhs.name;
        }
        ~Component(){}
};
    
class Pin{ // for def's PI and PO and lef's macro pin
  public:
    string name;
    vector<pair<float, float>> pos_list; //pos_list[i].first = pos_x, pos_list[i].second = pos_y
    // orientation ? (not sure if all the orientation would be North in all test cases)
    Pin(){
        name = "none";
        pair<float, float> p1(0.0, 0.0);
        pos_list.push_back(p1);
    }
    Pin(string na){
        name = na;
    }  
    void addPinLoc(float px, float py){
        pair<float, float> p1(px, py);
        pos_list.push_back(p1);
    }
    bool operator==(const Pin & rhs){
        return this->name == rhs.name;
    }
    ~Pin(){}
};

class Component_type{
    public:
    string name;
    float width_x, width_y;
    // vector<Pin> pin_list;
    unordered_map<string, Pin> pin_list;
    // OBS obstruction, probabaly need it but ignore it for now
    Component_type(){
        name = "none";
        width_x = 0.0;
        width_y = 0.0;
    }
    Component_type(string na){
        name = na;
        width_x = 0.0;
        width_y = 0.0;
    }
    void setWidth(float wx, float wy){
        width_x = wx;
        width_y = wy;
    }
    void addPin(Pin pi){
        pin_list[pi.name] = pi;
    }
    bool operator==(const Component_type & rhs){
        return this->name == rhs.name;
    }
    ~Component_type(){};
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
    unordered_map<string, Component> component_dict; 
    unordered_map<string, Pin> pin_dict;
    unordered_map<string, Component_type> mctype_dict;
    vector<string> macro;
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
                num_components = stoi(words[1]);
                for(int i=0; i<num_components; ++i){
                    getline(def, first_line);
                    getline(def, second_line);
                    splitStringToWords(first_line, words);
                    splitStringToWords(second_line, sec_words);
                    Component mac(words[1], stof(sec_words[3]), stof(sec_words[4]), false, North, words[2]);
                    component_dict[words[1]] = mac;
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
                    Pin pi(words[1]);
                    pi.addPinLoc(stof(third_words[5]), stof(third_words[6]));
                    pin_dict[words[1]] = pi;
                    
                }
                break; // finishing pin reading
            }else{ // cur_state == "END"
                break;
            }
        }
        // testing
        // cout << "def testing: \n" ;
        // cout << "num_components: " << num_components << "\n";
        // cout << component_dict["FE_OFC114571_n176682"].pos_x << " " << component_dict["FE_OFC114571_n176682"].pos_y << "\n";
        // cout << component_dict["o809057"].pos_x << " " << component_dict["o809057"].pos_y << "\n";
        // cout << "num_pins: " << num_pins << "\n";
        // vector<pair<float, float>> pos = pin_dict["FE_RN_2"].pos_list;
        // cout << pos[0].first << " " << pos[0].second << "\n";

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
                num_macros = stoi(words[1]);
                macro.reserve(num_macros); // reserving space for # movable macros
                for(int i=0; i<num_macros; ++i){
                    getline(mlist, first_line);
                    getline(mlist, second_line);
                    splitStringToWords(first_line, words);
                    splitStringToWords(second_line, sec_words);
                    component_dict[words[4]].movable = true; // update bool movable from false to true
                    component_dict[words[4]].pos_x = stof(sec_words[9]);  // update pos_x
                    component_dict[words[4]].pos_y = stof(sec_words[10]); // update pos_y
                    // need to update the orientation ??
                    macro.push_back(words[4]);
                }
                break; // end of reading mlist
            }
            
        }
        // testing
        // cout << "mlist testing: \n" ;
        // for(int i =0; i<macro.size(); ++i){
        //     cout << macro[i] << "\n";
        // }
        mlist.close();
    }

    if(lef.is_open()){
        string cur_state = "INIT";
        vector<string> words, sec_words;
        string first_line, second_line;
        string macro_name;
        bool units_done = false;
        bool is_standardCell = true;
        while(getline(lef, line)){
            word = line.substr(0, line.find(" ")); // first word of the sentense
            // switching cur_state
            if(cur_state == "INIT" && word == "UNITS")
                cur_state = "UNITS";
            else if(cur_state =="UNITS" && word == "MACRO")
                cur_state = "STDCELLS";
            
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
            }else if(cur_state == "STDCELLS"){
                splitStringToWords(line, words);
                string macro_name = words[1];
                Component_type mtype(macro_name);

                // get an extra line to ensure is reading standard cell or macro
                getline(lef, line);
                is_standardCell = isalpha(line[1])? 0:1;
                
                if(is_standardCell){
                    while(getline(lef, line)){ // read inside MACRO ... END MACRO
                        
                        if(line.substr(4, line.find(" ", 4) - 4) == "SIZE"){
                            splitStringToWords(line, words);
                            mtype.setWidth(stof(words[5]), stof(words[7]));
                        }else if(line.substr(4, line.find(" ", 4) - 4) == "PIN"){
                            string pin_name = line.substr(8, line.find(" ", 8) - 8); // not sure
                            // cout << pin_name << "\n";
                            Pin pi(pin_name);
                            while(getline(lef, line)){ // read inside PIN ... END PIN
                                if(line.substr(8, line.find(" ", 8) - 8) == "RECT"){
                                    splitStringToWords(line, words);
                                    // RECT: init_x=words[9], init_y=words[10], end_x=words[11], end_y=words[12]
                                    float mid_x =  (stof(words[9]) + stof(words[11])) / 2.0;
                                    float mid_y = (stof(words[10]) + stof(words[12])) / 2.0;
                                    pi.addPinLoc(mid_x, mid_y);
                                }else if(line.substr(8, line.find(" ", 8) - 8) == "END"){
                                    mtype.addPin(pi);
                                    break;
                                }
                            }
                        }else if(line.substr(0, line.find(" ", 0)) == "END"){
                            mctype_dict[macro_name] = mtype;
                            getline(lef, line); // read an empty line
                            break;
                        }
                    }
                }else{// is macro
                    while(getline(lef, line)){ // read inside MACRO ... END MACRO
                        
                        if(line.substr(3, line.find(" ", 3) - 3) == "SIZE"){
                            splitStringToWords(line, words);
                            mtype.setWidth(stof(words[4]), stof(words[6]));
                        }else if(line.substr(1, line.find(" ", 1) - 1) == "PIN"){
                            string pin_name = line.substr(5, line.find(" ", 5) - 5); // not sure
                            // cout << pin_name << "\n";
                            Pin pi(pin_name);
                            while(getline(lef, line)){ // read inside PIN ... END PIN
                                if(line.substr(3, line.find(" ", 3) - 3) == "RECT"){
                                    splitStringToWords(line, words);
                                    // RECT: init_x=words[4], init_y=words[5], end_x=words[6], end_y=words[7]
                                    float mid_x =  (stof(words[4]) + stof(words[5])) / 2.0;
                                    float mid_y = (stof(words[6]) + stof(words[7])) / 2.0;
                                    pi.addPinLoc(mid_x, mid_y);
                                    
                                }else if(line.substr(1, line.find(" ", 1) - 1) == "END"){
                                    mtype.addPin(pi);
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
            
        }
        // testing
        // cout << "lef testing: \n" ;
        // cout << mctype_dict["block_1219x6795_1243f"].width_x << " " <<  mctype_dict["block_1219x6795_1243f"].width_y << "\n";
        // unordered_map<string, Pin> test_list = mctype_dict["block_1219x6795_1243f"].pin_list;
        // vector<pair<float, float>> pos = test_list["o0"].pos_list;
        // for(int i=0; i<pos.size(); ++i){
        //     cout << pos[i].first << " " << pos[i].second << "\n";
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
    for (int iteration=0; iteration<ITERATION; iteration++){
        for (unordered_map<string, Component> :: iterator macro_itr = component_dict.begin() ; macro_itr != component_dict.end() ; macro_itr++){
            int xaccum[4] = {0,0,0,0}; //N, FN, S, FS
            int yaccum[4] = {0,0,0,0};


            for (int i = 0; i < len(macro_itr->second.pins); i++ ){
                unordered_map<string, Component> :: iterator pin_itr;
                unordered_map<string, Component> :: iterator pin_itr;
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
                for(int i=0; i<num_macros; ++i){ // num_macros is calculated previously
                    getline(mlist2, first_line);  // directly wirte same first line to dmp file
                    dmp << first_line << "\n";
                    getline(mlist2, second_line); // change position to new calculated and write to dmp file
                    // splitStringToWords(first_line, words);
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
                            dmp << component_dict[macro[i]].pos_x; // pos_x
                        else if(j==10)
                            dmp << component_dict[macro[i]].pos_y; // pos_y
                        else if(j==12){ // orientation
                            orientation ori = component_dict[macro[i]].orient;
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
        // wirting last few lines
        while(getline(mlist2, line)){
            dmp << line << "\n";
        }
        mlist2.close();
        dmp.close();
    }

    return 0;
}
