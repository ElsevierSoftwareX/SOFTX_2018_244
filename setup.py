import os
import re
import sys
import platform
import subprocess

from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources = [])
        self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError('CMake must be installed to build the following extensions: ' +
                               ', '.join(e.name for e in self.extensions))

        if platform.system() == 'Windows':
            cmake_version = LooseVersion(re.search(r'version\s*([\d.]+)', out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError('CMake >= 3.1.0 is required on Windows')

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE',
                      '-DPYTHON_EXECUTABLE=' + sys.executable,
                      '-DBUILD_EXAMPLES=OFF']

        cfg = 'Debug' if self.debug else 'Release'
        cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]

        print('building CMake extension in %s configuration' % cfg)

        if platform.system() == 'Windows':
            cmake_args += ['-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=' + extdir]
            cmake_args += ['-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=' + extdir]
            if sys.maxsize > 2**32:
                cmake_args += ['-A', 'x64']
            build_args = ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir]
            build_args = ['--', '-j4']

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\'{}\''.format(env.get('CXXFLAGS', ''), self.distribution.get_version())

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd = self.build_temp, env = env)
        subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd = self.build_temp)

def cleanhtml(raw_html):
    cleanr = re.compile('<.*?>')
    cleantext = re.sub(cleanr, '', raw_html)
    return cleantext

def read(fname):
    dname = os.path.dirname(__file__)
    fname = os.path.join(dname, fname)

    with open(fname) as f:
        contents = f.read()
        sanatized = cleanhtml(contents)

        try:
            from m2r import M2R
            m2r = M2R()
            return m2r(sanatized)
        except:
            return sanatized

setup(
    name = 'dpsim',
    version = '1.0.0',
    author = 'Markus Mirz, Steffen Vogel',
    author_email = 'acs-software@eonerc.rwth-aachen.de',
    description = 'A real-time simulator that operates in the dynamic phasor as well as electromagentic transient domain',
    license = 'GPL-3.0',
    keywords = 'simulation power system real-time',
    url = 'https://www.fein-aachen.org/projects/dpsim/',
    packages = find_packages('Source/Python'),
    package_dir = {
        'dpsim': 'Source/Python/dpsim',
        'villas': 'Source/Python/villas'
    },
    long_description = read('README.md'),
    classifiers = [
        'Development Status :: 4 - Beta',
        'Topic :: Scientific/Engineering',
        'License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)',
        'Operating System :: MacOS :: MacOS X',
        'Operating System :: Microsoft :: Windows',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: Implementation :: CPython'
    ],
    install_requires = [
        'villas-dataprocessing>=0.2',
        'progressbar2',
        'ipywidgets'
    ],
    setup_requires = [
        'm2r',
        'pytest-runner',
        'wheel'
    ],
    tests_require=[
        'pytest',
        'pyyaml'
    ],
    ext_modules = [
        CMakeExtension('_dpsim')
    ],
    cmdclass = {
        'build_ext': CMakeBuild
    },
    zip_safe = False
)
