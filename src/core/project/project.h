#ifndef BM_SEGMENTER_PROJECT_H
#define BM_SEGMENTER_PROJECT_H

#include <string>

class Project {
private:
    std::string name_;
    std::string description_;
    std::string save_file_;
public:
    Project(std::string &name, std::string &description);

    /**
     * Returns the current name of the project
     * @return name of project
     */
    std::string& getName() { return name_; }

    /**
     * Returns the current description of the project
     * @return description of project
     */
    std::string& getDescription() { return description_; }

    /**
     * Returns the save file if it has been precised
     * @return
     */
    std::string& getSaveFile() { return save_file_; }

    /**
     * Sets the current save file for this project
     * @param save_file path of the file
     */
    void setSaveFile(std::string& save_file) {
        save_file_ = save_file;
    }

    /**
     * Sets the name of the projects
     * @param name name of project
     */
    void setName(std::string name) { name_ = name; }
};


#endif //BM_SEGMENTER_PROJECT_H
