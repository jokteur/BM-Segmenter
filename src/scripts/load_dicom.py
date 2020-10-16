import numpy as np
import pydicom
from pydicom.filereader import read_dicomdir
from os.path import dirname, join


def load_scan_from_dicom(path):
    """
    Loads the dicomdir and reads the image from the first series of the first study
    Returns scan, image_dir
    """
    dicom_dir = read_dicomdir(path)
    base_dir = dirname(path)

    # Get directly to the series of images, considering there is only one patient and one study in the dicomdir
    series = dicom_dir.patient_records[0].children[0].children[0]

    # Take the first image of the series
    image_name = series.children[0]
    image_path = join(base_dir, *image_name.ReferencedFileID)

    # For reading the ROI
    return pydicom.dcmread(image_path), dirname(image_path)


def get_pixels_hu(scan):
    """
    Converts the data of scan into standard Hounsfield units (HU)
    """
    # Convert to int16 (from sometimes int16),
    # should be possible as values should always be low enough (<32k)
    image = scan.pixel_array
    image = image.astype(np.int16)

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

    return np.array(image, dtype=np.int16)
