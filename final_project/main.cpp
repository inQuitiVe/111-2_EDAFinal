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

#define MACRO_ITERATION 1
#define INIT_ITERATION 2
#define primary_mul 1
#define CAL_TIME 1     // set to 0 if don't want to measure execution time
#define SHOW_MESSAGE 1 // set to 0 if don't want to see detail info

using namespace std;
typedef pair<float, float> float_pair;

enum orientation
{
    North,
    FlipNorth,
    South,
    FlipSouth
};

orientation ori_list[4] = {North, FlipNorth, South, FlipSouth};
float_pair origin(0.0, 0.0);

void transorient(float_pair compnt_pos, float_pair compnt_size, float_pair pin_pos, orientation orient, float *pin_abs_pos)
{
    if (orient == North)
    {
        pin_abs_pos[0] = compnt_pos.first + pin_pos.first;
        pin_abs_pos[1] = compnt_pos.second + pin_pos.second;
    }
    if (orient == FlipNorth)
    {
        pin_abs_pos[0] = compnt_pos.first + compnt_size.first - pin_pos.first;
        pin_abs_pos[1] = compnt_pos.second + pin_pos.second;
    }
    if (orient == South)
    {
        pin_abs_pos[0] = compnt_pos.first + compnt_size.first - pin_pos.first;
        pin_abs_pos[1] = compnt_pos.second + compnt_size.second - pin_pos.second;
    }
    if (orient == FlipSouth)
    {
        pin_abs_pos[0] = compnt_pos.first + pin_pos.first;
        pin_abs_pos[1] = compnt_pos.second + compnt_size.second - pin_pos.second;
    }
}

bool check_overlap(float a_pos_x, float a_pos_y, float a_width_x, float a_width_y, float b_pos_x, float b_pos_y, float b_width_x, float b_width_y)
{
    float a_down_bound = a_pos_y;
    float a_top_bound = a_pos_y + a_width_y;
    float a_left_bound = a_pos_x;
    float a_right_bound = a_pos_x + a_width_x;
    float b_down_bound = b_pos_y;
    float b_top_bound = b_pos_y + b_width_y;
    float b_left_bound = b_pos_x;
    float b_right_bound = b_pos_x + b_width_x;

    if ((b_top_bound > a_down_bound && b_top_bound < a_top_bound) || (b_down_bound > a_down_bound && b_down_bound < a_top_bound))
        if ((b_left_bound > a_left_bound && b_left_bound < a_right_bound) || (b_right_bound > a_left_bound && b_right_bound < a_right_bound))
            return true;
    if ((a_top_bound > b_down_bound && a_top_bound < b_top_bound) || (a_down_bound > b_down_bound && a_down_bound < b_top_bound))
        if ((a_left_bound > b_left_bound && a_left_bound < b_right_bound) || (a_right_bound > b_left_bound && a_right_bound < b_right_bound))
            return true;

    return false;
}

void handle_overlap(bool *finish_boundary, float *move_distance_x, float *move_distance_y, float a_pos_x, float a_pos_y, float a_width_x, float a_width_y, float b_pos_x, float b_pos_y, float b_width_x, float b_width_y)
{
    float current_pos_x = a_pos_x - int(floor(*move_distance_x));
    float current_pos_y = a_pos_y - int(floor(*move_distance_y));

    float a_down_bound = a_pos_y;
    float a_top_bound = a_pos_y + a_width_y;
    float a_left_bound = a_pos_x;
    float a_right_bound = a_pos_x + a_width_x;

    float current_down_bound = current_pos_y;
    float current_top_bound = current_pos_y + a_width_y;
    float current_left_bound = current_pos_x;
    float current_right_bound = current_pos_x + a_width_x;

    float b_down_bound = b_pos_y;
    float b_top_bound = b_pos_y + b_width_y;
    float b_left_bound = b_pos_x;
    float b_right_bound = b_pos_x + b_width_x;

    int x_max = 0, y_max = 0;
    bool overlap_x = true;
    bool overlap_y = true;

    if (check_overlap(a_pos_x, a_pos_y, a_width_x, a_width_y, b_pos_x, b_pos_y, b_width_x, b_width_y))
    {
        if(SHOW_MESSAGE)
            cout << "\toverlap";
        if (b_pos_x - current_pos_x > 0)
        {
            x_max = b_left_bound - current_right_bound;
            if (x_max <= 0)
                overlap_x = false;
        }
        else
        {
            x_max = (-1) * (current_left_bound - b_right_bound);
            if (x_max >= 0)
                overlap_x = false;
        }
        if (b_pos_y - current_pos_y > 0)
        {
            y_max = b_down_bound - current_top_bound;
            if (y_max <= 0)
                overlap_y = false;
        }
        else
        {
            y_max = (-1) * (current_down_bound - b_top_bound);
            if (y_max >= 0)
                overlap_y = false;
        }

        if (overlap_x && overlap_y)
        {
            if (abs(x_max - *move_distance_x) > abs(y_max - *move_distance_y))
                overlap_x = false;
            else
                overlap_y = false;
        }

        if (overlap_x)
            *move_distance_x = x_max;
        if (overlap_y)
            *move_distance_y = y_max;

        *finish_boundary = false;
    }
}

