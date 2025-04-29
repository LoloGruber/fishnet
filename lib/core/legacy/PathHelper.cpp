//
// Created by grube on 20.09.2021.
//

#include "PathHelper.h"

PathHelper *PathHelper::INSTANCE = nullptr;

std::unique_ptr<boost::filesystem::path> PathHelper::toAbsolute(const boost::filesystem::path &p, bool checkExists) {
    if (not checkExists) {
        return std::make_unique<boost::filesystem::path>(boost::filesystem::absolute(p));
    }
    auto pAbs = std::make_unique<boost::filesystem::path>(p);
    if (not boost::filesystem::exists(p)) {
        pAbs = std::move(std::make_unique<boost::filesystem::path>(PathHelper::WORKING_DIR_PATH() / *pAbs)); //if path was relative to current working directory, try this
        if (boost::filesystem::exists(*pAbs)) {
            return std::move(std::make_unique<boost::filesystem::path>(boost::filesystem::absolute(*pAbs)));
        } else {
            return nullptr;
        }
    }
    return std::move(std::make_unique<boost::filesystem::path>(boost::filesystem::absolute(*pAbs)));
}

std::unique_ptr<boost::filesystem::path> PathHelper::toAbsolute(std::string &p, bool checkExists) {
    return toAbsolute(boost::filesystem::path(p),checkExists);
}


// void Temp_Directory::instantiate(const boost::filesystem::path& working_dir) {
//    fs::path absolute = fs::absolute(working_dir);
//    Temp_Directory::INSTANCE = new Temp_Directory(absolute);
//}
