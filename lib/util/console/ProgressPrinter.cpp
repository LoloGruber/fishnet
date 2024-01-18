//
// Created by grube on 09.01.2022.
//

#include <iostream>
#include <utility>
#include "ProgressPrinter.h"

bool ProgressPrinter::disable = false;

void ProgressPrinter::visit() {
    if (not disable) {
        counter++;
        if (counter > (goal / 100) * printed) {
            printed++;
            std::cout << "#" << std::flush;
        }
    }

}

ProgressPrinter::ProgressPrinter(unsigned long goal, std::string  task) : goal(goal), task(std::move(task)) {
    initialize();
}

ProgressPrinter::~ProgressPrinter() {
    if (goal != 0) {
        std::cout << std::endl;
    }
    std::cout <<"[Finished]: " << this->task << std::endl << std::endl;
}

void ProgressPrinter::initialize() {
    std::cout << "[Starting]: " << this->task << std::endl;
    std::cout << "0%";
    for (int i = 0; i < 94; i++) {
        std::cout << "_";
    }
    std::cout << "100%" << std::endl;
}

ProgressPrinter::ProgressPrinter(std::string  task):goal(0), task(std::move(task)) {
    std::cout << "[Starting]: " << this->task << std::endl;
}

