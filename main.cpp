#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
#include <bitset>
#include <algorithm>
#include <typeinfo>
#include <assert.h>
#include <stdlib.h>

using namespace std;

const int MEMORY_SIZE = 0x0600000;
const int DATA_SEGMENT = 0x0500000;
const int TEXT_SEGMENT = 0x0400000;

/*class for storing address of label */
class Label{

    public:

        /* Constructor */
        Label(string name, int32_t address, string data_type = "instruction", string content = ""){
        this->name = name;
        this->address = address;
        this->data_type = data_type;
        this->content = content;
        }

        /* Getters */
        std::string getName(){
            return name;
        }

        int32_t getAddress(){
            return address;
        }

        string getData_type(){
            return data_type;
        }

        string getContent(){
            return content;
        }

        void changeAddress(int32_t new_address){
            address = new_address;
        }

    private:

        /* Instace variables */
        std::string name;
        int32_t address;
        std::string data_type;
        std::string content;

        /* Overloading */
        friend bool operator==(Label label, string string);
        friend bool operator==(string string, Label label);

};

/* Overloading functions */
bool operator==(Label label, string string){

    return label.getName() == string;

}

bool operator==(string string, Label label){

    return label.getName() == string;

}

vector <Label> label_list;

vector<string> data_types {

    ".ascii", ".asciiz", ".word", ".byte", ".half"

};


map<string, int> register_dir = {

    {"$zero", 0}, {"$at", 1}, {"$v0", 2}, {"$v1", 3}, {"$a0", 4}, {"$a1", 5}, {"$a2", 6}, {"$a3", 7},
    {"$t0", 8}, {"$t1", 9}, {"$t2", 10}, {"$t3", 11}, {"$t4", 12}, {"$t5", 13}, {"$t6", 14}, {"$t7", 15},
    {"$s0", 16}, {"$s1", 17}, {"$s2", 18}, {"$s3", 19}, {"$s4", 20}, {"$s5", 21}, {"$s6", 22}, {"$s7", 23},
    {"$t8", 24}, {"$t9", 25}, {"$k0", 26}, {"$k1", 27}, {"$gp", 28}, {"$sp", 29}, {"$fp", 30}, {"$ra", 31}

};

vector<string> dataType_list {

    ".ascii", ".asciiz", ".word", ".byte", ".half"

};

map<string, string> r_type {

    {"add", "100000"}, {"addu", "100001"}, {"and", "100100"}, {"div", "011010"}, {"divu", "011011"},
    {"jalr", "001001"}, {"jr", "001000"}, {"mfhi", "010000"}, {"mflo", "010010"}, {"mthi", "010001"},
    {"mtlo", "010011"}, {"mult", "011000"}, {"multu", "011001"}, {"nor", "100111"}, {"or", "100101"},
    {"sll", "000000"}, {"sllv", "000100"}, {"slt", "101010"}, {"sltu", "101011"}, {"sra", "000011"},
    {"srav", "000111"}, {"srl", "000010"}, {"srlv", "000110"}, {"sub", "100010"}, {"subu", "100011"},
    {"syscall", "001100"}, {"xor", "100110"}, {"clo","100001"}, {"clz", "100000"}, {"mul", "011000"},
    {"madd", "000000"}, {"maddu", "000001"}, {"msub", "000100"}, {"msubu", "000101"}, {"teq", "110100"},
    {"tne", "110110"}, {"tge", "110000"}, {"tgeu", "110001"}, {"tlt", "110010"}, {"tltu", "110011"}

};

map<string, string> i_type {

    {"addi", "001000"}, {"addiu", "001001"}, {"andi", "001100"}, {"beq", "000100"}, {"bgez", "000001"},
    {"bgtz", "000111"}, {"blez", "000110"}, {"bltz", "000001"}, {"bne", "000101"}, {"lb", "100000"},
    {"lbu", "100100"}, {"lh", "100001"}, {"lhu", "100101"}, {"lui", "001111"}, {"lw", "100011"},
    {"ori", "001101"}, {"sb", "101000"}, {"slti", "001010"}, {"sltiu", "001011"},{"teqi", "000001"},
    {"sh", "101001"}, {"sw", "101011"}, {"swcl", "111001"}, {"xori", "001110"}, {"tnei", "000001"},
    {"tgei", "000001"}, {"tgeiu", "000001"}, {"tlti", "000001"}, {"tltiu", "000001"}, {"lwr", "100110"}, 
    {"lwl", "100010"}, {"ll", "110000"}, {"swr", "101110"}, {"swl", "101010"}, {"sc", "111000"},
    {"bgezal", "000001"},{"bltzal", "000001"}

};

