#include <exception>
#include <thread>

#include "project_manager.h"
#include <gtest/gtest.h>

TEST(ProjectManager, CreateDeleteProjects) {
    ProjectManager& projectManager = ProjectManager::getInstance();

    projectManager.newProject("prj1");

    EXPECT_EQ(projectManager.getNumProjects(), 1)
        << "Number of projects does not correspond ";

    EXPECT_STREQ(projectManager.getCurrentProject()->getName().c_str(), "prj1")
        << "Name of project is not correct";

    projectManager.newProject("prj2");

    EXPECT_EQ(projectManager.getNumProjects(), 2)
        << "Number of projects does not correspond ";

    EXPECT_STREQ(projectManager.getCurrentProject()->getName().c_str(), "prj1")
        << "Name of project is not correct";

}