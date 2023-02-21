#ifndef SECD_ASM_VALUE_H
#define SECD_ASM_VALUE_H

#include <vector>
#include <unordered_map>

#include "common.h"

#include "assembler/util.h"
#include "assembler/file.h"
#include "assembler/attribute.h"

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