map<string, string> j_type {
    
    {"j", "000010"}, {"jal", "000011"}
    
};  

/* Looks for the address of the label from the label list */
int32_t label_finder(string label){
    
    for(auto & elem: label_list){

        if(label == elem){

            return elem.getAddress();

        }
    
    }

    return -1;
}

/* Changes the register to its corresponding 5-bit address */
string reg_bit(string reg){

    string result;

    if (reg.length() == 5 && reg!="$zero") return reg;

    if (reg == "0"){
        return "00000";
    } else{
        auto it = register_dir.find(reg);
        result = bitset<5>(it->second).to_string();
        return result;
    }

}

string makeR_type(string instruction, string rd, string rs, string rt, string shamt, string funct){

    string op;
    if (instruction == "clo" || instruction == "clz" || instruction == "mul" || instruction == "madd" || instruction == "maddu" || instruction == "msub" || instruction == "msubu"){
        op = "011100";
    }
    else{
        op = "000000";
    }
    string dest_reg = reg_bit(rd);
    string first_reg = reg_bit(rs);
    string second_reg = reg_bit(rt);
    int temp = stoi(shamt);
    string shift_amt = bitset<5>(temp).to_string();

    return op + first_reg + second_reg + dest_reg + shift_amt + funct;

}

string makeI_type(string instruction, string op, string rt, string rs, string immediate){
    string dest_reg = reg_bit(rt);
    string first_reg = reg_bit(rs);
    int temp = stoi(immediate);
    string imm_val = bitset<16>(temp).to_string();
    return op + first_reg + dest_reg + imm_val;
}

string makeJ_type(string op, string address){
    int temp = stoi(address);
    string address_val = bitset<26>(temp).to_string();

    return op + address_val;
}

/* Find and erase line that contains said char*/
string findChar(string arg, string str){

    int found = str.find(arg);

    while (true){
        if (found == -1)break;
        else{
            str.erase(found);
            found=str.find(arg,found);
        }
    }
    return str;
}

/*deleting comment using findChar*/
string deletecomment (string str){

    string output = findChar("#", str);
    return output;
}

/*saving the address of label, and erasing it from the input*/
string save_address (string input, int & line_counter){
    string output = findChar(".", input);

    int found = input.find(":");

    while (true){
        if (found == -1)break;
        else{
            Label address (output.substr(0,found), 0x400000 + (line_counter) * 4);
            label_list.push_back(address);
            output.erase(0,found+1);
            found=output.find(":",found);
        }
    }
    return output;
}

void save_data (string input){
    int found_colon = input.find(":");
    string title = input.substr(0, found_colon);
    string content = input.substr(found_colon+1);
    vector <string> token;
    content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
    content.erase(std::remove(content.begin(), content.end(), '\"'), content.end());
    content.erase(std::remove(content.begin(), content.end(), '\t'), content.end());
    int pos = 0;
    for (int i =0; i < content.length(); i++){
        if (content[i] == ' '){
            int len = i-pos;
            string append = content.substr(pos,len);
            if (append != ""){
                token.push_back(append);
            }
            pos= i+1;
        }
    }
    token.push_back(content.substr(pos));

    for(const auto & data_type: dataType_list){
        if (data_type == token[0]){
            int content_size = token.size();
            string data_content;
            for (int i = 1; i < content_size; i++ ){
                data_content = data_content + token[i];
            }
            Label address(title, 0, data_type, data_content);
            label_list.push_back(address);
            break;
        }
    }
}

/*dividing tokens and saving it in a vector*/
void divide_tokens (string output, vector <vector<string>> & token, int &line_counter){
    vector <string> column;
    output.erase(std::remove(output.begin(), output.end(), '\t'), output.end());
    output.erase(std::remove(output.begin(), output.end(), ','), output.end());
    int length_line = output.length();
    int pos = 0;

    for (int i = 0; i < length_line; i++){
        if (output[i] == ' '){
            int len = i-pos;
            string append = output.substr(pos,len);
            if (append != ""){
                column.push_back(append);    
            }
            pos = i+1;
        }
    }

    column.push_back(output.substr(pos));
    if (column[0] != ""){
        token.push_back(column);
        line_counter++;
    } 
}

