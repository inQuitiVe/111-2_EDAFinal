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
#include <climits>
#include <ctype.h> // for isalpha()
#include <cmath>
#include <time.h>

#define ITERATION 1
#define CAL_TIME  1 // set to 0 if don't want to measure execution time 

using namespace std;
typedef pair<float, float> float_pair;

enum orientation{
    North,
    FlipNorth,
    South,
    FlipSouth
};
                    
orientation ori_list[4] = {North, FlipNorth, South, FlipSouth};
float_pair origin(0.0, 0.0);

void transorient(float_pair compnt_pos, float_pair compnt_size, float_pair pin_pos, orientation orient, float* pin_abs_pos){
    if (orient == North){
        pin_abs_pos[0] = compnt_pos.first + pin_pos.first;
        pin_abs_pos[1] = compnt_pos.second + pin_pos.second;
    }
    if (orient == FlipNorth){
        pin_abs_pos[0] = compnt_pos.first + compnt_size.first - pin_pos.first;
        pin_abs_pos[1] = compnt_pos.second + pin_pos.second;
    }
    if (orient == South){
        pin_abs_pos[0] = compnt_pos.first + compnt_size.first - pin_pos.first;
        pin_abs_pos[1] = compnt_pos.second + compnt_size.second - pin_pos.second;
    }
    if (orient == FlipSouth){
        pin_abs_pos[0] = compnt_pos.first + pin_pos.first;
        pin_abs_pos[1] = compnt_pos.second + compnt_size.second - pin_pos.second;
    }
}

bool check_overlap(float a_pos_x, float a_pos_y, float a_width_x, float a_width_y, float b_pos_x, float b_pos_y, float b_width_x, float b_width_y){
    float a_down_bound = a_pos_y;
    float a_top_bound = a_pos_y + a_width_y;
    float a_left_bound = a_pos_x;
    float a_right_bound = a_pos_x + a_width_x;
    float b_down_bound = b_pos_y;
    float b_top_bound = b_pos_y + b_width_y;
    float b_left_bound = b_pos_x;
    float b_right_bound = b_pos_x + b_width_x;

    if ((b_top_bound >= a_down_bound && b_top_bound <= a_top_bound) || (b_down_bound >= a_down_bound && b_down_bound <= a_top_bound))
        if ((b_left_bound >= a_left_bound && b_left_bound <= a_right_bound) || (b_right_bound >= a_left_bound && b_right_bound <= a_right_bound))
            return true;
    if ((a_top_bound >= b_down_bound && a_top_bound <= b_top_bound) || (a_down_bound >= b_down_bound && a_down_bound <= b_top_bound))
        if ((a_left_bound >= b_left_bound && a_left_bound <= b_right_bound) || (a_right_bound >= b_left_bound && a_right_bound <= b_right_bound))
            return true;
    return false;
}


class Component{
    public:
        string name; // only for implementing operator == function
        float pos_x, pos_y;
        bool movable;
        orientation orient;
        string component_type;
        vector<pair<string, string>> pin_connection; //pin_connection[i].first=pin_name, .second=wire_name
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
        void addConnection(string pin_name, string connect_name){
            pair<string, string> p1(pin_name, connect_name);
            pin_connection.push_back(p1);
        }
        bool operator==(const Component & rhs){
            return this->name == rhs.name;
        }
        ~Component(){}
};
    
class Pin{ // for def's PI and PO and lef's macro pin
    public:
        string name;
        vector<float_pair> pos_list; //pos_list[i].first = pos_x, pos_list[i].second = pos_y
        // orientation ? (not sure if all the orientation would be North in all test cases)
        Pin(){
            name = "none";
            float_pair p1(0.0, 0.0);
            pos_list.push_back(p1);
        }
        Pin(string na){
            name = na;
        }  
        void addPinLoc(float px, float py){
            float_pair p1(px, py);
            pos_list.push_back(p1);
        }
        bool operator==(const Pin & rhs){
            return this->name == rhs.name;
        }
        ~Pin(){};
};

