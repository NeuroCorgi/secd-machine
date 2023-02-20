#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>

#include "machine.h"
#include "opcode.h"
#include "registers.h"
#include "value.h"

struct instruction {
    secd::opcodes::OpCode opcode{secd::opcodes::NON};
    secd::value::operands_list operands;
};
using block = std::vector<instruction>;

struct attribute {
    uint16_t length;
    enum Tag {
        Int      = 0x11,
        Id       = 0x22,
        Code     = 0x33,
        Operands = 0x44,
    } tag;
    std::unique_ptr<const char> payload;

    template<attribute::Tag Tag, typename T>
    void set(T&& value) {
        using payload_type = typename std::remove_reference<T>::type;

        length = sizeof(payload_type) + 1;
        tag = Tag;

        payload_type* temp = new payload_type{value};
        payload = std::unique_ptr<const char>(reinterpret_cast<const char *>(temp));
    }

    template<attribute::Tag Tag>
    void set(std::string& value) {
        length = static_cast<uint16_t>(value.length() + 1);
        tag = Tag;

        char* temp = new char[length];
        strncpy(temp, value.c_str(), length - 1);
        payload = std::unique_ptr<const char>(temp);
    }

    template<attribute::Tag Tag>
    void set(std::string&& value) {
        length = static_cast<uint16_t>(value.length() + 1);
        tag = Tag;

        char* temp = new char[length];
        strncpy(temp, value.c_str(), length - 1);
        payload = std::unique_ptr<const char>(temp);
    }

    template<attribute::Tag Tag>
    void set(block& block) {
        static_assert(Tag == attribute::Code, "List of instructions must be a code block");
        tag = Tag;

        std::ostringstream oss;

        for (const auto& ins : block) {
            oss.write(reinterpret_cast<const char *>(&(ins.opcode)), sizeof(uint8_t));
            for (const auto& op : ins.operands) {
                oss.write(reinterpret_cast<const char *>(&op), sizeof(uint16_t));
            }
        }
        auto str = oss.str();

        length = static_cast<uint16_t>(str.length() + 1);

        char *temp = new char[length];
        memcpy(temp, str.c_str(), length);
        payload = std::unique_ptr<const char>(temp);
    }

    template<attribute::Tag Tag>
    void set(secd::value::operands_list& ops) {
        static_assert(Tag == attribute::Operands, "Operand list must have operands tag");
        
        length = ops.size() * sizeof(uint16_t) + 1;
        tag = Tag;

        std::ostringstream oss;
        for (const auto& op : ops) {
            oss.write(reinterpret_cast<const char*>(&op), sizeof(uint16_t));
        }
        auto str = oss.str();

        char* temp = new char[length];
        strncpy(temp, str.c_str(), length);
        payload = std::unique_ptr<const char>(temp);
    }
        
};

template<attribute::Tag tag>
std::string mangle(std::string name);

template<>
std::string mangle<attribute::Int>(std::string name) {
    return name + "_const";
}

template<>
std::string mangle<attribute::Id>(std::string name) {
    return name + "_id";
}

template<>
std::string mangle<attribute::Code>(std::string name) {
    return name + "_block";
}

template<>
std::string mangle<attribute::Operands>(std::string name) {
    return name + "_ops";
}

class attributes {
public:
    template<attribute::Tag Tag>
    uint16_t index(std::string name) {
        auto mangled_name = mangle<Tag>(name);
        if (!_indices.contains(mangled_name)) {
            _indices[mangled_name] = _attributes.size();
            _attributes.push_back(attribute{0, Tag, nullptr});
        }
        return _indices[mangled_name];
    }   

    attribute& operator[](uint16_t index) {
        return _attributes[index];
    }

    uint16_t size() const {
        return _attributes.size();
    }

    auto begin() const {
        return _attributes.begin();
    }
    
    auto end() const {
        return _attributes.end();
    }

private:
    std::vector<attribute> _attributes;
    std::unordered_map<std::string, uint16_t> _indices;
} attributes;

std::istream& operator>>(std::istream& is, secd::value::operands_list& ops) {
    char paren;
    is >> paren;
    if (paren != '(') {
        throw std::runtime_error("Operands list expected to start with '('");
    }

    std::string rand;
    is >> rand;
    while (!rand.ends_with(')')) {
        ops.push_back(attributes.index<attribute::Id>(rand));
        is >> rand;
    }
    
    if (rand != ")") {
        auto sub = rand.substr(0, rand.length() - 1);
        ops.push_back(attributes.index<attribute::Id>(sub));
    }

    return is;
}

