import toml
import os

from .workspace import get_dirs, get_root


def load_dataset(path: str):
    root_dir = get_root(path)
    """Loads dataset into project."""
    filename = os.path.join(root_dir, "dataset.toml")
    result = {}

    dirs = get_dirs(root_dir)[1]
    if os.path.isfile(filename):
        data = toml.load(os.path.join(root_dir, "dataset.toml"))

        for id in data["files"]:
            dicom_dir = os.path.join(dirs["dicoms"], id)
            groups = [name for name, files in data["groups"].items() if id in set(files)]
            files = [os.path.join(dicom_dir, name) for name in os.listdir(dicom_dir)]

            result[id] = {"groups": groups, "files": files}

    return result