class Component_type{
    public:
        string name;
        float width_x, width_y;
        // vector<Pin> pin_list;
        unordered_map<string, Pin> pin_list;
        // OBS obstruction, probably need it but ignore it for now
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
    ifstream verilog2(argv[1]);
    ifstream lef(argv[2]);
    ifstream def(argv[3]);
    ifstream mlist(argv[4]);
    ifstream mlist2(argv[4]);
    ifstream txt(argv[5]);
    ofstream dmp(argv[6]);
    string line;
    string word;
    unordered_map<string, Component> component_dict; 
    unordered_map<string, Pin> pin_dict;
    unordered_map<string, Component_type> mctype_dict;
    unordered_map<string, vector<pair<string, string>>> connection_dict; // .first is component_name .second is pin_name
    unordered_map<string, bool> stored_wire;
    unordered_map<string, bool> is_primary_io;
    vector<string> movable_macro;
    vector<string> all_macro;
    Rect diearea(0, 0, 0, 0);

    /********** variables related to time **********/
    time_t start, after_input, after_algo, end;

    /********** read input files **********/
    // readInputFile(def, def_state);
    time(&start);
    cout << "Read Input Files\n";
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
                    is_primary_io[words[1]] = true;
                }
                break; // finishing pin reading
            }else{ // cur_state == "END"
                break;
            }
        }
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
                movable_macro.reserve(num_macros); // reserving space for # movable macros
                all_macro.reserve(num_macros);
                for(int i=0; i<num_macros; ++i){
                    getline(mlist, first_line);
                    getline(mlist, second_line);
                    splitStringToWords(first_line, words);
                    splitStringToWords(second_line, sec_words);

                    // FIXED or PLACED: sec_words[7]
                    if(sec_words[7] == "PLACED"){
                        component_dict[words[4]].movable = true; // update bool movable from false to true
                        movable_macro.push_back(words[4]);
                    }
                    
                    component_dict[words[4]].pos_x = stof(sec_words[9]);  // update pos_x
                    component_dict[words[4]].pos_y = stof(sec_words[10]); // update pos_y
                    // need to update the orientation ??
                    all_macro.push_back(words[4]);
                    
                }
                break; // end of reading mlist
            }
        }
    }

    if(verilog.is_open()){
        string cur_state = "INIT";
        vector<string> words;
        while(getline(verilog, line)){
            // switching cur_state
            if(cur_state == "INIT" && line == "// Start cells"){
                getline(verilog, line); // get the first line of cells
                cur_state = "CELLS";
            }

            // do different things according to cur_state
            if(cur_state == "INIT"){
                continue; // do nothing
            }else if(cur_state == "CELLS"){
                splitStringToWords(line, words);
                int pin_num = words.size() - 4;
                string comp_name = words[1]; // exclude mctype_name, macro_name, (, and );
                for(int i=0; i<pin_num; ++i){
                    int left_brace_pos = words[i+3].find("(");
                    int right_brace_pos = words[i+3].find(")");
                    string pin_name = words[i+3].substr(1, left_brace_pos-1);
                    string wire_name = words[i+3].substr(left_brace_pos+1, right_brace_pos-left_brace_pos-1);

                    // add connection only if the component is macro & movable
                    if(component_dict[comp_name].movable){ 
                        component_dict[comp_name].addConnection(pin_name, wire_name);
                        stored_wire[wire_name] = 1; // actually the value is not important
                    }
                }
                if(line == "") break; // end of reading verilog file
            }
        }
        verilog.close();
    }

    if(verilog2.is_open()){
        string cur_state = "INIT";
        vector<string> words;
        while(getline(verilog2, line)){
            // word = line.substr(0, line.find(" ")); // first word of the sentense
            // switching cur_state
            if(cur_state == "INIT" && line == "// Start cells"){
                getline(verilog2, line); // get the first line of cells
                cur_state = "CELLS";
            }
                
            // do different things according to cur_state
            if(cur_state == "INIT"){
                continue; // do nothing
            }else if(cur_state == "CELLS"){            
                splitStringToWords(line, words);
                int pin_num = words.size() - 4;
                string comp_name = words[1];
                for(int i=0; i<pin_num; ++i){
                    int left_brace_pos = words[i+3].find("(");
                    int right_brace_pos = words[i+3].find(")");
                    string pin_name = words[i+3].substr(1, left_brace_pos-1);
                    string wire_name = words[i+3].substr(left_brace_pos+1, right_brace_pos-left_brace_pos-1);
                    if(stored_wire.find(wire_name) != stored_wire.end()){
                        pair<string, string> p1(comp_name, pin_name);
                        vector<pair<string, string>> v1;
                        if(connection_dict.find(wire_name) == connection_dict.end()){ // the key is new to connection_dict
                            v1.push_back(p1);
                            connection_dict[wire_name] = v1;
                        }else{ // the key already exists
                            v1 = connection_dict[wire_name];
                            v1.push_back(p1);
                            connection_dict[wire_name] = v1;
                        }
                    }     
                }
                if(line == "") break; // end of reading verilog file
            }
        }
        verilog.close();
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
                            mtype.setWidth(stof(words[5])*lef_scalar, stof(words[7])*lef_scalar);
                        }else if(line.substr(4, line.find(" ", 4) - 4) == "PIN"){
                            string pin_name = line.substr(8, line.find(" ", 8) - 8); // not sure
                            Pin pi(pin_name);
                            while(getline(lef, line)){ // read inside PIN ... END PIN
                                if(line.substr(8, line.find(" ", 8) - 8) == "RECT"){
                                    splitStringToWords(line, words);
                                    float mid_x =  (stof(words[9]) + stof(words[11])) / 2.0;
                                    float mid_y = (stof(words[10]) + stof(words[12])) / 2.0;
                                    pi.addPinLoc(mid_x*lef_scalar, mid_y*lef_scalar);
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
                            mtype.setWidth(stof(words[4])*lef_scalar, stof(words[6])*lef_scalar);
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
                                    pi.addPinLoc(mid_x*lef_scalar, mid_y*lef_scalar);
                                    
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
        lef.close();
    }

    if(txt.is_open()){
        string line;
        getline(txt, line, ' ');
        getline(txt, line, ' ');
        MAX_DISPLACEMENT = stoi(line);
        MAX_DISPLACEMENT *= def_scalar; // change to the same unit
        txt.close();
    }
    time(&after_input);
    

    /********** force-directed approach **********/
    /*
    for (int i=0; i<num_macros; i++){ // move all macros
        int wire_total_length_x = 0, wire_total_length_y = 0;
        for (int j=0; j<component_dict[movable_macro[i]].pin_connection.size(); j++){ // consider all the connected components of the macro
            wire = component_dict[movable_macro[i]].pin_connection[j].second;
            wire_total_length_x += wire.x;
            wire_total_length_y += wire.y;
            
        }
    }
    */
    

    /********** determine final Macro location **********/
    cout << "Determine Macro Location...\n";
    for (int iteration=0; iteration<ITERATION; iteration++){
        for (int macro_itr=0; macro_itr!=movable_macro.size(); macro_itr++){
            cout << "\tMacro #" << macro_itr;
            int accum_num = 0;
            float xaccum[4] = {0,0,0,0}; //N, FN, S, FS
            float yaccum[4] = {0,0,0,0};
            string operating_macro_name = movable_macro[macro_itr];
            Component* operating_macro = &component_dict[operating_macro_name];
            string operating_macro_type = operating_macro->component_type;
            vector<pair<string, string>> operating_connection = operating_macro->pin_connection;
            for (int pin_idx=0; pin_idx<operating_connection.size(); pin_idx++){
                string connect_pin  = operating_connection[pin_idx].first;
                string connect_wire = operating_connection[pin_idx].second;
                vector<pair<string, string>> wire_to_all_connections = connection_dict[connect_wire];
                Component_type operating_macro_type_name = mctype_dict[operating_macro_type];

                float_pair macro_size(operating_macro_type_name.width_x, operating_macro_type_name.width_y);
                // calculate macro_pin_relative_pos
                float macro_pin_relative_pos[4][2];
                for (int i=0; i<4; i++){
                    transorient(origin, macro_size, operating_macro_type_name.pin_list[connect_pin].pos_list[0], ori_list[i], macro_pin_relative_pos[i]);
                }

                if(is_primary_io.find(connect_wire) != is_primary_io.end()){ // pin is PI/PO
                    for (int i=0; i<4; i++){
                        cout << "IO add " << (pin_dict[connect_wire].pos_list[0].first  - macro_pin_relative_pos[i][0]) << "\t";
                        xaccum[i]  +=  (pin_dict[connect_wire].pos_list[0].first  - macro_pin_relative_pos[i][0]);
                        yaccum[i]  +=  (pin_dict[connect_wire].pos_list[0].second - macro_pin_relative_pos[i][1]);
                        accum_num ++ ;   
                    }
                }
                
                for (int desti_idx = 0; desti_idx != wire_to_all_connections.size(); desti_idx++){
                    string desti_macro_name = wire_to_all_connections[desti_idx].first;
                    if (desti_macro_name == (operating_macro->name)) 
                        continue;
                      
                    Component desti_macro = component_dict[desti_macro_name];
                    string desti_macro_type = desti_macro.component_type;
                    string desti_pin_name = wire_to_all_connections[desti_idx].second;

                    float_pair compnt_pos (desti_macro.pos_x, desti_macro.pos_y);
                    float_pair compnt_size(mctype_dict[desti_macro_type].width_x, mctype_dict[desti_macro_type].width_y);
                    // 0, 0 without any reason ??????????
                    
                    float pin_abs_pos[4][2];
                    for (int i=0; i<4; i++)
                        transorient(compnt_pos, compnt_size, mctype_dict[desti_macro_type].pin_list[desti_pin_name].pos_list[0], desti_macro.orient, pin_abs_pos[i]);

                    for (int i=0; i<4; i++){
                        // 相對距離 = 接到的pin的絕對位置 - 相對macro的pin位置
                        cout << pin_abs_pos[i][0] << " " << macro_pin_relative_pos[i][0] << "\t";
                        cout << pin_abs_pos[i][1] << " " << macro_pin_relative_pos[i][1] << "\t";
                        xaccum[i]  +=  (pin_abs_pos[i][0] - macro_pin_relative_pos[i][0]);
                        yaccum[i]  +=  (pin_abs_pos[i][1] - macro_pin_relative_pos[i][1]);  
                        accum_num ++ ; 
                    }
                }
            }
                

            // iteration的目的是destination的pinpos可能會因為macro移動而改變，但平常計算仍需要用initial pos來計算
            xaccum[0] /= accum_num;
            yaccum[0] /= accum_num;
            int optimal_pos_x = xaccum[0]; 
            int optimal_pos_y = yaccum[0];
            orientation optimal_orient = North;
            int current_pos_x = int(operating_macro->pos_x); // get pos_x, pos_y from def file, if unit->smallest, then int is ok
            int current_pos_y = int(operating_macro->pos_y);

            cout << '\t' << accum_num << "  ";
            // find optimal pos_x, pos_y, and orient
            for(int i=1; i<4; ++i){ //i=0:North, i=1:FlipNorth, i=2:South, i=3:FlipSouth, 
                xaccum[i] /= accum_num;
                yaccum[i] /= accum_num;
                if ((abs(optimal_pos_x - current_pos_x) +  abs(optimal_pos_y - current_pos_y)) > 
                    (abs(   xaccum[i]  - current_pos_x) +  abs(   yaccum[i]  - current_pos_y))){
                    optimal_pos_x = xaccum[i]; // int<-float problem(?)
                    optimal_pos_y = yaccum[i];
                    optimal_orient = ori_list[i];
                }
            }
            
            cout << optimal_pos_x << ' ' << optimal_pos_y << '\t';
            float x_diff = optimal_pos_x - current_pos_x;
            float y_diff = optimal_pos_y - current_pos_y;
            float optimal_total_displacement = abs(x_diff) + abs(y_diff);
            cout << "\tMove " << optimal_total_displacement << " without constraint. ";
            // float buffer_pos_x, buffer_pos_y; // buffer_pos_x, buffer_pos_y: int?
            // if (optimal_total_displacement > MAX_DISPLACEMENT){ // consider the constraint MAX_DISPLACEMENT
            //     cout << "\tConsider constraint: ";
            //     // 1. ratio method
            //     /*
            //     float ratio = MAX_DISPLACEMENT/optimal_total_displacement;
            //     buffer_pos_x = int(floor(x_diff * ratio)) + current_pos_x;
            //     buffer_pos_y = int(floor(y_diff * ratio)) + current_pos_y;
            //     */
                
            //     // 2. move the macro to the position with the same x&y remaining distance first, then move macro along x&y axis with equal step
            //     float xy_diff = abs(x_diff) - abs(y_diff);
            //     float remain_displacement = (MAX_DISPLACEMENT - abs(xy_diff))/2;

            //     if (xy_diff >= MAX_DISPLACEMENT){
            //         if (x_diff > 0)
            //             buffer_pos_x = current_pos_x + MAX_DISPLACEMENT;
            //         else
            //             buffer_pos_x = current_pos_x - MAX_DISPLACEMENT;
            //         buffer_pos_y = current_pos_y;
            //     }
            //     else if (xy_diff <= -1*MAX_DISPLACEMENT){
            //         if (y_diff > 0)
            //             buffer_pos_y = current_pos_y + MAX_DISPLACEMENT;
            //         else
            //             buffer_pos_y = current_pos_y - MAX_DISPLACEMENT;
            //         buffer_pos_x = current_pos_x;
            //     }
            //     else if (xy_diff > 0){
            //         if (x_diff > 0)
            //             buffer_pos_x = current_pos_x + int(floor(xy_diff + remain_displacement));
            //         else
            //             buffer_pos_x = current_pos_x - int(floor(xy_diff + remain_displacement));
            //         if (y_diff > 0)
            //             buffer_pos_y = current_pos_y + int(floor(remain_displacement));
            //         else
            //             buffer_pos_y = current_pos_y - int(floor(remain_displacement));
            //     }
            //     else{
            //         if (y_diff > 0)
            //             buffer_pos_y = current_pos_y + int(floor(abs(xy_diff) + remain_displacement));
            //         else
            //             buffer_pos_y = current_pos_y - int(floor(abs(xy_diff) + remain_displacement));
            //         if (x_diff > 0)
            //             buffer_pos_x = current_pos_x + int(floor(remain_displacement));
            //         else
            //             buffer_pos_x = current_pos_x - int(floor(remain_displacement));
            //     }
            // }
            // else{ // directly move the macro to the optimal position
            //     buffer_pos_x = int(floor(x_diff)) + current_pos_x;
            //     buffer_pos_y = int(floor(y_diff)) + current_pos_y;
            // }

            int move_distance_x =0, move_distance_y = 0;
            if (optimal_total_displacement > MAX_DISPLACEMENT){
                float xy_diff = abs(x_diff) - abs(y_diff);
                float remain_displacement = (MAX_DISPLACEMENT - abs(xy_diff))/2;

                if (xy_diff >= MAX_DISPLACEMENT){
                    if (x_diff > 0)
                        move_distance_x =  + MAX_DISPLACEMENT;
                    else
                        move_distance_x =  - MAX_DISPLACEMENT;
                    move_distance_y = 0;
                }
                else if (xy_diff <= -1*MAX_DISPLACEMENT){
                    if (y_diff > 0)
                        move_distance_y =  + MAX_DISPLACEMENT;
                    else
                        move_distance_y =  - MAX_DISPLACEMENT;
                    move_distance_x = 0;
                }
                else if (xy_diff > 0){
                    if (x_diff > 0)
                        move_distance_x =  + int(floor(xy_diff + remain_displacement));
                    else
                        move_distance_x =  - int(floor(xy_diff + remain_displacement));
                    if (y_diff > 0)
                        move_distance_y =  + int(floor(remain_displacement));
                    else
                        move_distance_y =  - int(floor(remain_displacement));
                }
                else{
                    if (y_diff > 0)
                        move_distance_y =  + int(floor(abs(xy_diff) + remain_displacement));
                    else
                        move_distance_y =  - int(floor(abs(xy_diff) + remain_displacement));
                    if (x_diff > 0)
                        move_distance_x =  + int(floor(remain_displacement));
                    else
                        move_distance_x =  - int(floor(remain_displacement));
                }
            }
            else{ // directly move the macro to the optimal position
                move_distance_x = int(floor(x_diff)) ;
                move_distance_y = int(floor(y_diff)) ;
            }
            
            bool overlap = true;
            string mode = "initial";
            while (move_distance_x + move_distance_y > 0){
                bool need_retuning = false;
                for(int overlap_itr = 0; overlap_itr != movable_macro.size(); overlap_itr++){
                    if (overlap_itr != macro_itr){ //確認要檢查的macro不是自己，畢竟自己會跟自己重疊
                        if (check_overlap(move_distance_x+current_pos_x, move_distance_y+current_pos_y, mctype_dict[operating_macro_name].width_x, mctype_dict[operating_macro_name].width_y, component_dict[movable_macro[overlap_itr]].pos_x, component_dict[movable_macro[overlap_itr]].pos_y, mctype_dict[component_dict[movable_macro[overlap_itr]].component_type].width_x, mctype_dict[component_dict[movable_macro[overlap_itr]].component_type].width_y)) {
                            if (mode == "initial") {
                                        move_distance_x = int(floor(move_distance_x/2));
                                        mode == "tune_y";
                                        need_retuning = true;
                                }
                            break;
                            if (mode == "tune_y") {
                                        move_distance_x *= 2;
                                        move_distance_y = int(floor(move_distance_y/2));
                                        mode == "tune_x";
                                        need_retuning = true;
                                }
                            break;
                            if (mode == "tune_x") {
                                        move_distance_x = int(floor(move_distance_x/4));
                                        mode == "tune_y";
                                        need_retuning = true;
                                }
                            break;
                        }
                    }
                } 

                if(!need_retuning){
                    overlap = false;
                    break;
                }
            }
            
            // // check if the macro is in the boundary of the chip and modify the position if it is not
            // if (buffer_pos_x < diearea.init_x){
            //     cout << "\tOut of the left boundary!";
            //     buffer_pos_x = diearea.init_x;
            // }
            // else if (buffer_pos_x + mctype_dict[operating_macro_name].width_x > diearea.end_x){
            //     cout << "\tOut of the right boundary!";
            //     buffer_pos_x = diearea.end_x - mctype_dict[operating_macro_name].width_x;
            // }
            // if (buffer_pos_y < diearea.init_y){
            //     cout << "\tOut of the bottom boundary!";
            //     buffer_pos_y = diearea.init_y;
            // }
            // else if (buffer_pos_y + mctype_dict[operating_macro_name].width_y > diearea.end_y){
            //     cout << "\tOut of the top boundary!";
            //     buffer_pos_y = diearea.end_y - mctype_dict[operating_macro_name].width_y;
            // }

            if (!overlap){
                operating_macro->pos_x = move_distance_x + current_pos_x;
                operating_macro->pos_y = move_distance_y + current_pos_y; 
                cout << "\t<- Update This Macro";
            }
            else
                cout << "\tOverlap!";
            cout << '\n';
            return 0;
        }
    }
    time(&after_algo);

    /********** write output file **********/
    if(mlist2.is_open() && dmp.is_open()){
        cout << "Write Output File\n";
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
                    splitStringToWords(second_line, sec_words);
                    for(int j=0; j<5; ++j) // 5 spaces
                        dmp << " ";
                    for(int j=6; j<14; ++j){ // rest of the sentence
                        dmp << " ";
                        if(j==9)
                            dmp << component_dict[movable_macro[i]].pos_x; // pos_x
                        else if(j==10)
                            dmp << component_dict[movable_macro[i]].pos_y; // pos_y
                        else if(j==12){ // orientation
                            orientation ori = component_dict[movable_macro[i]].orient;
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
    time(&end);
    double time_input = double(after_input-start);
    double time_algo  = double(after_algo-after_input);
    double time_whole = double(end-start);
    if(CAL_TIME){
        cout << "Time(input, algo, whole): " << fixed << time_input << " " << time_algo << " " << time_whole
        << setprecision(3) << " sec\n";
    }
    return 0;
}