class Component
{
public:
    string name; // only for implementing operator == function
    float pos_x, pos_y;
    float init_pos_x, init_pos_y;
    bool movable;
    orientation orient;
    string component_type;
    vector<pair<string, string>> pin_connection; // pin_connection[i].first=pin_name, .second=wire_name
    Component()
    { // default constructor
        name = "none";
        pos_x = 0.0;
        pos_y = 0.0;
        init_pos_x = 0.0;
        init_pos_y = 0.0;
        movable = false;
        orient = North;
        component_type = "none";
    }
    Component(string na, float px, float py, bool mov, orientation ori, string mtype)
    {
        name = na;
        pos_x = px;
        pos_y = py;
        init_pos_x = px;
        init_pos_y = py;
        movable = mov;
        orient = ori;
        component_type = mtype;
    }
    void addConnection(string pin_name, string connect_name)
    {
        pair<string, string> p1(pin_name, connect_name);
        pin_connection.push_back(p1);
    }
    bool operator==(const Component &rhs)
    {
        return this->name == rhs.name;
    }
    ~Component() {}
};

class Pin
{ // for def's PI and PO and lef's macro pin
public:
    string name;
    vector<float_pair> pos_list; // pos_list[i].first = pos_x, pos_list[i].second = pos_y
    // orientation ? (not sure if all the orientation would be North in all test cases)
    Pin()
    {
        name = "none";
        float_pair p1(0.0, 0.0);
        pos_list.push_back(p1);
    }
    Pin(string na)
    {
        name = na;
    }
    void addPinLoc(float px, float py)
    {
        float_pair p1(px, py);
        pos_list.push_back(p1);
    }
    bool operator==(const Pin &rhs)
    {
        return this->name == rhs.name;
    }
    ~Pin(){};
};

class Component_type
{
public:
    string name;
    float width_x, width_y;
    // vector<Pin> pin_list;
    unordered_map<string, Pin> pin_list;
    // OBS obstruction, probably need it but ignore it for now
    Component_type()
    {
        name = "none";
        width_x = 0.0;
        width_y = 0.0;
    }
    Component_type(string na)
    {
        name = na;
        width_x = 0.0;
        width_y = 0.0;
    }
    void setWidth(float wx, float wy)
    {
        width_x = wx;
        width_y = wy;
    }
    void addPin(Pin pi)
    {
        pin_list[pi.name] = pi;
    }
    bool operator==(const Component_type &rhs)
    {
        return this->name == rhs.name;
    }
    ~Component_type(){};
};

class Rect
{ // a rectangle from (init_x, init_y) to (end_x, end_y)
public:
    int init_x, init_y, end_x, end_y;
    Rect()
    {
        set(0, 0, 0, 0);
    }
    Rect(int inx, int iny, int edx, int edy)
    {
        set(inx, iny, edx, edy);
    }
    ~Rect(){};
    void set(int inx, int iny, int edx, int edy)
    {
        init_x = inx;
        init_y = iny;
        end_x = edx;
        end_y = edy;
    }
};

// code ref: https://java2blog.com/split-string-space-cpp/#Using_find_and_substr_methods
void splitStringToWords(string str, vector<string> &words, string delimiter = " ")
{
    int start = 0;
    int end = str.find(delimiter);
    words.clear();
    while (end != -1)
    {
        words.push_back(str.substr(start, end - start));
        start = end + delimiter.size();
        end = str.find(delimiter, start);
    }
    words.push_back(str.substr(start, end - start));
}

