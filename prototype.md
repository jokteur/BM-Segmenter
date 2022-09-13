# BM-Segmenter prototype
## Development environment setup
Here are step-by-step instructions to set up a development environment for this software, to compile it and to distribute it using Clion on Windows.

### Create a conda environment with python 3.10 and all needed libraries
If you don't already have a conda installation, google miniconda and install it. Open a conda terminal via Windows search > `Anacona Prompt`.

As the time of writing, the versions of the libraries in the below commands are the most recent and have been tested with this software. Future library versions may work but if you want to make sure to avoid any compatibility issue, use those versions.

Run the following commands :
```
conda create -n bmsegmenter -c conda-forge python=3.10 cudatoolkit=11.2 cudnn=8.1.0
conda activate bmsegmenter
pip install scikit-learn==1.1.1 matplotlib==3.5.2 pandas==1.4.3 seaborn==0.11.2 numpy==1.23.3 pydicom==2.3.0 pylibjpeg==1.4.0 toml==0.10.2 tensorflow==2.9.1 tensorflow-addons==0.17.1 keras-tuner==1.1.3 scikit-image==0.19.3
 ```

Most of the libraries installed with pip are available in conda, but installing them with conda poses problems to make the software portable.

### Install MSCV (Microsoft's C++ compiler)
You can get MSVC by installing Visual Studio. To get the compiler, check the "Desktop development with C++" option during the installation.

It is important to install the english version of MSCV. Other languages cause issues with Ninja.

### Install OpenCV
Download OpenCV 4.6.0 for Windows and extract it somewhre, for example in `C:\lib\opencv`

Add `<path to your opencv directory>\build` to your PATH (Windows search > environment variables > environment variables > Path > Edit > New) and then restart your computer

### Set up the project directory
Git clone this repo with its submodules :

```git clone https://github.com/jokteur/BM-Segmenter.git --recurse-submodules```

Create a copy of the file `CMakePresets_template.json` named `CMakePresets.json` at the root of the project. Edit it to replace `<<PATH TO YOUR CONDA ENVIRONMENT>>` with the appropriate path. It should be something like `C:/users/<your username>/miniconda3/envs/bmsegmenter`
### Set up Clion
- Open the project in Clion
- Open the `Toolchains` settings (CTRL + SHIFT + A > toolchains) and select Visual Studio
  - Change the architecture to amd64
  - Set Visual Studio as default by moving it to the beginning of the list using the triangle arrow 
- Open the `CMake` settings (CTRL + SHIFT + A > cmake)
  - Disable the first profile by unchecking its `Enable profile` checkbox
  - Enable the debug and release profiles
- Open src > python > CMakeLists.txt and uncomment the bloc of code that begins with `install(` at the middle of the file.
  - This code lets the python libraries and scripts be copied in the software directory when it is installed. To make future installations faster, you can comment back those lines after the first install if you don't modify the python files of the software.
- Click on `CMake` at the bottom and reload the CMake project
### Optional : set up the machine learning model
This step will enable the machine learning prediction feature in the software. You will need a Tensorflow model able to do such predictions. We do not provide our model here because it may give access to sensitive data, therefore if you don't have access to the model used at the CHUV you will need to train one yourself. See [this project](https://github.com/damienmaier/l3-segmentation) for more information.

- Copy your Tensorflow model directory to `src/scripts/project_edition/mlsegmentation/model`
- In `src/scripts/project_edition/mlsegmentation`, make a copy of the file `config_template.py` named `config.py`
### Build and install
- Click on Build > Install
- The software is compiled and installed in `out/install/debug` or `out/install/release`
- Copy the file `opencv_world460.dll` (if compiled in release) or `opencv_world460d.dll` (if compiled in debug) from `<your opencv directory>/build/x64/vc15/bin` to the direcotry of the main executable of the program
- Copy all the files in `<your conda environment's directory>/Library/bin` to the directory of the main executable of the program
- You can create a batch file at the root of the installation directory to run the software without having to search for the executable :
```
cd 0.1
BM-Segmenter.exe
```
### Debugging
Several assert statements from the ImGui library will fail when using the program. This is not a problem when the program is compiled in release mode because in this case the asserts are ignored.

If you run the program in debug mode, you will need to comment out the asserts that fail.
