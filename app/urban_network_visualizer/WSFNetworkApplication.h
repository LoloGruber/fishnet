#include "util/Parameters.h"
#include "utils/Geography/OGRDatasetWrapper.h"
#include <ogr_core.h>

class WSFNetworkApplication
{
private:
    PathHelper * tmp;
    std::unique_ptr<OGRDatasetWrapper> area;
    std::unique_ptr<OGRDatasetWrapper> imp;
    std::unique_ptr<OGRDatasetWrapper> pop;
    Parameters::PathPointer output;
    OGRSpatialReference *spatialRef;
    std::vector<std::shared_ptr<CentralityMeasure>> centralityMeasures;
    bool merge;
    struct NetworkConfiguration networkConfig;
    
public:
    WSFNetworkApplication(int argc, char * argv[]);

    void run();
    ~WSFNetworkApplication();
};


