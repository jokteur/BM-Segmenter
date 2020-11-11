"""Module for handling DICOM files.
"""
import numpy as np
import pydicom
import os
from os.path import dirname, join
from collections import OrderedDict

from pydicom.filereader import read_dicomdir
from pydicom import fileset

from .util import special_sort


class DicomFolder:
    """
    Class that stores all the information about DICOM in a particular dicom folder
    """

    def __init__(self, root_folder):
        """Sets the root folder of the class"""
        self.root = root_folder
        self.dicoms = OrderedDict()

    def add(
        self,
        path,
        patientID,
        studyDate,
        studyTime,
        studyDescription,
        seriesNumber,
        modality,
        instanceNumber,
    ):
        """
        Adds to the dir structure a case (i.e. 1 image) from a certain Patient, modality, etc.
        """
        if patientID not in self.dicoms:
            self.dicoms[patientID] = {}

        path = str(path)
        patientID = str(patientID)
        studyDate = str(studyDate)
        studyTime = str(studyTime)
        studyDescription = str(studyDescription)
        seriesNumber = str(seriesNumber)
        modality = str(modality)
        instanceNumber = str(instanceNumber)

        study_key = studyDate, studyTime, studyDescription
        series_key = seriesNumber, modality

        if study_key not in self.dicoms[patientID]:
            self.dicoms[patientID][study_key] = {}

        if series_key not in self.dicoms[patientID][study_key]:
            self.dicoms[patientID][study_key][series_key] = []

        self.dicoms[patientID][study_key][series_key].append(
            {"path": path, "instanceNumber": instanceNumber}
        )

    def get_cases_list(self):
        """Returns the list of cases found in the folder (ordered)."""
        cases = []
        arg_sort, patientIDs = special_sort(self.dicoms.keys())
        patients = list(self.dicoms.values())
        for i, corrected in enumerate(arg_sort):
            cases.append((patientIDs[i], patients[corrected]))
        return cases

    def __str__(self):
        """Returns a string representation of the DICOM present in the root folder."""
        str_rep = []
        for patientID, patients in self.dicoms.items():
            str_rep.append(f"PatientID: {patientID}")
            for (studyDate, studyTime, studyDescription), study in patients.items():
                str_rep.append(
                    " " * 2 + f"Study date: {studyDate or 'N.A.'} at {studyTime or 'N.A.'},"
                    f" Study description: {studyDescription or 'N.A.'}"
                )
                for (seriesNumber, modality), series in study.items():
                    str_rep.append(
                        " " * 4 + f"Series number: {seriesNumber or 'N.A.'},"
                        f" Modality: {modality or 'N.A'}"
                    )
                    for image in series:
                        number = image["instanceNumber"]
                        str_rep.append(" " * 6 + f"Image number: {number or 'N.A'}")
        return "\n".join(str_rep)