int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        cout << "Usage: ./DMP caseOO.v caseOO.lef caseOO.def caseOO.mlist caseOO.txt caseOO.dmp\n";
        return 0;
    }

    /********** define variables **********/
    int MAX_DISPLACEMENT;
    int NEG_MAX_DISPLACEMENT;
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
    unordered_map<string, bool> is_macro;
    vector<string> movable_macro;
    vector<string> all_component;
    vector<string> all_macro;
    Rect diearea(0, 0, 0, 0);

    /********** variables related to time **********/
    time_t start, after_input, after_std, after_macro, end;

    /********** read input files **********/
    // readInputFile(def, def_state);
    time(&start);
    cout << "Read Input Files...\n";
    if (def.is_open())
    {
        string cur_state = "INIT";
        vector<string> words, sec_words, third_words;
        string first_line, second_line, third_line, fourth_line;
        while (getline(def, line))
        {
            word = line.substr(0, line.find(" ")); // first word of the sentense
            // switching cur_state
            if (cur_state == "INIT" && word == "COMPONENTS")
                cur_state = "COMPONENTS";
            else if (cur_state == "COMPONENTS" && word == "PINS")
                cur_state = "PINS";
            else if (cur_state == "PINS" && word == "END")
                cur_state = "END";

            // do different things according to cur_state
            if (cur_state == "INIT")
            {
                continue; // do nothing
            }
            else if (cur_state == "COMPONENTS")
            {
                splitStringToWords(line, words);
                num_components = stoi(words[1]);
                all_component.reserve(num_components);
                for (int i = 0; i < num_components; ++i)
                {
                    getline(def, first_line);
                    getline(def, second_line);
                    splitStringToWords(first_line, words);
                    splitStringToWords(second_line, sec_words);
                    Component mac(words[1], stof(sec_words[3]), stof(sec_words[4]), false, North, words[2]);
                    component_dict[words[1]] = mac;
                    all_component.push_back(words[1]);
                }
                getline(def, line); // END COMPONENTS
                getline(def, line); // empty lines
            }
            else if (cur_state == "PINS")
            {
                splitStringToWords(line, words);
                num_pins = stoi(words[1]);
                for (int i = 0; i < num_pins; ++i)
                {
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
            }
            else
            { // cur_state == "END"
                break;
            }
        }
        def.close();
    }

    if (mlist.is_open())
    {
        string cur_state = "INIT";
        vector<string> words, sec_words;
        string first_line, second_line;
        while (getline(mlist, line))
        {
            word = line.substr(0, line.find(" ")); // first word of the sentense
            // switching cur_state
            if (cur_state == "INIT" && word == "UNITS")
                cur_state = "UNITS";
            else if (cur_state == "UNITS" && word == "DIEAREA")
                cur_state = "DIEAREA";
            else if (cur_state == "DIEAREA" && word == "COMPONENTS")
                cur_state = "COMPONENTS";

            // do different things according to cur_state
            if (cur_state == "INIT")
            {
                continue; // do nothing
            }
            else if (cur_state == "UNITS")
            {
                splitStringToWords(line, words);
                if (words.size() > 1)
                {
                    def_scalar = stoi(words[3]);
                }
            }
            else if (cur_state == "DIEAREA")
            {
                splitStringToWords(line, words);
                if (words.size() > 1)
                {
                    diearea.set(stoi(words[2]), stoi(words[3]), stoi(words[6]), stoi(words[7]));
                }
            }
            else if (cur_state == "COMPONENTS")
            {
                splitStringToWords(line, words);
                num_macros = stoi(words[1]);
                movable_macro.reserve(num_macros); // reserving space for # movable macros
                all_macro.reserve(num_macros);
                for (int i = 0; i < num_macros; ++i)
                {
                    getline(mlist, first_line);
                    getline(mlist, second_line);
                    splitStringToWords(first_line, words);
                    splitStringToWords(second_line, sec_words);

                    // FIXED or PLACED: sec_words[7]
                    if (sec_words[7] == "PLACED")
                    {
                        component_dict[words[4]].movable = true; // update bool movable from false to true
                        movable_macro.push_back(words[4]);
                    }

                    component_dict[words[4]].pos_x = stof(sec_words[9]);  // update pos_x
                    component_dict[words[4]].pos_y = stof(sec_words[10]); // update pos_y
                    component_dict[words[4]].init_pos_x = stof(sec_words[9]);  // update pos_x
                    component_dict[words[4]].init_pos_y = stof(sec_words[10]); // update pos_y
                    // need to update the orientation ??
                    all_macro.push_back(words[4]);
                    is_macro[words[4]] = true;
                }
                break; // end of reading mlist
            }
        }
    }

    if (verilog.is_open())
    {
        string cur_state = "INIT";
        vector<string> words;
        while (getline(verilog, line))
        {
            // switching cur_state
            if (cur_state == "INIT" && line == "// Start cells")
            {
                getline(verilog, line); // get the first line of cells
                cur_state = "CELLS";
            }

            // do different things according to cur_state
            if (cur_state == "INIT")
            {
                continue; // do nothing
            }
            else if (cur_state == "CELLS")
            {
                splitStringToWords(line, words);
                int pin_num = words.size() - 4;
                string comp_name = words[1]; // exclude mctype_name, macro_name, (, and );
                for (int i = 0; i < pin_num; ++i)
                {
                    int left_brace_pos = words[i + 3].find("(");
                    int right_brace_pos = words[i + 3].find(")");
                    string pin_name = words[i + 3].substr(1, left_brace_pos - 1);
                    string wire_name = words[i + 3].substr(left_brace_pos + 1, right_brace_pos - left_brace_pos - 1);

                    // add connection only if the component is macro & movable
                    // if (component_dict[comp_name].movable)
                    // {
                    //     component_dict[comp_name].addConnection(pin_name, wire_name);
                    //     stored_wire[wire_name] = 1; // actually the value is not important
                    // }

                    // switch to add connections no mather is std or macro
                    component_dict[comp_name].addConnection(pin_name, wire_name);
                    stored_wire[wire_name] = 1; // actually the value is not important
                }
                if (line == "")
                    break; // end of reading verilog file
            }
        }
        verilog.close();
    }

    if (verilog2.is_open())
    {
        string cur_state = "INIT";
        vector<string> words;
        while (getline(verilog2, line))
        {
            // word = line.substr(0, line.find(" ")); // first word of the sentense
            // switching cur_state
            if (cur_state == "INIT" && line == "// Start cells")
            {
                getline(verilog2, line); // get the first line of cells
                cur_state = "CELLS";
            }

            // do different things according to cur_state
            if (cur_state == "INIT")
            {
                continue; // do nothing
            }
            else if (cur_state == "CELLS")
            {
                splitStringToWords(line, words);
                int pin_num = words.size() - 4;
                string comp_name = words[1];
                for (int i = 0; i < pin_num; ++i)
                {
                    int left_brace_pos = words[i + 3].find("(");
                    int right_brace_pos = words[i + 3].find(")");
                    string pin_name = words[i + 3].substr(1, left_brace_pos - 1);
                    string wire_name = words[i + 3].substr(left_brace_pos + 1, right_brace_pos - left_brace_pos - 1);
                    if (stored_wire.find(wire_name) != stored_wire.end())
                    {
                        pair<string, string> p1(comp_name, pin_name);
                        vector<pair<string, string>> v1;
                        if (connection_dict.find(wire_name) == connection_dict.end())
                        { // the key is new to connection_dict
                            v1.push_back(p1);
                            connection_dict[wire_name] = v1;
                        }
                        else
                        { // the key already exists
                            v1 = connection_dict[wire_name];
                            v1.push_back(p1);
                            connection_dict[wire_name] = v1;
                        }
                    }
                }
                if (line == "")
                    break; // end of reading verilog file
            }
        }
        verilog.close();
    }

    if (lef.is_open())
    {
        string cur_state = "INIT";
        vector<string> words, sec_words;
        string first_line, second_line;
        string macro_name;
        bool units_done = false;
        bool is_standardCell = true;
        while (getline(lef, line))
        {
            word = line.substr(0, line.find(" ")); // first word of the sentense
            // switching cur_state
            if (cur_state == "INIT" && word == "UNITS")
                cur_state = "UNITS";
            else if (cur_state == "UNITS" && word == "MACRO")
                cur_state = "STDCELLS";

            // do different things according to cur_state
            if (cur_state == "INIT")
            {
                continue; // do nothing
            }
            else if (cur_state == "UNITS")
            {
                if (!units_done)
                { // only do once
                    getline(lef, line);
                    splitStringToWords(line, words);
                    lef_scalar = stoi(words[4]);
                    units_done = true;
                }
            }
            else if (cur_state == "STDCELLS")
            {
                splitStringToWords(line, words);
                string macro_name = words[1];
                Component_type mtype(macro_name);

                // get an extra line to ensure is reading standard cell or macro
                getline(lef, line);
                is_standardCell = isalpha(line[1]) ? 0 : 1;

                if (is_standardCell)
                {
                    while (getline(lef, line))
                    { // read inside MACRO ... END MACRO

                        if (line.substr(4, line.find(" ", 4) - 4) == "SIZE")
                        {
                            splitStringToWords(line, words);
                            mtype.setWidth(stof(words[5]) * lef_scalar, stof(words[7]) * lef_scalar);
                        }
                        else if (line.substr(4, line.find(" ", 4) - 4) == "PIN")
                        {
                            string pin_name = line.substr(8, line.find(" ", 8) - 8); // not sure
                            Pin pi(pin_name);
                            while (getline(lef, line))
                            { // read inside PIN ... END PIN
                                if (line.substr(8, line.find(" ", 8) - 8) == "RECT")
                                {
                                    splitStringToWords(line, words);
                                    float mid_x = (stof(words[9]) + stof(words[11])) / 2.0;
                                    float mid_y = (stof(words[10]) + stof(words[12])) / 2.0;
                                    pi.addPinLoc(mid_x * lef_scalar, mid_y * lef_scalar);
                                }
                                else if (line.substr(8, line.find(" ", 8) - 8) == "END")
                                {
                                    mtype.addPin(pi);
                                    break;
                                }
                            }
                        }
                        else if (line.substr(0, line.find(" ", 0)) == "END")
                        {
                            mctype_dict[macro_name] = mtype;
                            getline(lef, line); // read an empty line
                            break;
                        }
                    }
                }
                else
                { // is macro
                    while (getline(lef, line))
                    { // read inside MACRO ... END MACRO

                        if (line.substr(3, line.find(" ", 3) - 3) == "SIZE")
                        {
                            splitStringToWords(line, words);
                            mtype.setWidth(stof(words[4]) * lef_scalar, stof(words[6]) * lef_scalar);
                        }
                        else if (line.substr(1, line.find(" ", 1) - 1) == "PIN")
                        {
                            string pin_name = line.substr(5, line.find(" ", 5) - 5); // not sure
                            Pin pi(pin_name);
                            while (getline(lef, line))
                            { // read inside PIN ... END PIN
                                if (line.substr(3, line.find(" ", 3) - 3) == "RECT")
                                {
                                    splitStringToWords(line, words);
                                    // RECT: init_x=words[4], init_y=words[5], end_x=words[6], end_y=words[7]
                                    float mid_x = (stof(words[4]) + stof(words[6])) / 2.0;
                                    float mid_y = (stof(words[5]) + stof(words[7])) / 2.0;
                                    pi.addPinLoc(mid_x * lef_scalar, mid_y * lef_scalar);
                                }
                                else if (line.substr(1, line.find(" ", 1) - 1) == "END")
                                {
                                    mtype.addPin(pi);
                                    break;
                                }
                            }
                        }
                        else if (line.substr(0, line.find(" ", 0)) == "END")
                        {
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

    if (txt.is_open())
    {
        string line;
        getline(txt, line, ' ');
        getline(txt, line, ' ');
        MAX_DISPLACEMENT = stoi(line);
        MAX_DISPLACEMENT *= def_scalar; // change to the same unit
        NEG_MAX_DISPLACEMENT = -1 * MAX_DISPLACEMENT;
        txt.close();
    }
    time(&after_input);

    /********** estimate standard cell location (PI and PO already fixed) **********/
    cout << "Determine STD Location...\n";
    bool show_first_std = true; // only show info about first std
    for (int init_iter = 0; init_iter < INIT_ITERATION; init_iter++)
    {
        cout << "num_iter: " << init_iter << "\n";
        show_first_std = true;
        for (int std_itr = 0; std_itr != all_component.size(); std_itr++)
        {
            int std_accum_num = 0;
            float std_xaccum = 0.0;
            float std_yaccum = 0.0;
            string opearting_std_name = all_component[std_itr];
            if(SHOW_MESSAGE && show_first_std)
                cout << "\tSTD_cell #" << std_itr << "\t" << opearting_std_name;
            if (is_macro.find(opearting_std_name) == is_macro.end())
            { // only run if current component is standard cell
                Component *operating_std = &component_dict[opearting_std_name];
                string operating_std_type_name = operating_std->component_type;
                vector<pair<string, string>> operating_connection = operating_std->pin_connection;
                for (int pin_idx = 0; pin_idx < operating_connection.size(); pin_idx++)
                {
                    string connect_pin_name = operating_connection[pin_idx].first;
                    string connect_wire = operating_connection[pin_idx].second;
                    vector<pair<string, string>> wire_to_all_connections = connection_dict[connect_wire];
                    Component_type operating_std_type = mctype_dict[operating_std_type_name];
                    Pin connect_pin = operating_std_type.pin_list[connect_pin_name];

                    float_pair std_size(operating_std_type.width_x, operating_std_type.width_y);
                    // calculate std_pin_relative_pos
                    float std_pin_relative_pos[2];
                    transorient(origin, std_size, connect_pin.pos_list[0], operating_std->orient, std_pin_relative_pos);

                    if (is_primary_io.find(connect_wire) != is_primary_io.end())
                    { // pin is PI/PO
                        if(SHOW_MESSAGE && show_first_std)
                            cout << "IO add " << (pin_dict[connect_wire].pos_list[0].first - std_pin_relative_pos[0]) << "\t";
                        std_xaccum += (pin_dict[connect_wire].pos_list[0].first - std_pin_relative_pos[0]) * primary_mul;
                        std_yaccum += (pin_dict[connect_wire].pos_list[0].second - std_pin_relative_pos[1]) * primary_mul;
                        std_accum_num += primary_mul;
                    }

                    for (int desti_idx = 0; desti_idx != wire_to_all_connections.size(); desti_idx++)
                    {
                        string desti_component_name = wire_to_all_connections[desti_idx].first;
                        if (desti_component_name == opearting_std_name)
                            continue; // do nothing

                        Component desti_component = component_dict[desti_component_name];
                        string desti_component_type_name = desti_component.component_type;
                        string desti_pin_name = wire_to_all_connections[desti_idx].second;
                        Pin desti_pin = mctype_dict[desti_component_type_name].pin_list[desti_pin_name];

                        float_pair component_pos(desti_component.pos_x, desti_component.pos_y);
                        float_pair component_size(mctype_dict[desti_component_type_name].width_x, mctype_dict[desti_component_type_name].width_y);

                        float pin_abs_pos[2];
                        transorient(component_pos, component_size, desti_pin.pos_list[0], desti_component.orient, pin_abs_pos);

                        // 相對距離 = 接到的pin的絕對位置 - 相對macro的pin位置
                        // TODO: modify the weight of macro
                        std_xaccum += (pin_abs_pos[0] - std_pin_relative_pos[0]);
                        std_yaccum += (pin_abs_pos[1] - std_pin_relative_pos[1]);
                        std_accum_num++;
                    }
                }

                std_xaccum /= std_accum_num;
                std_yaccum /= std_accum_num;
                int optimal_std_pos_x = std_xaccum;
                int optimal_std_pos_y = std_yaccum;
                orientation optimal_orient = operating_std->orient; // do not modify orientation
                int current_std_pos_x = int(operating_std->pos_x);  // only for debug usage
                int current_std_pos_y = int(operating_std->pos_y);
                float x_diff = optimal_std_pos_x - current_std_pos_x;
                float y_diff = optimal_std_pos_y - current_std_pos_y;
                float optimal_total_displacement = abs(x_diff) + abs(y_diff);
                // TODO: check if optimal position exceeds boundary (not sure)
                if ((optimal_std_pos_x < diearea.init_x) || (optimal_std_pos_x + mctype_dict[operating_std_type_name].width_x > diearea.end_x)
                || (optimal_std_pos_y < diearea.init_y) || (optimal_std_pos_y + mctype_dict[operating_std_type_name].width_y > diearea.end_y))
                {
                    // cout << "STD cell #" << std_itr << " moves out of boundary!\n";
                    // restore position
                    optimal_std_pos_x = current_std_pos_x;
                    optimal_std_pos_y = current_std_pos_y;
                }
                
                if(SHOW_MESSAGE && show_first_std){
                    cout << "\n\toriginal_std_pos_x:" << current_std_pos_x << ' ' << "\toriginal_std_pos_y: " << current_std_pos_y << '\n';
                    cout << "\toptimal_std_pos_x:" << optimal_std_pos_x << ' ' << "\toptimal_std_pos_y: " << optimal_std_pos_y << '\n';
                    cout << "\toptimal_orient: " << optimal_orient; // same as original orient
                    cout << "\tSTD Moving " << optimal_total_displacement << "\n";
                    show_first_std = false;
                }
                
                // operating_std->orient = optimal_orient;
                operating_std->pos_x = optimal_std_pos_x;
                operating_std->pos_y = optimal_std_pos_y;
            }
        }
    }
    time(&after_std);

    /********** determine final Macro location **********/
    cout << "Determine Macro Location...\n";
    for (int iteration = 0; iteration < MACRO_ITERATION; iteration++)
    {
        cout << "num_iter: " << iteration << "\n";
        for (int macro_itr = 0; macro_itr != movable_macro.size(); macro_itr++)
        {
            int accum_num = 0;
            float xaccum[4] = {0, 0, 0, 0}; // N, FN, S, FS
            float yaccum[4] = {0, 0, 0, 0};
            string operating_macro_name = movable_macro[macro_itr];
            Component *operating_macro = &component_dict[operating_macro_name];
            string operating_macro_type = operating_macro->component_type;
            vector<pair<string, string>> operating_connection = operating_macro->pin_connection;
            if(SHOW_MESSAGE)
                cout << "\tMacro #" << macro_itr << "\t" << operating_macro_name;
            for (int pin_idx = 0; pin_idx < operating_connection.size(); pin_idx++)
            {
                string connect_pin = operating_connection[pin_idx].first;
                string connect_wire = operating_connection[pin_idx].second;
                vector<pair<string, string>> wire_to_all_connections = connection_dict[connect_wire];
                Component_type operating_macro_type_name = mctype_dict[operating_macro_type];
                Pin pin_connect_pin = operating_macro_type_name.pin_list[connect_pin];

                float_pair macro_size(operating_macro_type_name.width_x, operating_macro_type_name.width_y);
                // calculate macro_pin_relative_pos
                float macro_pin_relative_pos[4][2];
                for (int i = 0; i < 4; i++)
                {
                    transorient(origin, macro_size, pin_connect_pin.pos_list[0], ori_list[i], macro_pin_relative_pos[i]);
                }

                if (is_primary_io.find(connect_wire) != is_primary_io.end())
                { // pin is PI/PO
                    for (int i = 0; i < 4; i++)
                    {
                        if(SHOW_MESSAGE)
                            cout << "IO add " << (pin_dict[connect_wire].pos_list[0].first - macro_pin_relative_pos[i][0]) << "\t";
                        xaccum[i] += (pin_dict[connect_wire].pos_list[0].first - macro_pin_relative_pos[i][0]);
                        yaccum[i] += (pin_dict[connect_wire].pos_list[0].second - macro_pin_relative_pos[i][1]);
                    }
                    accum_num++;
                }

                for (int desti_idx = 0; desti_idx != wire_to_all_connections.size(); desti_idx++)
                {
                    string desti_macro_name = wire_to_all_connections[desti_idx].first;
                    if (desti_macro_name == (operating_macro->name))
                        continue;

                    Component desti_macro = component_dict[desti_macro_name];
                    string desti_macro_type = desti_macro.component_type;
                    string desti_pin_name = wire_to_all_connections[desti_idx].second;
                    Pin desti_pin = mctype_dict[desti_macro_type].pin_list[desti_pin_name];

                    float_pair compnt_pos(desti_macro.pos_x, desti_macro.pos_y);
                    float_pair compnt_size(mctype_dict[desti_macro_type].width_x, mctype_dict[desti_macro_type].width_y);

                    float pin_abs_pos[4][2];
                    for (int i = 0; i < 4; i++)
                        transorient(compnt_pos, compnt_size, desti_pin.pos_list[0], desti_macro.orient, pin_abs_pos[i]);

                    for (int i = 0; i < 4; i++)
                    {
                        // 相對距離 = 接到的pin的絕對位置 - 相對macro的pin位置
                        // cout << pin_abs_pos[i][0] << " " << macro_pin_relative_pos[i][0] << "\t";
                        // cout << pin_abs_pos[i][1] << " " << macro_pin_relative_pos[i][1] << "\t";
                        xaccum[i] += (pin_abs_pos[i][0] - macro_pin_relative_pos[i][0]);
                        yaccum[i] += (pin_abs_pos[i][1] - macro_pin_relative_pos[i][1]);
                    }
                    accum_num++;
                }
            }

            // iteration的目的是destination的pinpos可能會因為macro移動而改變，但平常計算仍需要用initial pos來計算
            xaccum[0] /= accum_num;
            yaccum[0] /= accum_num;
            int optimal_pos_x = xaccum[0];
            int optimal_pos_y = yaccum[0];
            orientation optimal_orient = ori_list[0];
            int initial_pos_x = int(operating_macro->init_pos_x); // get pos_x, pos_y from def file, if unit->smallest, then int is ok
            int initial_pos_y = int(operating_macro->init_pos_y);
            int current_pos_x = int(operating_macro->pos_x);
            int current_pos_y = int(operating_macro->pos_y);

            // cout << '\t' << accum_num << "  ";
            // find optimal pos_x, pos_y, and orient
            for (int i = 1; i < 4; ++i)
            { // i=0:North, i=1:FlipNorth, i=2:South, i=3:FlipSouth
                xaccum[i] /= accum_num;
                yaccum[i] /= accum_num;
                if ((abs(optimal_pos_x - initial_pos_x) + abs(optimal_pos_y - initial_pos_y)) >
                    (abs(xaccum[i] - initial_pos_x) + abs(yaccum[i] - initial_pos_y)))
                {
                    optimal_pos_x = xaccum[i]; // int<-float problem(?)
                    optimal_pos_y = yaccum[i];
                    optimal_orient = ori_list[i];
                }
            }
            operating_macro->orient = optimal_orient;
            float x_diff = optimal_pos_x - initial_pos_x;
            float y_diff = optimal_pos_y - initial_pos_y;
            float optimal_total_displacement = abs(x_diff) + abs(y_diff);
            if(SHOW_MESSAGE){
                cout << "\n\toptimal_pos_x:" << optimal_pos_x << ' ' << "\toptimal_pos_y: " << optimal_pos_y << '\n';
                cout << "\toptimal_orient: " << optimal_orient;
                cout << "\tMove " << optimal_total_displacement << " (without constraint)\n";
            }

            float move_distance_x = 0.0, move_distance_y = 0.0;
            // determine move distance x and y
            if (optimal_total_displacement > MAX_DISPLACEMENT)
            {
                float xy_diff = abs(x_diff) - abs(y_diff); // distance moving first, let rest of xy be square
                float remain_displacement = (MAX_DISPLACEMENT - abs(xy_diff)) / 2;

                if (xy_diff >= MAX_DISPLACEMENT) // x>y
                {
                    move_distance_x = (x_diff > 0) ? MAX_DISPLACEMENT : NEG_MAX_DISPLACEMENT;
                    move_distance_y = 0;
                }
                else if (xy_diff <= -1 * MAX_DISPLACEMENT) // y>x
                {
                    move_distance_y = (y_diff > 0) ? MAX_DISPLACEMENT : NEG_MAX_DISPLACEMENT;
                    move_distance_x = 0;
                }
                else if (xy_diff > 0) // MAX_DISPLACEMENT > xy_diff > 0
                {
                    move_distance_x = (x_diff > 0) ? floor(xy_diff + remain_displacement) : (-floor(xy_diff + remain_displacement));
                    move_distance_y = (y_diff > 0) ? floor(remain_displacement) : (-floor(remain_displacement));
                }
                else // -MAX_DISPLACEMENT < xy_diff < 0
                {
                    float movedis = floor(abs(xy_diff) + remain_displacement);
                    move_distance_y = (y_diff > 0) ? movedis : -movedis;
                    move_distance_x = (x_diff > 0) ? floor(remain_displacement) : (-floor(remain_displacement));
                }
            }
            else // optimal_total_displacement <= MAX_DISPLACEMENT
            {    // directly move the macro to the optimal position
                move_distance_x = floor(x_diff);
                move_distance_y = floor(y_diff);
            }

            bool overlap = false;
            float buffer_pos_x, buffer_pos_y;

            for (int i = 0; i < 100; i++)
            {
                bool finish_boundary = false;
                buffer_pos_x = int(floor(move_distance_x)) + initial_pos_x;
                buffer_pos_y = int(floor(move_distance_y)) + initial_pos_y;
                for (int overlap_itr = 0; overlap_itr != movable_macro.size(); overlap_itr++)
                {
                    if (buffer_pos_x < diearea.init_x)
                    {
                        cout << "\tOut of the left boundary!";
                        // buffer_pos_x = diearea.init_x;
                        move_distance_x = diearea.init_x - initial_pos_x;
                    }
                    else if (buffer_pos_x + mctype_dict[operating_macro_name].width_x > diearea.end_x)
                    {
                        cout << "\tOut of the right boundary!";
                        // buffer_pos_x = diearea.end_x - mctype_dict[operating_macro_name].width_x;
                        move_distance_x = diearea.end_x - mctype_dict[operating_macro_name].width_x - initial_pos_x;
                    }

                    if (buffer_pos_y < diearea.init_y)
                    {
                        cout << "\tOut of the bottom boundary!";
                        // buffer_pos_y = diearea.init_y;
                        move_distance_y = diearea.init_y - initial_pos_y;
                    }
                    else if (buffer_pos_y + mctype_dict[operating_macro_name].width_y > diearea.end_y)
                    {
                        cout << "\tOut of the top boundary!";
                        // buffer_pos_y = diearea.end_y - mctype_dict[operating_macro_name].width_y;
                        move_distance_y = diearea.end_y - mctype_dict[operating_macro_name].width_y - initial_pos_y;
                    }

                    if (overlap_itr != macro_itr) //確認要檢查的macro不是自己，畢竟自己會跟自己重疊
                        handle_overlap(&finish_boundary, &move_distance_x, &move_distance_y, buffer_pos_x, buffer_pos_y, mctype_dict[operating_macro_name].width_x, mctype_dict[operating_macro_name].width_y, component_dict[movable_macro[overlap_itr]].pos_x, component_dict[movable_macro[overlap_itr]].pos_y, mctype_dict[component_dict[movable_macro[overlap_itr]].component_type].width_x, mctype_dict[component_dict[movable_macro[overlap_itr]].component_type].width_y);
                }
                if (finish_boundary)
                    break;
            }

            operating_macro->pos_x = int(floor(move_distance_x)) + initial_pos_x;
            operating_macro->pos_y = int(floor(move_distance_y)) + initial_pos_y;
        }
    }
    time(&after_macro);

    /********** write output file **********/
    if (mlist2.is_open() && dmp.is_open())
    {
        cout << "Write Output File...\n";
        dmp << fixed << setprecision(0);
        string cur_state = "INIT";
        vector<string> words, sec_words;
        string first_line, second_line;
        while (getline(mlist2, line))
        {
            word = line.substr(0, line.find(" ")); // first word of the sentense
            // switching cur_state
            if (cur_state == "INIT" && word == "COMPONENTS")
                cur_state = "COMPONENTS";
            // do different things according to cur_state
            if (cur_state == "INIT")
            {
                dmp << line << "\n";
            }
            else if (cur_state == "COMPONENTS")
            {
                dmp << line << "\n"; // COMPONENTS 175 ;
                for (int i = 0; i < num_macros; ++i)
                {                                // num_macros is calculated previously
                    getline(mlist2, first_line); // directly wirte same first line to dmp file
                    dmp << first_line << "\n";
                    getline(mlist2, second_line); // change position to new calculated and write to dmp file
                    splitStringToWords(second_line, sec_words);
                    for (int j = 0; j < 5; ++j) // 5 spaces
                        dmp << " ";
                    for (int j = 6; j < 14; ++j)
                    { // rest of the sentence
                        dmp << " ";
                        if (j == 9)
                            dmp << component_dict[movable_macro[i]].pos_x; // pos_x
                        else if (j == 10)
                            dmp << component_dict[movable_macro[i]].pos_y; // pos_y
                        else if (j == 12)
                        { // orientation
                            orientation ori = component_dict[movable_macro[i]].orient;
                            if (ori == North)
                                dmp << "N";
                            else if (ori == FlipNorth)
                                dmp << "FN";
                            else if (ori == South)
                                dmp << "S";
                            else if (ori == FlipSouth)
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
        while (getline(mlist2, line))
        {
            dmp << line << "\n";
        }
        mlist2.close();
        dmp.close();
    }
    time(&end);
    double time_input = double(after_input - start);
    double time_std = double(after_std - after_input);
    double time_macro = double(after_macro - after_std);
    double time_whole = double(end - start);
    if (CAL_TIME)
    {
        cout << "Time(input, std, macro, whole): " << fixed << "(" << time_input << ", " << time_std << ", "
             << time_macro << ", " << time_whole << ")" << setprecision(3) << " sec\n";
    }
    return 0;
}