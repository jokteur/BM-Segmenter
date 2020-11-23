"""Module for handling the workspace
"""
import os

from .util import make_safe_filename


def get_root(save_file: str):
    return os.path.dirname(save_file)


def _make_folders(root: str):
    try:
        os.mkdir(os.path.join(root, "data"))
    except FileExistsError:
        pass
    try:
        os.mkdir(os.path.join(root, "data", "dicoms"))
    except FileExistsError:
        pass
    try:
        os.mkdir(os.path.join(root, "data", "masks"))
    except FileExistsError:
        pass
    try:
        os.mkdir(os.path.join(root, "tmp"))
    except FileExistsError:
        pass
    try:
        os.mkdir(os.path.join(root, "tmp", "train"))
    except FileExistsError:
        pass
    try:
        os.mkdir(os.path.join(root, "tmp", "train", "x"))
    except FileExistsError:
        pass
    try:
        os.mkdir(os.path.join(root, "tmp", "train", "y"))
    except FileExistsError:
        pass
    try:
        os.mkdir(os.path.join(root, "models"))
    except FileExistsError:
        pass


def setup_workspace(path: str, name: str, extension):
    if os.path.isdir(path):
        name = make_safe_filename(name)
        try:
            os.mkdir(os.path.join(path, name))
            _make_folders(os.path.join(path, name))
        except FileExistsError:
            pass
        except OSError as e:
            return False, f"Could not create directories: {e.what()}"

        return True, os.path.join(path, name, f"{name}.{extension}")
    else:
        return False, "Path is not a directory"


def get_dirs(path: str):
    """
    Fetches and returns the directory structure found at the path
    """

    if os.path.isdir(path):
        dirs = {}

        if os.path.isdir(os.path.join(path, "data", "dicoms")):
            dirs["dicoms"] = os.path.join(path, "data", "dicoms")
        if os.path.isdir(os.path.join(path, "data", "masks")):
            dirs["masks"] = os.path.join(path, "data", "masks")
        if os.path.isdir(os.path.join(path, "models")):
            dirs["models"] = os.path.join(path, "models")
        if os.path.isdir(os.path.join(path, "data", "train", "x")):
            dirs["train_x"] = os.path.join(path, "data", "train", "x")
        if os.path.isdir(os.path.join(path, "data", "train", "y")):
            dirs["train_y"] = os.path.join(path, "data", "train", "y")
        return True, dirs
    else:
        return False, "Path is not a directory"


def create_series_dir(path: str, id: str):
    """Creates a director6'pè^¨$ for a Dicom Series in the data/dicom folder."""
    _make_folders(path)
    try:
        os.mkdir(os.path.join(path, "data", "dicoms", id))
    except FileExistsError:
        return os.path.join(path, "data", "dicoms", id), False
    return os.path.join(path, "data", "dicoms", id), True