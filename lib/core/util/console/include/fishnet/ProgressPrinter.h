#pragma once
namespace fishnet::util {
/**
 * Class to show the user the estimated progress of a iteration / pipeline step
 */
class ProgressPrinter {
private:
    unsigned long goal; //number of iterations to be reached
    unsigned long counter = 0;
    int printed = 0;
    const std::string task; //name of the task that is executed

    void initialize();

public:
    static bool disable; //ProgressPrinter is disable when in --silent mode, default: ProgressPrinter is enabled

    ProgressPrinter(unsigned long goal,std::string  task);

    explicit ProgressPrinter(std::string  task);

    ~ProgressPrinter();

    void visit();
};
}
