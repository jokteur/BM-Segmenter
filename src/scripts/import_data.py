import os
import numpy as np
import toml

from .workspace import get_dirs, create_series_dir
from .load_dicom import load_scan_from_dicom


def import_dicom(path: str, root_dir: str, id: str, num: int, replace=False):
    """Imports dicom as a numpy matrix in the project folder

    Arguments
    ---------
    path : str
        path to the dicom file
    root_dir : str
        path to the root of the project
    id : str
        name of the case (should be a number followed by an underscore)
    replace : bool
        if True, then even if the file exists, if is replaced

    Returns
    -------
    True if the saved successfully, False if the file already exists
    """

    id_dir = create_series_dir(root_dir, id)

    filename = os.path.join(id_dir[0], str(num))
    if not os.path.isfile(filename + ".npz") or replace:
        pixels, spacing, thickness, location = load_scan_from_dicom(path)
        np.savez_compressed(filename, pixels, np.array(spacing), np.array([thickness, location]))
        return True
    else:
        return False


def add_to_dataset(paths: list, group_name: str, root_dir: str):
    """Adds the list of paths to the dataset toml"""
    filename = os.path.join(root_dir, "dataset.toml")

    data = {"files": set(), "groups": {group_name: set()}}
    if os.path.isfile(filename):
        data = toml.load(os.path.join(root_dir, "dataset.toml"))

        if not "files" in data:
            data["files"] = set()

        if not "groups" in data:
            data["groups"] = {}

        if not group_name in data["groups"]:
            data["groups"][group_name] = set()

        data["groups"][group_name] = set(data["groups"][group_name])
        data["files"] = set(data["files"])

        for path in paths:
            path = os.path.basename(path)
            data["files"].add(path)
            data["groups"][group_name].add(path)

    toml.dump(data, open(filename, "w"))