std::istream& operator>>(std::istream& is, instruction& i) {
    std::string name;
    is >> name;
    i.opcode = secd::opcodes::opcode_code(name);
    i.operands = {};
    switch (i.opcode) {
    case secd::opcodes::SEL: {
        std::string then_b, else_b;
        is >> then_b >> else_b;

        auto then_index = attributes.index<attribute::Code>(then_b);

        auto else_index = attributes.index<attribute::Code>(else_b);

        i.operands = {then_index, else_index};
        break;
    }
    case secd::opcodes::LDC: {
        int constant;
        is >> constant;

        auto const_index = attributes.index<attribute::Int>(std::to_string(constant));
        attributes[const_index].set<attribute::Int>(constant);

        i.operands = {const_index};
        break;
    }
    case secd::opcodes::ST:
    case secd::opcodes::LDE: {
        std::string name;
        is >> name;

        auto name_index = attributes.index<attribute::Id>(name);
        attributes[name_index].set<attribute::Id>(name);

        i.operands = {name_index};
        break;
    }
    case secd::opcodes::LDF: {
        std::string body;
        secd::value::operands_list ops;
        is >> body >> ops;

        auto body_index = attributes.index<attribute::Code>(body);

        auto operands_index = attributes.index<attribute::Operands>("ops");
        attributes[operands_index].set<attribute::Operands>(ops);

        i.operands = {body_index, operands_index};
        break;
    }
    default:
        break;
    }
    return is;
}

std::istream& operator>>(std::istream& is, block&) {
    block block{};
    std::string label;
    is >> label;
    if (!label.ends_with(':')) {
        throw std::runtime_error("Label expected, got " + label);
    }
    auto pure_label = label.substr(0, label.length() - 1); // Label without colon at the end
    std::cout << "Pure Label: " << pure_label << "\n";

    instruction ins;
    do {
        is >> ins;
        block.push_back(ins);
    } while (ins.opcode != secd::opcodes::STP);

    auto block_index = attributes.index<attribute::Code>(pure_label);
    std::cout << "index: " << block_index << "\n";
    attributes[block_index].set<attribute::Code>(block);
    std::cout << "length: " << attributes[block_index].length << "\n\n";

    for (const auto& ins : block) {
        std::cout << "OPCODE: " << secd::opcodes::opcode_name(ins.opcode) << " "
                  << ins.operands.size() << "\n";
    }
    std::cout << "\n";
    
    return is;
}

void write_magic(std::ofstream& ofs) {
    uint32_t magic = 0x1CEDC0DE;
    ofs.write(reinterpret_cast<const char*>(&magic), sizeof(uint32_t));
}

void write_attribute(std::ofstream& ofs, const attribute& a) {
    ofs.write(reinterpret_cast<const char*>(&a.length), sizeof(uint16_t));
    ofs.write(reinterpret_cast<const char*>(&a.tag), sizeof(uint8_t));
    ofs.write(a.payload.get(), a.length - 1);
}

void write_attributes(std::ofstream& ofs) {
    uint16_t length = attributes.size();
    ofs.write(reinterpret_cast<const char *>(&length), sizeof(uint16_t));

    for (const auto& a : attributes) {
        write_attribute(ofs, a);
    }
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cerr << "Program should be run with only one argument: the function file name.\n";
        std::cerr << (argc - 1) << " was provided.\n";
        return 1;
    }

    std::ifstream ifs{argv[1]};

    block b;
    ifs >> b;
    ifs >> b;
    ifs >> b;
    ifs >> b;

    std::ofstream ofs{"aaa.func"};
    std::cout << "\n";

    for (const auto& at : attributes) {
        std::cout << at.length << " ";
        switch (at.tag) {
        case attribute::Int:
            std::cout << "int";
            break;
        case attribute::Id:
            std::cout << "id";
            break;
        case attribute::Operands:
            std::cout << "ops";
            break;
        case attribute::Code:
            std::cout << "code";
            break;
        }
        std::cout << "\n";
    }

    write_magic(ofs);
    write_attributes(ofs);

    return 0;
}
