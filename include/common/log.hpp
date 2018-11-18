#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <stdio.h> 
#include <string> 
#include <initializer_list>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

// Function to log conviniently.

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

void LOG_INFO_GREEN(long ts, std::initializer_list<std::string> a_args) {
    printf("%s[%ld]: level=\"INFO\"%s%s", KGRN, ts, log(a_args).c_str(), KNRM);
}

void LOG_INFO_CYAN(long ts, std::initializer_list<std::string> a_args) {
    printf("%s[%ld]: level=\"INFO\"%s%s", KCYN, ts, log(a_args).c_str(), KNRM);
}

void LOG_INFO_MAG(long ts, std::initializer_list<std::string> a_args) {
    printf("%s[%ld]: level=\"INFO\"%s%s", KMAG, ts, log(a_args).c_str(), KNRM);
}

void LOG_INFO_WHITE(long ts, std::initializer_list<std::string> a_args) {
    printf("%s[%ld]: level=\"INFO\"%s%s", KWHT, ts, log(a_args).c_str(), KNRM);
}

void LOG_WARNING(long ts, std::initializer_list<std::string> a_args) {
    printf("%s[%ld]: level=\"WARNING\"%s%s", KYEL, ts, log(a_args).c_str(), KNRM);
}

void LOG_ERROR(long ts, std::initializer_list<std::string> a_args) {
    printf("%s[%ld]: level=\"ERROR\"%s%s", KRED, ts, log(a_args).c_str(), KNRM);
}

void LOG_FATAL(long ts, std::initializer_list<std::string> a_args) {
    printf("%s[%ld]: level=\"FATAL\"%s%s", KRED, ts, log(a_args).c_str(), KNRM);
    exit(1);
}

#endif