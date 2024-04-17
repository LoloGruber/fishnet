//
// Created by lolo on 3/26/24.
//

#ifndef FISHNET_PROJECT_GDALINITIALIZER_HPP
#define FISHNET_PROJECT_GDALINITIALIZER_HPP
#include <gdal.h>
#include <gdal_priv.h>

namespace fishnet {

class GDALInitializer {
    private:
        static inline bool initialized = false;
    public:
        inline static void init() {
            [[likely]] if (initialized)
                return;
            initialized = true;
            GDALAllRegister();
            // further initialization if necessary
        }
    };
}

#endif //FISHNET_PROJECT_GDALINITIALIZER_HPP