void machineCode_gen(vector<string> token, vector<string> & machine_code, int line_counter){
    map<string, string>::iterator iterator;

    if (token[0] == "add"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "addu"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "addi"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, token[1], token[2], token[3]);
        machine_code.push_back(result);
    }
    else if (token[0] == "addiu"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, token[1], token[2], token[3]);
        machine_code.push_back(result);
    }
    else if (token[0] == "and"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "andi"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, token[1], token[2], token[3]);
        machine_code.push_back(result);
    }
    else if (token[0] == "clo"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "clz"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "div"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "divu"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result); 
    }
    else if (token[0] == "mult"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "multu"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "mul"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "madd"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result);   
    }
    else if (token[0] == "msub"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "maddu"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "msubu"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "nor"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "or"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "ori"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, token[1], token[2], token[3]);
        machine_code.push_back(result);
    }
    else if (token[0] == "sll"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], token[3], iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "sllv"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "sra"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], token[3], iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "srav"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "srl"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], token[3], iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "srlv"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "sub"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "subu"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "xor"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "xori"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, token[1], token[2], token[3]);
        machine_code.push_back(result);
    }
    else if (token[0] == "lui"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, 0, token[1], token[2]);
        machine_code.push_back(result);
    }
    else if (token[0] == "slt"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "sltu"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2], token[3], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "slti"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, token[1], token[2], token[3]);
        machine_code.push_back(result);
    }
    else if (token[0] == "sltiu"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, token[1], token[2], token[3]);
        machine_code.push_back(result);
    }
    // page 658 textbook
    else if (token[0] == "beq"){
        iterator = i_type.find(token[0]);
        int label_address = label_finder(token[3]);

        if (label_address == -1){
            string result = makeI_type(token[0], iterator->second, token[2], token[1], token[3]);
            machine_code.push_back(result);
        } 
        else{
            int rel_address = label_address - (0x400000 + ((line_counter *4) + 4));
            string result = makeI_type(token[0], iterator->second, token[2], token[1], to_string(rel_address/4));
            machine_code.push_back(result);
        }
    }
    else if (token[0] == "bgez"){
        iterator = i_type.find(token[0]);
        int label_address = label_finder(token[2]);

        if (label_address == -1){
            string result = makeI_type(token[0], iterator->second, "00001", token[1], token[2]);
            machine_code.push_back(result);
        } 
        else{
            int rel_address = label_address - (0x400000 + ((line_counter *4) + 4));
            string result = makeI_type(token[0], iterator->second, "00001", token[1], to_string(rel_address/4));
            machine_code.push_back(result);
        }
    }
    else if (token[0] == "bgezal"){
        iterator = i_type.find(token[0]);
        int label_address = label_finder(token[2]);

        if (label_address == -1){
            string result = makeI_type(token[0], iterator->second, "10001", token[1], token[2]);
            machine_code.push_back(result);
        } 
        else{
            int rel_address = label_address - (0x400000 + ((line_counter *4) + 4));
            string result = makeI_type(token[0], iterator->second, "10001", token[1], to_string(rel_address/4));
            machine_code.push_back(result);
        }
    }
    else if (token[0] == "bgtz"){
        iterator = i_type.find(token[0]);
        int label_address = label_finder(token[2]);

        if (label_address == -1){
            string result = makeI_type(token[0], iterator->second, "00000", token[1], token[2]);
            machine_code.push_back(result);
        } 
        else{
            int rel_address = label_address - (0x400000 + ((line_counter *4) + 4));
            string result = makeI_type(token[0], iterator->second, "00000", token[1], to_string(rel_address/4));
            machine_code.push_back(result);
        }
    }
    else if (token[0] == "blez"){
        iterator = i_type.find(token[0]);
        int label_address = label_finder(token[2]);

        if (label_address == -1){
            string result = makeI_type(token[0], iterator->second, "00000", token[1], token[2]);
            machine_code.push_back(result);
        } 
        else{
            int rel_address = label_address - (0x400000 + ((line_counter *4) + 4));
            string result = makeI_type(token[0], iterator->second, "00000", token[1], to_string(rel_address/4));
            machine_code.push_back(result);
        }
    }
    else if (token[0] == "bltzal"){
        iterator = i_type.find(token[0]);
        int label_address = label_finder(token[2]);

        if (label_address == -1){
            string result = makeI_type(token[0], iterator->second, "10000", token[1], token[2]);
            machine_code.push_back(result);
        } 
        else{
            int rel_address = label_address - (0x400000 + ((line_counter *4) + 4));
            string result = makeI_type(token[0], iterator->second, "10000", token[1], to_string(rel_address/4));
            machine_code.push_back(result);
        }
    }
    else if (token[0] == "bltz"){
        iterator = i_type.find(token[0]);
        int label_address = label_finder(token[2]);

        if (label_address == -1){
            string result = makeI_type(token[0], iterator->second, "00000", token[1], token[2]);
            machine_code.push_back(result);
        } 
        else{
            int rel_address = label_address - (0x400000 + ((line_counter *4) + 4));
            string result = makeI_type(token[0], iterator->second, "00000", token[1], to_string(rel_address/4));
            machine_code.push_back(result);
        }
    }
    else if (token[0] == "bne"){
        iterator = i_type.find(token[0]);
        int label_address = label_finder(token[3]);

        if (label_address == -1){
            string result = makeI_type(token[0], iterator->second, token[2], token[1], token[3]);
            machine_code.push_back(result);
        } 
        else{
            int rel_address = label_address - (0x400000 + ((line_counter *4) + 4));
            string result = makeI_type(token[0], iterator->second, token[2], token[1], to_string(rel_address/4));
            machine_code.push_back(result);
        }
    }
    else if (token[0] == "j"){
        iterator = j_type.find(token[0]);
        int label_address = label_finder(token[1]);

        if(label_address == -1){
            string result = makeJ_type(iterator->second,token[1]);
            machine_code.push_back(result);
        }
        else{
            string result = makeJ_type(iterator->second,to_string(label_finder(token[1])/4));
            machine_code.push_back(result);
        }
    }
    else if (token[0] == "jal"){
        iterator = j_type.find(token[0]);
        int label_address = label_finder(token[1]);

        if(label_address == -1){
            string result = makeJ_type(iterator->second,token[1]);
            machine_code.push_back(result);
        }
        else{
            string result = makeJ_type(iterator->second,to_string(label_finder(token[1])/4));
            machine_code.push_back(result);
        }
    }
    else if (token[0] == "jalr"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], token[2],"0", "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "jr"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1],"0", "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "teq"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "teqi"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, "01100", token[1], token[2]);
        machine_code.push_back(result);
    }
    else if (token[0] == "tne"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "tnei"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, "01110", token[1], token[2]);
        machine_code.push_back(result);
    }
    else if (token[0] == "tge"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "tgeu"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "tgei"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, "01000", token[1], token[2]);
        machine_code.push_back(result);
    }
    else if (token[0] == "tgeiu"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, "01001", token[1], token[2]);
        machine_code.push_back(result);
    }
    else if (token[0] == "tlt"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "tltu"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], token[2], "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "tlti"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, "01010", token[1], token[2]);
        machine_code.push_back(result);
    }
    else if (token[0] == "tltiu"){
        iterator = i_type.find(token[0]);
        string result = makeI_type(token[0], iterator->second, "01011", token[1], token[2]);
        machine_code.push_back(result);
    }
    else if (token[0] == "lb"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "lbu"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "lh"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "lhu"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "lw"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "lwl"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "lwr"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "ll"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "sb"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "sh"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "sw"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "swl"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "swr"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "sc"){
        iterator = i_type.find(token[0]);
        
        size_t open_bracket = token[2].find('(');
        size_t close_bracket = token[2].find(')');
        string rs = token[2].substr(open_bracket + 1, close_bracket - open_bracket - 1);
        string imm = token[2].substr(0, open_bracket);
        
        string result = makeI_type(token[0], iterator->second, token[1], rs, imm);
        machine_code.push_back(result);
    }
    else if (token[0] == "mfhi"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], "0", "0", "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "mflo"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], token[1], "0", "0", "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "mthi"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], "0", "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "mtlo"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", token[1], "0", "0", iterator->second);
        machine_code.push_back(result);
    }
    else if (token[0] == "syscall"){
        iterator = r_type.find(token[0]);
        string result = makeR_type(token[0], "0", "0", "0", "0", iterator->second);
        machine_code.push_back(result);
    }
}


