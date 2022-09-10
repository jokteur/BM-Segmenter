
try:
    import traceback
    import pathlib
    import python.scripts.project_edition.project
    def predict(project_path, mask_name):
        try:
            project_to_edit = python.scripts.project_edition.project.Project(pathlib.Path(project_path))
            project_to_edit.add_ml_predictions(mask_name)
        except Exception as e:
            print("hoho")
            print(traceback.format_exc())
            print(e)
except Exception as e:
    print("haha")
    print(e)
    print(traceback.format_exc())