class DiscoverDicoms:
    """
    Class for discovering DICOMs in a certain folder.

    This class is a generator, meaning that it can be used with next to get results
    """

    def __init__(self, path):
        """Initializes the generator class

        Arguments
        ---------
        path : str
            path to a folder that will explored by the class
        """
        self.path = path
        self.explore = True
        self.exclude_pattern = ""

        if not os.path.isdir(path):
            self.explore = False

        self.walk_iter = os.walk(path)

        self.data = DicomFolder(path)

    def __iter__(self):
        return self

    def __next__(self):
        """
        Generator that searches for DICOM in the defined path.

        Returns
        -------
        str
            folder that is currently being search
        bool
            True if DICOM(s) were found in the folder
        """
        dirpath, dirnames, filenames = next(self.walk_iter)

        if self.exclude_pattern:
            while self.exclude_pattern in dirpath:
                dirpath, dirnames, filenames = next(self.walk_iter)
            self.exclude_pattern = ""

        found_dicom = False
        error_list = []
        for file in filenames:
            filepath = os.path.join(dirpath, file)
            ds = None
            try:
                ds = pydicom.dcmread(filepath)
            except pydicom.errors.InvalidDicomError:
                continue
            except AttributeError:
                error_list.append(f"Attribute error when reading {filepath}")
                self.exclude_pattern = dirpath

            if not ds:
                continue

            # Check if dicom is a fileset (DICOMDIR)
            # http://dicom.nema.org/medical/dicom/current/output/chtml/part10/chapter_8.html
            fs = None
            try:
                fs = fileset.FileSet(ds)
            except ValueError:
                pass

            if fs:
                self.exclude_pattern = dirpath
                for instance in fs:
                    self.data.add(
                        instance.path,
                        instance.PatientID,
                        instance.StudyDate,
                        instance.StudyTime,
                        instance.StudyDescription,
                        instance.SeriesNumber,
                        instance.Modality,
                        instance.InstanceNumber,
                    )
                found_dicom = True
            elif hasattr(ds, "PixelData"):
                patientID = ""
                modality = ""
                studyDate = ""
                studyTime = ""
                studyDescription = ""
                seriesNumber = ""
                instanceNumber = ""
                if hasattr(ds, "PatientID"):
                    patientID = ds.PatientID
                if hasattr(ds, "Modality"):
                    modality = ds.Modality
                if hasattr(ds, "StudyDate"):
                    studyDate = ds.StudyDate
                if hasattr(ds, "StudyTime"):
                    studyTime = ds.StudyTime
                if hasattr(ds, "StudyDescription"):
                    studyDescription = ds.StudyDescription
                if hasattr(ds, "SeriesNumber"):
                    seriesNumber = ds.SeriesNumber
                if hasattr(ds, "InstanceNumber"):
                    instanceNumber = ds.InstanceNumber
                    self.data.add(
                        filepath,
                        patientID,
                        studyDate,
                        studyTime,
                        studyDescription,
                        seriesNumber,
                        modality,
                        instanceNumber,
                    )
                found_dicom = True

        return dirpath[len(self.path) :], found_dicom, "\n".join(error_list)


def load_scan_from_dicom(path):
    """
    Loads the dicomdir and reads the image depending on the arguments

    Returns
    -------
    numpy array : hu reading of the image
    bool : if no image has been read or found, returns false
    """

    ds = None
    try:
        ds = pydicom.dcmread(path)
    except pydicom.errors.InvalidDicomError:
        return False, "Invalid DICOM file."
    except FileNotFoundError:
        return False, "Could not find the DICOM file."
    except AttributeError:
        return False, "Attribute error when reading the DICOM file."

    slice_thickness = 1
    slice_location = 1
    pixel_spacing = (1, 1)
    try:
        ds.pixel_array
        pixel_spacing = (float(ds["PixelSpacing"][0]), float(ds["PixelSpacing"][0]))
    except:
        return (
            False,
            "Problem when opening the image in the DICOM file.\n The image should contain pixels and PixelSpacing.",
        )

    if hasattr(ds, "SliceLocation"):
        slice_location = float(ds["SliceLocation"].value)
    if hasattr(ds, "SliceThickness"):
        slice_thickness = float(ds["SliceThickness"].value)

    return (
        get_pixels_hu(ds),
        pixel_spacing,
        slice_thickness,
        slice_location,
    )


def get_pixels_hu(scan):
    """
    Converts the data of scan into standard Hounsfield units (HU)
    """
    # Convert to int16 (from sometimes int16),
    # should be possible as values should always be low enough (<32k)
    pixel_array = scan.pixel_array
    image = np.array(pixel_array, dtype=np.int16)

    # Set outside-of-scan pixels to 1
    # The intercept is usually -1024, so air is approximately 0
    image[image == -2000] = 0

    # Convert to Hounsfield units (HU)
    intercept = scan.RescaleIntercept
    slope = scan.RescaleSlope

    if slope != 1:
        image = slope * image.astype(np.float64)
        image = image.astype(np.int16)

    image += np.int16(intercept)

    return image  # np.array(image, dtype=np.int16)
