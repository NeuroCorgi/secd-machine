#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "machine.h"

#include "opcode.h"
#include "registers.h"
#include "value.h"

bool check_magic(std::ifstream& ifs) {
    uint32_t magic;
    ifs.read(reinterpret_cast<char *>(&magic), sizeof(uint32_t));
    return magic == 0x1CEDC0DE;
}

uint16_t read_attributes_length(std::ifstream& ifs) {
    uint16_t length;
    ifs.read(reinterpret_cast<char *>(&length), sizeof(uint16_t));
    return length;
}

secd::value::attribute read_attribute(std::ifstream& ifs) {
    uint16_t length = 0;
    uint8_t  tag    = 0;
    ifs.read(reinterpret_cast<char *>(&length), sizeof(uint16_t));
    ifs.read(reinterpret_cast<char *>(&tag), sizeof(uint8_t));

    switch (tag) {
    case 0x11: { // Integer 
        int x;
        ifs.read(reinterpret_cast<char *>(&x), length - 1);
        return x;
    }
    case 0x22: { // Variable id
        char* d = new char[length + 1]{0};
        ifs.read(d, length - 1);
        std::string x{d};
        delete[] d;
        return x;
    }
    case 0x33: { // Code
        char* d = new char[length];
        ifs.read(d, length - 1);
        std::string x; x.reserve(length);
        std::memcpy(x.data(), d, length - 1);
        delete[] d;
        return x;
    }
    case 0x44: { // Operands list
        std::vector<uint16_t> operand_indecies;
        for (std::size_t i = 0; i < (length - 1) / sizeof(uint16_t); ++i) {
            uint16_t ind;
            ifs.read(reinterpret_cast<char *>(&ind), sizeof(uint16_t));
            operand_indecies.push_back(ind);
        }
        return operand_indecies;
    }
    default:
        break;
    }
}

int main(int argc, char* argv[]) {

   if (argc != 2) {
       std::cerr << "Program should be run with only one argument: the function file name.\n";
       std::cerr << (argc - 1) << " was provided.\n";
       return 1;
   }

    std::ifstream ifs{argv[1]};

    if (!check_magic(ifs)) {
        std::cerr << "Unknown file format\n";
        return 2;
    }

    auto length = read_attributes_length(ifs);
    secd::value::attribute_list attributes{length};

    for (auto i = 0; i < length; ++i) {
        attributes[i] = std::make_shared<secd::value::attribute>(read_attribute(ifs));
    }

    secd::state state{attributes};

    try {
        secd::run(state);
    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    if (state.stack.empty()) {
        std::cout << "Stack is empty\n";
    } else {
        auto val = *state.stack.top();
        switch (val.index()) {
        case secd::value::attribute_type::Int:
            std::cout << std::get<int>(*state.stack.top()) << "\n";
            break;
        case secd::value::attribute_type::Bool:
            std::cout << std::get<bool>(*state.stack.top()) << "\n";
            break;
        default:
            std::cout << "Unprintable value on top of the stack\n";
            break;
        }
    }
}
