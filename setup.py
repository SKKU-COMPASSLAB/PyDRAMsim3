import os
import logging
import subprocess
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from Cython.Build import cythonize

PROJECT_ROOT_DIR    = os.path.abspath(os.path.dirname(__file__))
DRAMSIM_SRC_DIR     = os.path.join(PROJECT_ROOT_DIR, "src")
PYDRAMSIM_ROOT_DIR  = os.path.join(PROJECT_ROOT_DIR, "pydramsim3")
PYDRAMSIM_LIB_DIR   = os.path.join(PYDRAMSIM_ROOT_DIR, "lib")

def build_dramsim3():
    logging.info("Building DRAMSim3...")
    
    cmake_args = ["cmake", "-S", PROJECT_ROOT_DIR, "-B", PYDRAMSIM_ROOT_DIR]
    build_args = ["cmake", "--build", PYDRAMSIM_ROOT_DIR]
    
    logging.info("Running: %s", " ".join(cmake_args))
    subprocess.check_call(cmake_args)
    
    logging.info("Running: %s", " ".join(cmake_args))
    subprocess.check_call(build_args)
    
class BuildDRAMSim3Extension(build_ext):
    def run(self):
        build_dramsim3()
        
        return super().run()

ext = Extension(
    "pydramsim3.pydramsim3",
    sources=["pydramsim3/pydramsim3.pyx"],
    include_dirs=[DRAMSIM_SRC_DIR],
    language="c",
    libraries=["dramsim3"],
    library_dirs=[PYDRAMSIM_LIB_DIR],
    extra_compile_args=["-O3"],
)

setup(
    name="pydramsim3",
    version="0.1",
    description='Python extension for the DRAMSim3',
    author='Seongwook Kim',
    author_email='su8939@skku.edu',
    packages=["pydramsim3"],
    package_data={"pydramsim3": ["pydramsim3.pyi", "py.typed"]},
    ext_modules=cythonize(
        [ext], 
        annotate=True, 
        compiler_directives={
            "annotation_typing": True,
        }, 
        language_level=3),
    include_package_data=True,
    zip_safe=False,
    cmdclass={"build_ext": BuildDRAMSim3Extension},
)
