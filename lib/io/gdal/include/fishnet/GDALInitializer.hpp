#pragma once
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>

namespace fishnet {

/**
 * @brief Singleton to initialize GDAL
 * 
 */
class GDALInitializer {
    private:
        static inline bool initialized = false;
    public:
        inline static void init() {
            [[likely]] if (initialized)
                return;
            initialized = true;
            GDALAllRegister();
            CPLSetConfigOption("OSR_DEFAULT_AXIS_MAPPING_STRATEGY","TRADITIONAL_GIS_ORDER");
        }
};
}