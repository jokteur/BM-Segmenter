import os
import toml

from .workspace import get_dirs, get_root
from .util import make_safe_filename


def save_segmentation(project_file: str, name: str, description: str, filename: str):
    """Saves a segmentation to a file."""
    root = get_root(project_file)
    dirs = get_dirs(root)[1]

    save_path = ""
    if filename:
        save_path = filename
    else:
        save_path = os.path.join(dirs["models"], make_safe_filename(name) + ".seg")
        if os.path.isfile(save_path):
            raise Exception("Segmentation with near identical name already exists")

        try:
            os.mkdir(os.path.join(dirs["masks"], make_safe_filename(name)))
        except FileExistsError:
            pass

    data = {"name": name, "description": description, "stripped_name": make_safe_filename(name)}
    toml.dump(data, open(save_path, "w"))

    return save_path


def load_segmentations(root_dir: str):
    if not os.path.isdir(root_dir):
        raise Exception("Project path is not a directory")

    dirs = get_dirs(root_dir)[1]

    segmentations = []
    for path in os.listdir(dirs["models"]):
        path = os.path.join(dirs["models"], path)
        if os.path.isfile(path) and path.endswith(".seg"):
            data = toml.load(path)
            data["path"] = path
            data["ids"] = [
                name[:-4] for name in os.listdir(os.path.join(dirs["masks"], data["stripped_name"]))
            ]
            segmentations.append(data)

    print(segmentations)
    return segmentations