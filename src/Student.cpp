#include "Student.h"

Student::Student(struct Curriculum curriculum) :
        User(curriculum.expoParameter, curriculum.budget, curriculum.permission),
        instResourceCap(curriculum.instResourceCap)
        {}

int Student::getInstResourceCapt() {
    return (*this).instResourceCap;
}

bool Student::spend(double newSpendings) {
    if ((*this).spendings+newSpendings < (*this).budget) {
        if (newSpendings < (*this).getInstResourceCapt()) {
            (*this).spendings += newSpendings;
            return true;
        }
    }
    return false;
}

