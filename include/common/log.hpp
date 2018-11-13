#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <stdio.h> 
#include <string> 
#include <initializer_list>

std::string log(std::initializer_list<std::string> a_args) {
    std::string s = "";
    bool even = false;
    for(auto a: a_args) {
        if(even) {
            s += "=\""+a+"\"";
        } else {
            s += ", "+a;
        }
        even = !even;
    }
    s += "\n";
    return s;
}

void LOG_INFO(long ts, std::initializer_list<std::string> a_args) {
    printf("[%ld]: level=\"INFO\"%s", ts, log(a_args).c_str());
}

void LOG_WARNING(long ts, std::initializer_list<std::string> a_args) {
    printf("[%ld]: level=\"WARNING\"%s", ts, log(a_args).c_str());
}

void LOG_ERROR(long ts, std::initializer_list<std::string> a_args) {
    printf("[%ld]: level=\"ERROR\"%s", ts, log(a_args).c_str());
}

void LOG_FATAL(long ts, std::initializer_list<std::string> a_args) {
    printf("[%ld]: level=\"FATAL\"%s", ts, log(a_args).c_str());
    exit(1);
}

#endif