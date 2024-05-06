#include "FindNeighboursTask.h"

int main(int argc, char const *argv[])
{
    FindNeighboursTask<fishnet::geometry::Polygon<double>> task;
    task.run();
    return 0;
}
