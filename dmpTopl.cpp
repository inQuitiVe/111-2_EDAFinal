// transfer dmp file to pl file
// usage: ./tran caseOO.dmp caseOO.pl out.pl
// argv       0           1         2      3

// .\tran.exe case01.dmp case01.pl out.pl
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

class Final_placement{
    public:
        string name; // only for implementing operator == function
        float pos_x, pos_y;
        string orient; // "N", "FN", "S", or "FS"
        Final_placement(){  // default constructor
            name = "none";
            pos_x = 0.0;
            pos_y = 0.0;
            orient = "N";
        }
        Final_placement(string na, float px, float py, string ori){
            name = na;
            pos_x = px;
            pos_y = py;
            orient = ori;
        }
        bool operator==(const Final_placement & rhs){
            return this->name == rhs.name;
        }
        ~Final_placement(){};
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
    if(argc != 4){
        cout << "usage: ./tran caseOO.dmp caseOO.pl out.pl\n";
        return 0;
    }

    /********** read input files **********/
    int num_components = 0;
    ifstream dmp(argv[1]);
    ifstream pl(argv[2]);
    ofstream out(argv[3]);
    string line;
    string word;
    unordered_map<string, Final_placement> placement_dict; 
    
    if(dmp.is_open()){
        string cur_state = "INIT";
        vector<string> words, sec_words;
        string first_line, second_line;
        while(getline(dmp, line)){
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
                for(int i=0; i<num_components; ++i){
                    getline(dmp, first_line);
                    getline(dmp, second_line);
                    splitStringToWords(first_line, words);
                    splitStringToWords(second_line, sec_words);
                    Final_placement fp(words[4], stof(sec_words[9]), stof(sec_words[10]), sec_words[12]);
                    placement_dict[words[4]] = fp;
                }
                break; // end of reading dmp
            }
        }
        dmp.close();
    }


    /********** write output file **********/
    if(pl.is_open() && out.is_open()){
        string cur_state = "INIT";
        vector<string> words, sec_words;
        string first_line, second_line;
        unordered_map<string, Final_placement>::const_iterator it;

        // write first 3 lines
        for(int i=0; i<3; ++i){
            getline(pl, line);
            out << line << "\n";
        }
        // write the rest of the file
        while(getline(pl, line)){
            word = line.substr(0, line.find(" ")); // first word of the sentense
            it = placement_dict.find(word);
            if(it == placement_dict.end()) // components no need to be modified
                out << line << "\n";
            else // modify pos_x, pos_y, orient according to placement_dict
                out << word << " " << placement_dict[word].pos_x << " " << placement_dict[word].pos_y << " : " << placement_dict[word].orient << "\n";    
        }
        pl.close();
        out.close();
    }
    return 0;
}



