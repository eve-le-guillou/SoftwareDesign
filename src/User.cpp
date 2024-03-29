#include <random>
#include <iostream>
#include "User.h"

User::User(int expoParameter, double budget, std::array<bool,5> permission):
        expoParameter(expoParameter),
        nextJobTime(0),
        budget(budget),
        spendings(0),
        permission({permission[0], permission[1], permission[2], permission[3], permission[4]}){
}

bool User::isTime(double time) {
    return ((*this).nextJobTime-time<=0);
}

double User::getBudget() {
    return (*this).budget;
}

double User::getSpendings() {
    return (*this).spendings;
}

void User::generateNewTime(double time) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<> d(1/(double)(*this).expoParameter);
    (*this).nextJobTime = time + d(gen);
}

std::array<bool,5> User::getPermission() {
    return (*this).permission;
}

int User::getExpoParameter() {
    return (*this).expoParameter;
}