/* simulator section*/

struct memory_PC{
    vector<string> memory;
    int text;
    int text_end;
    int data;
    int data_end;
    vector<int> registers;

};

void initialize(memory_PC &ram){
    ram.memory.resize(MEMORY_SIZE/4);   //each line is 4 bytes, so the memory is divided by 4
    ram.text = TEXT_SEGMENT/4;
    ram.text_end = TEXT_SEGMENT/4;
    ram.data = DATA_SEGMENT/4;        // data segment starts after 5MB
    ram.data_end = DATA_SEGMENT/4;     // for text_end and data_end we first initialized on the beginning of the segment
    ram.registers.resize(32);
    ram.registers[29] = ram.text_end;       //stack pointer
    ram.registers[30] = ram.text;           // frame pointer
}
        
/* get line of machine code from vector and store it to memory*/
void store_text(memory_PC &memory, vector<string> temp){
    for (int i = 0; i < temp.size(); i++){
        int limit = memory.data;
        int current = memory.text_end;
        if (current <= limit){
            memory.memory[memory.text_end] = temp[i];
            memory.text_end += 1;
            memory.registers[29] = memory.text_end;
        }
    }
}

/*store data from the .data section*/
void store_data(memory_PC &memory, vector<Label> &address){
    for (int i=0; i<address.size();i++){
        int limit = MEMORY_SIZE/4;
        int current = memory.data_end;

        if (address[i].getData_type() == ".ascii"){
            string content = address[i].getContent();
            address[i].changeAddress(memory.data_end);
            int content_length = content.length(); 
            for (int n = 0; n<content_length; n+=4){
                int remainder = content_length - n - 1;
                if (remainder >= 4){
                    memory.memory[memory.data_end] = content.substr(n,4);
                    memory.data_end += 1;           //pointer move by 4
                    memory.registers[29] = memory.text_end;
                }
                else if(remainder < 4){     //if the ascii doesn't take the whole block of memory
                    string final_content = content.substr(n);
                    while (remainder < 4){
                        final_content = final_content + "-";
                        remainder++;
                    }
                    memory.memory[memory.data_end] = final_content;
                    memory.data_end+=1;     //pointer move by 4
                    memory.registers[29] = memory.text_end;
                }
            }
        }
        else if (address[i].getData_type() == ".asciiz"){
            string content = address[i].getContent() + '\0';
            address[i].changeAddress(memory.data_end);
            int content_length = content.length(); 
            for (int n = 0; n<content_length; n+=4){
                int remainder = content_length - n;
                if (remainder >= 4){
                    memory.memory[memory.data_end] = content.substr(n,4);
                    memory.data_end += 1;           //pointer move by 4
                    memory.registers[29] = memory.text_end;
                }
                else if(remainder <4){          //if the asciiz doesn't take the whole block of memory
                    string final_content = content.substr(n);
                    while (remainder <= 4){
                        final_content = final_content + "-";
                        remainder++;
                    }
                    memory.memory[memory.data_end] = final_content;
                    memory.data_end+=1;         //pointer move by 4
                    memory.registers[29] = memory.text_end;
                }
            }
        }
        else if (address[i].getData_type() == ".word"){
            string content = address[i].getContent();
            address[i].changeAddress(memory.data_end);
            content.erase(std::remove(content.begin(), content.end(), ','), content.end());
            for (int n = 0; n<content.length(); n++){
                memory.memory[memory.data_end] = content[n];
                memory.data_end += 1;           //pointer move by 4
                memory.registers[29] = memory.text_end;
            }
        }
        else if (address[i].getData_type() == ".half"){
            string content = address[i].getContent();
            address[i].changeAddress(memory.data_end);
            content.erase(std::remove(content.begin(), content.end(), ','), content.end());
            int content_length = content.length(); 
            for (int n = 0; n<content_length; n+=2){
                int remainder = content_length - n;
                if (remainder >= 2){
                    memory.memory[memory.data_end] = content.substr(n,2);
                    memory.data_end += 1;           //pointer move by 4
                    memory.registers[29] = memory.text_end;
                }
                else if(remainder < 2){         //if the half doesn't take the whole block of memory
                    string final_content = content.substr(n);
                    while (remainder <= 2){
                        final_content = final_content + "-";
                        remainder++;
                    }
                    memory.memory[memory.data_end] = final_content;
                    memory.data_end+=1;         //pointer move by 4
                    memory.registers[29] = memory.text_end;
                }
            }
        }
        else if (address[i].getData_type() == ".byte"){
            string content = address[i].getContent();
            address[i].changeAddress(memory.data_end);
            content.erase(std::remove(content.begin(), content.end(), ','), content.end());
            int content_length = content.length(); 
            for (int n = 0; n<content_length; n+=4){
                int remainder = content_length - n;
                if (remainder >= 4){
                    memory.memory[memory.data_end] = content.substr(n,4);
                    memory.data_end += 1;           //pointer move by 4
                }
                else if(remainder < 4){     //if the byte doesn't take the whole block of memory
                    string final_content = content.substr(n);
                    while (remainder <= 4){
                        final_content = final_content + "-";
                        remainder++;
                    }
                    memory.memory[memory.data_end] = final_content;
                    memory.data_end+=1;         //pointer move by 4
                }
            }
        }
    }
}

