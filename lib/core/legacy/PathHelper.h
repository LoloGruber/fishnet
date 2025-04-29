//
// Created by grube on 20.09.2021.
//

#ifndef BACHELORARBEIT_PATHUTILS_H
#define BACHELORARBEIT_PATHUTILS_H

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>

/**
 * Returns constants depending on the working directory
 *
 * Implemented as Singleton
 */
class PathHelper {
private:
    boost::filesystem::path WORKING_DIR;
    boost::filesystem::path TEMP_DIR;
    static PathHelper * INSTANCE;

    explicit PathHelper(boost::filesystem::path & working_dir){
        this->WORKING_DIR = working_dir;
        this->TEMP_DIR = working_dir / "tmp";
        try {
            if (boost::filesystem::is_directory(TEMP_DIR) and boost::filesystem::exists(TEMP_DIR)) {
                boost::filesystem::remove_all(TEMP_DIR);
            }
            boost::filesystem::create_directory(this->TEMP_DIR);
        } catch (boost::filesystem::filesystem_error &filesystem_error) {
            throw std::invalid_argument("TMP Directory " + TEMP_DIR.string() + " could not be created");
        }
    }
    PathHelper() { }

public:
    static PathHelper * instantiate(const boost::filesystem::path& working_dir){
        if (!INSTANCE) {
            boost::filesystem::path absolute = boost::filesystem::absolute(working_dir);
            INSTANCE = new PathHelper(absolute);
            return INSTANCE;
        }
        return INSTANCE;
    }
    /**
     *
     * @return path to temporary directory
     */
    static boost::filesystem::path TMP_PATH(){
        if (PathHelper::INSTANCE == nullptr) {
            throw std::invalid_argument("Temp_Directory not instantiated");
        }
        return PathHelper::INSTANCE->TEMP_DIR;
    }
    /**
     *
     * @return path to working directory
     */
    static boost::filesystem::path WORKING_DIR_PATH(){
        return PathHelper::INSTANCE->WORKING_DIR;
    }

    /**
     *
     * @param p path to be put into absolute form
     * @param checkExists if the function should check if the path exists (default: yes)
     * @return absolute path or null (if checkExists = true and path does not exists)
     */
    static std::unique_ptr<boost::filesystem::path> toAbsolute(const boost::filesystem::path & p,bool checkExists = true);

    /**
     * Overload to call toAbsolute with a string
     * @param p
     * @param checkExists
     * @return
     */
    static std::unique_ptr<boost::filesystem::path>  toAbsolute(std::string & p,bool checkExists = true);

    /**
     * Special deconstructor, to remove the temporary directory after termination of the program
     */
    ~PathHelper(){
        std::cout << "Deleting Temporary Folder "<<TEMP_DIR.string() << std::endl;
        boost::filesystem::remove_all(TEMP_DIR);
    }
};

#endif //BACHELORARBEIT_PATHUTILS_H
