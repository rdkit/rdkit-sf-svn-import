import os
import re
from datetime import datetime
from setuptools import setup

pkg_version = ''
path_to_pkg_version = os.path.realpath(__file__)
have_src_dir = False
src_dir = None
if (os.path.isfile(path_to_pkg_version)):
    src_dir = os.path.dirname(path_to_pkg_version)
    have_src_dir = os.path.isdir(src_dir)
if (not have_src_dir):
    raise OSError('Could not find SRC_DIR, got: ' + str(src_dir))
# parse Code/cmake/Modules/RDKitVersion.cmake
rdkitVersion_cmake_module_path = os.path.join(src_dir,
    'Code', 'cmake', 'Modules', 'RDKitVersion.cmake')

with open(rdkitVersion_cmake_module_path, 'rt') as hnd:
    # vars we want to read
    var_set = set(['RDKit_Year', 'RDKit_Month', 'RDKit_Revision',
        'RDKit_ABI', 'RDKit_RELEASENAME'])
    var_dict = {}
    line = hnd.readline()
    while (line):
        # is this an uncommented set command?
        m = re.match('^\s*set\s*\((\w+)\s*\"(.*)\"\s*\)',
            line, re.IGNORECASE)
        # if it is
        if (m is not None):
            # extract the var name
            var_name = m.group(1)
            if (var_name in var_set):
                # if the var name is in the vars we want to read
                var_value = m.group(2)
                keepLooping = True
                while (keepLooping):
                    # recursively replace variables we already found
                    m = re.match('^.*\${(\w+)}', var_value)
                    keepLooping = (m is not None)
                    if (keepLooping):
                        v = var_dict.get(m.group(1))
                        # if the variable is not defined, remove
                        # the preceding dot
                        eat_dot = 0
                        if (v is None):
                            v = ''
                            eat_dot = 1
                        # replace variable name with its value
                        s = m.start(1) - (2 + eat_dot)
                        e = m.end(1) + 1
                        var_value = var_value[:s] + v + var_value[e:]
                # assign variable value and keep parsing
                var_dict[var_name] = var_value
        line = hnd.readline()
d = datetime.today().strftime('%Y%m%d')
rdkitVersion = var_dict.get('RDKit_RELEASENAME')
if (rdkitVersion is not None):
    if rdkitVersion.endswith('.dev1'):
        pkg_version = rdkitVersion[:-1] + d
    else:
        pkg_version = rdkitVersion
else:
    # if extracting rdkitVersion somehow failed, use the date
        pkg_version = d

setup(rdkitVersion = pkg_version)
