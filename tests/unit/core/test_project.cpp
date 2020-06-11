#include <exception>
#include <thread>

#include "project_manager.h"
#include <gtest/gtest.h>

TEST(ProjectManager, CreateDeleteProjects) {
    ProjectManager& projectManager = ProjectManager::getInstance();

    auto project1 = projectManager.newProject("prj1", "description");

    EXPECT_EQ(projectManager.getNumProjects(), 1)
        << "Number of projects does not correspond ";

    EXPECT_STREQ(projectManager.getCurrentProject()->getName().c_str(), "prj1")
        << "Name of project is not correct";

    auto project2 = projectManager.newProject("prj2", "description");

    EXPECT_EQ(projectManager.getNumProjects(), 2)
        << "Number of projects does not correspond ";

    EXPECT_STREQ(projectManager.getCurrentProject()->getName().c_str(), "prj1")
        << "Name of project is not correct";

    projectManager.removeProject(project1);
    EXPECT_EQ(projectManager.getNumProjects(), 1)
        << "Project1 was not removed correctly";

    projectManager.removeProject(project2);
    EXPECT_EQ(projectManager.getNumProjects(), 0)
        << "Project2 was not removed correctly";
}

TEST(ProjectManager, setAndDuplicateProject) {
    ProjectManager& projectManager = ProjectManager::getInstance();

    auto project1 = projectManager.newProject("prj1", "description1");
    auto project2 = projectManager.newProject("prj2", "description2");

    projectManager.setCurrentProject(project1);

    EXPECT_EQ(projectManager.getCurrentProject(), project1)
        << "The current project has not been set correctly";

    auto projectDuplicate = projectManager.duplicateCurrentProject();

    EXPECT_TRUE(*project1 == *projectDuplicate)
        << "Project was not duplicated correctly";

    // Always remove the create projects for the next tests
    projectManager.removeProject(project1);
    projectManager.removeProject(project2);
    projectManager.removeProject(projectDuplicate);
}

TEST(ProjectManager, SaveOpenProject) {
    ProjectManager& projectManager = ProjectManager::getInstance();
    auto project1 = projectManager.newProject("prj1", "description");

    projectManager.saveProjectToFile(project1, "proj.ml_proj");
    auto project2 = projectManager.openProjectFromFile("proj.ml_proj");

    EXPECT_TRUE(*project1 == *project2)
        << "Project was not saved and opened correctly";
    // Always remove the create projects for the next tests
    projectManager.removeProject(project1);
    projectManager.removeProject(project2);
}