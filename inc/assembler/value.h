#ifndef SECD_ASM_VALUE_H
#define SECD_ASM_VALUE_H

#include <sstream>
#include <unordered_map>

#include "common.h"

#include "vm/opcode.h"
#include "vm/value.h"

namespace secd::assembler::value {

struct instruction {
    instruction() = default;
    instruction(secd::opcodes::OpCode, secd::value::operands_list);
    
    secd::opcodes::OpCode opcode{secd::opcodes::NON};
    secd::value::operands_list operands;

    friend std::istream& operator>>(std::istream&, instruction);
};

struct block {
    std::string label;
    std::vector<instruction> instructions;

    friend std::istream& operator>>(std::istream&, block);
};

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
        tag = Tag;

        std::ostringstream oss;

        for (const auto& ins : block.instructions) {
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

}

// The architecture of this project is completely broken
// Or is it how it really should work?
#include "assembler/util.h"

namespace secd::assembler::value {

class attribute_list {
public:
    template<attribute::Tag Tag>
    uint16_t index(std::string& name) {
        using namespace secd::assembler::util;
        
        auto mangled_name = mangle<Tag>(name);
        if (!_indices.contains(mangled_name)) {
            _indices[mangled_name] = _attributes.size();
            _attributes.emplace_back();
        }
        return _indices[mangled_name];
    }

    template<attribute::Tag Tag>
    uint16_t index(std::string&& name) {
        using namespace secd::assembler::util;
        
        auto mangled_name = mangle<Tag>(name);
        if (!_indices.contains(mangled_name)) {
            _indices[mangled_name] = _attributes.size();
            _attributes.emplace_back();
        }
        return _indices[mangled_name];
    }

    attribute& operator[](uint16_t index);

    uint16_t size() const;

    auto begin() const {
        return _attributes.begin();
    }
    
    auto end() const {
        return _attributes.end();
    }

public:
    std::vector<attribute> _attributes;
    std::unordered_map<std::string, uint16_t> _indices;
};
    
}

#endif // SECD_ASM_VALUE_H
