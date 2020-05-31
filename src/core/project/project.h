#ifndef BM_SEGMENTER_PROJECT_H
#define BM_SEGMENTER_PROJECT_H

#include <string>

class Project {
private:
    std::string name_;
    std::string description_;
public:
    Project(std::string &name, std::string &description);

    /**
     * Returns the current name of the project
     * @return name of project
     */
    std::string& getName() { return name_; }

    /**
     * Sets the name of the projects
     * @param name name of project
     */
    void setName(std::string name) { name_ = name; }
};


#endif //BM_SEGMENTER_PROJECT_H