/*main program to run the instructions from the memory*/
void run_instructions(memory_PC &ram){
    int PC = ram.text;      //program counter
    while (PC <= ram.text_end){
        string instruction = ram.memory[PC].substr(0,6);
        string first_reg = ram.memory[PC].substr(6,5);
        string second_reg = ram.memory[PC].substr(11,5);
        string third_reg = ram.memory[PC].substr(16,5);
        string shamt = ram.memory[PC].substr(21,5);
        string operation = ram.memory[PC].substr(26,6);


    }
}

int main(int argc, char* argv[]){
    
    if(argc != 4){
        std::cerr << "Error, wrong number of inputs" << endl;
        exit(1);
    }
    
    string input_filename = argv[1];
    string syscall_filename = argv[2];
    string output_filename = argv[3];

    string myText;
    vector <vector<string>> token;
    ifstream inputfile(input_filename);
    ofstream outputFile(output_filename);
    bool segment_checker = true;
    int line_counter = 0;
    vector<string> machine_code;

    /*fetching lines from the .in file*/
    while (getline (inputfile, myText)) {
        string output = deletecomment(myText);

        /*to check whether it's .data section or .text section*/
        if (output.find(".data") != string::npos) segment_checker = false;
        if (output.find(".text") != string::npos) segment_checker = true;

        if (segment_checker == true){
            output = save_address(output, line_counter);
            divide_tokens(output, token, line_counter);

        } 
        if (segment_checker == false){
            if (output != ".data"){

                save_data(output);
            }
        }
    }

    // for (int i = 0; i < token.size(); i++){
    //     for (int j = 0; j < token[i].size(); j++)        for debugging
    //     {
    //         cout << token[i][j] << " ";
    //     }
    //     cout << endl;
    // }
    
    /*generate the machine code*/
    for (int i = 0; i<token.size(); i++){
        machineCode_gen(token[i],machine_code, i);
    }

    for (int i = 0; i<machine_code.size(); i++){
        outputFile << machine_code[i] << endl;
    }

    /* close the input output file for 3.2 */
    inputfile.close();
    outputFile.close();

    /*initialize memory and registers*/
    memory_PC ram;
    initialize(ram);
    store_text(ram, machine_code);
    store_data(ram,label_list);

    // int start = ram.data;
    // int limit = ram.data_end;                    for debugging memory        
    // for (int i = start; i<=limit; i++){
    //     cout<< ram.memory[i]<< endl;
    // }
    // cout << ram.data*4 << " "<< ram.data_end*4 <<endl;


    // int num_label = label_list.size();
    // for (int i = 0; i < num_label; i++){       for debugging
    //     cout << label_list[i].getName() << " " << label_list[i].getData_type()<< " " << label_list[i].getContent()<<" "<< label_list[i].getAddress()<< " "<<endl;
    // }

}   
