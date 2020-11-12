"""Module for handling the workspace
"""
import os

from .util import make_safe_filename


def setup_workspace(path: str, name: str, extension):
    if os.path.isdir(path):
        name = make_safe_filename(name)
        try:
            os.mkdir(os.path.join(path, name))
            os.mkdir(os.path.join(path, name, "data"))
            os.mkdir(os.path.join(path, name, "data", "dicoms"))
            os.mkdir(os.path.join(path, name, "data", "masks"))
            os.mkdir(os.path.join(path, name, "tmp"))
            os.mkdir(os.path.join(path, name, "tmp", "train"))
            os.mkdir(os.path.join(path, name, "tmp", "train", "x"))
            os.mkdir(os.path.join(path, name, "tmp", "train", "y"))
            os.mkdir(os.path.join(path, name, "models"))
        except OSError as e:
            return False, f"Could not create directories: {e.what()}"

        return True, os.path.join(path, name, f"{name}.{extension}")
    else:
        return False, "Path is not a directory"