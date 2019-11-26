#pragma once

#include "User.h"
#include <utility>


struct Curriculum {
    double budget;
    int expoParameter;
    int instResourceCap;
    std::array<bool,5> permission;
};

class Student : public User {
private:
    int instResourceCap;

public:
    Student(struct Curriculum curriculum);
    char whatAmI() override;
    int getInstResourceCapt();
};

