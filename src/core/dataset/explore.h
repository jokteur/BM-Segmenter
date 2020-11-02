#pragma once

#include <string>
#include <vector>

#include "events.h"
#include "jobscheduler.h"

#define JOB_EXPLORE_NAME explore_dataset

 namespace core {
     namespace dataset {
         /**
          * A flat representation of a medical case
          * (PatientID, Study, Series, Image Number)
          * Here using the DICOM nomenclature (Camel Case)
          */
         struct Case {
             std::string patientID;         // Patient ID, can be a number, of some sort
             std::string studyDate;         // Date of study
             std::string studyTime;         // Time of study
             std::string studyDescription;  // Description of the study
             std::string seriesNumber;              // Number that identifies the series in the study
             std::string modality;          // Modality of the series (CT, MR, etc.)
             std::string path;              // Path to the DICOMDIR of dicom image
             std::string instanceNumber;            // Image instance number in the series
         };

         struct SeriesNode;
         struct StudyNode;
         struct PatientNode;
         /**
          * A tree representation of a medical cases
          */
         struct ImageNode {
             std::string path;
             std::string number;
             int tree_count;
         };
         struct SeriesNode {
             std::string modality;
             std::string number;
             std::vector<ImageNode> images;
             int tree_count;
         };
         struct StudyNode {
             std::string date;
             std::string time;
             std::string description;
             std::vector<SeriesNode> series;
             int tree_count;
         };
         struct PatientNode {
             std::string ID;
             std::vector<StudyNode> study;
             int tree_count;
         };

         struct SeriesPayload {
             SeriesNode series;
             Case case_;
         };

         class ExplorerBuildEvent : public Event {
         private:
             std::vector<core::dataset::PatientNode>& cases_;
         public:
             explicit ExplorerBuildEvent(std::vector<core::dataset::PatientNode>& cases) :
                     cases_(cases), Event("dataset/explorer/build") {}

             std::vector<core::dataset::PatientNode>& getCases() { return cases_; }
         };

         class SelectSeriesEvent : public Event {
         private:
             SeriesPayload series_;
         public:
             explicit SelectSeriesEvent(const std::string& name, SeriesPayload& series) : series_(series), Event(std::string("dataset/") + name) {}

             SeriesPayload& getSeries() { return series_; }
         };
#define SELECTCASEEVENT_PTRCAST(image) (reinterpret_cast<SeriesPayload*>((image)))

        /**
         * @brief The Explore class allows to explore and discover the content of a certain folder
         *
         * This class mainly interfaces with the python script load_dicom.py and will explore the
         * given folder for DICOM images and classify by Patient IDs.
         *
         */
         class Explore {
         public:
             enum status {EXPLORE_SLEEPING, EXPLORE_WORKING, EXPLORE_SUCCESS, EXPLORE_CANCELLED, EXPLORE_PARTIAL_SUCCESS, EXPLORE_ERROR};
         private:
             std::string path_;
             std::vector<PatientNode> cases_; // Tree representation of the cases discovered in the folder

             EventQueue& event_queue_;
             status status_;
             JobReference jobRef_;
         public:

             /**
              * Initializes the exploration path
              * @param path to a folder to explore
              *
              * The path can not be changed later
              */
             explicit Explore() : event_queue_(EventQueue::getInstance()), status_(EXPLORE_SLEEPING) {}

             /**
              * @brief Opens the Python interpreter and explores all the readable DICOMs present in
              * the folder.
              *
              * The function will push some log events about the progression of the exploration.
              * The function is launched as a Job on the JobScheduler under the name JOB_EXPLORE_NAME
              *
              * @returns SUCCESS if no error have been encountered, PARTIAL SUCCESS when encountering
              * some errors but still DICOMS have been found and ERROR if the exploration failed
              */
             void findDicoms(const std::string& path);

             /**
              * @return a flat representation of all the cases the have been found
              */
             std::vector<PatientNode>& getCases() { return cases_; }

             /**
              * @return the status of the exploration of the folder
              */
             status& getStatus() { return status_; }

             /**
              * Whenever findDicoms is called, a new job is launched.
              * @return the job reference created in the findDicoms function
              */
             JobReference& getJobReference() {return jobRef_; }
         };
     }
 }
