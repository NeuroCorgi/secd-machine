#include <iostream>
#include <fstream>

#include "vm/value.h"
#include "assembler/file.h"
#include "assembler/attribute.h"
#include "assembler/value.h"

secd::assembler::value::attribute_list attributes;

void read_operands(std::istream& is, secd::value::operands_list& ops) {
    ops.clear();
    
    char paren;
    is >> paren;
    if (paren != '(') {
        throw std::runtime_error("Operands list expected to start with '('");
    }

    std::string rand;
    is >> rand;
    while (!rand.ends_with(')')) {
        ops.push_back(attributes.index<secd::assembler::value::attribute::Id>(rand));
        is >> rand;
    }
    
    if (rand != ")") {
        auto sub = rand.substr(0, rand.length() - 1);
        ops.push_back(attributes.index<secd::assembler::value::attribute::Id>(sub));
    }
}

std::istream& secd::assembler::value::operator>>(std::istream& is, secd::assembler::value::instruction& ins) {
    using namespace secd::assembler::value;
    
    std::string name;
    is >> name;
    ins.opcode = secd::opcodes::opcode_code(name);
    ins.operands = {};
    switch (ins.opcode) {
    case secd::opcodes::SEL: {
        std::string then_b, else_b;
        is >> then_b >> else_b;

        auto then_index = attributes.index<attribute::Code>(then_b);

        auto else_index = attributes.index<attribute::Code>(else_b);

        ins.operands = {then_index, else_index};
        break;
    }
    case secd::opcodes::LDC: {
        int constant;
        is >> constant;

        auto const_index = attributes.index<attribute::Int>(std::to_string(constant));
        attributes[const_index].set<attribute::Int>(constant);

        ins.operands = {const_index};
        break;
    }
    case secd::opcodes::ST:
    case secd::opcodes::LDE: {
        std::string name;
        is >> name;

        auto name_index = attributes.index<attribute::Id>(name);
        attributes[name_index].set<attribute::Id>(name);

        ins.operands = {name_index};
        break;
    }
    case secd::opcodes::LDF: {
        std::string body;
        std::vector<uint16_t> ops;
        is >> body;
        read_operands(is, ops);

        auto body_index = attributes.index<attribute::Code>(body);

        // TODO: remove "ops", it is impossible to have several functions now
        auto operands_index = attributes.index<attribute::Operands>("ops");
        attributes[operands_index].set<attribute::Operands>(ops);

        ins.operands = {body_index, operands_index};
        break;
    }
    default:
        break;
    }
    return is;
}

std::istream& secd::assembler::value::operator>>(std::istream& is, secd::assembler::value::block& block) {
    using namespace secd::assembler::value;
    
    std::string label;
    is >> label;
    if (!label.ends_with(':')) {
        throw std::runtime_error("Label expected, got " + label);
    }
    label = label.substr(0, label.length() - 1); // Label without colon at the end
    std::cout << "Pure Label: " << label << "\n";
    block.label = label;
    block.instructions = {};

    for (auto iter = std::istream_iterator<instruction>(is); iter->opcode != secd::opcodes::STP; iter++) {
        block.instructions.push_back(*iter);
    }
    block.instructions.emplace_back(secd::opcodes::STP, secd::value::operands_list{});

    auto block_index = attributes.index<attribute::Code>(label);
    std::cout << "index: " << block_index << "\n";
    attributes[block_index].set<attribute::Code>(block);
    std::cout << "length: " << attributes[block_index].length << "\n\n";

    
    for (const auto& ins : block.instructions) {
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

void write_attribute(std::ofstream& ofs, const secd::assembler::value::attribute& a) {
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

    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " <input-file> <output-file>\n";
        return 1;
    }

    std::ifstream ifs{argv[1]};

    // Main should always be the last code block
    for (
        auto iter = std::istream_iterator<secd::assembler::value::block>(ifs);
        iter->label != "main";
        iter++
    );

    std::ofstream ofs{argv[2]};

    write_magic(ofs);
    write_attributes(ofs);

    return 0;
}
