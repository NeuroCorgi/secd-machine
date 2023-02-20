#include "registers.h"


namespace secd::registers {

control::control(const std::string& control) : _buffer{control.data()}, _end{_buffer + control.capacity()} {}

bool control::empty() const {
    return _buffer == _end;
}

secd::opcodes::OpCode control::read_opcode() {
    uint8_t code;
    read(reinterpret_cast<char *>(&code), sizeof(uint8_t));
    return static_cast<secd::opcodes::OpCode>(code);
}

uint16_t control::read_operand() {
    uint16_t operand = 0;
    read(reinterpret_cast<char *>(&operand), sizeof(uint16_t));
    return operand;
}

void control::read(char * str, std::size_t count) {
    if (_end - _buffer <= count) {
        throw std::runtime_error("Control : Read : Cannot read");
    }
    std::memcpy(str, _buffer, count);
    _buffer += count;
}

const environment::value_type& environment::get(const std::string& id) const {
    if (_env.contains(id)) {
        return _env.at(id);
    } 
    if (_parent) {
        return (*_parent).get(id);
    }
    throw std::runtime_error("Env : Lookup failed");
}

environment::value_type& environment::get(const std::string& id) {
    if (_env.contains(id)) {
        return _env.at(id);
    } 
    if (_parent) {
        return (*_parent).get(id);
    }
    throw std::runtime_error("Env : Lookup failed");
}

void environment::set(const std::string& id, environment::value_type& val) {
    _env[id] = val;
}

environment environment::derive(std::shared_ptr<environment> parent) {
    environment env;
    env._parent = parent;

    return env;
}

}
