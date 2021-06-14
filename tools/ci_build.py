#!/usr/bin/env python3
# This file is part of SoFCheck
#
# Copyright (c) 2021 Alexander Kernozhitsky and SoFCheck contributors
#
# SoFCheck is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SoFCheck is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

import os
import json
import argparse
import subprocess
import warnings
import shlex
import sys


def pexec(cmd, **kwargs):
    cmd_str = ' '.join([shlex.quote(x) for x in cmd])
    sys.stderr.write('$ ' + cmd_str + '\n')
    subprocess.check_call(cmd, **kwargs)


def bool_to_onoff(val):
    return 'ON' if val else 'OFF'


def list_or_str(val):
    return [val] if isinstance(val, str) else val


def gen_cmake_command(config, storage, src_path):
    result = [
        'cmake', src_path,
        '-DCMAKE_BUILD_TYPE=' + config['cmake-build-type'],
        '-DCMAKE_INSTALL_PREFIX=' + storage['path']['prefix'],
        '-DCMAKE_C_COMPILER=' + ';'.join(list_or_str(storage['cmd']['cc'])),
        '-DCMAKE_CXX_COMPILER=' + ';'.join(list_or_str(storage['cmd']['cxx'])),
    ]
    if 'cmake-generator' in config:
        result += ['-G', config['cmake-generator']]
    return result


def cmake_build(config, storage, build_dir):
    pexec(['cmake', '--build', build_dir, '-j', '--config',
           config['cmake-build-type']])


def configure(config, storage, args):
    cc_names = {
        'gcc': 'gcc',
        'clang': 'clang'
    }
    cxx_names = {
        'gcc': 'g++',
        'clang': 'clang++'
    }
    linux_pkg_names = {
        'gcc': 'g++',
        'clang': 'clang'
    }
    macos_pkg_names = {
        'gcc': 'gcc',
        'clang': 'llvm'
    }

    compiler = list(config['compiler'].split('-'))
    comp_version = '' if len(compiler) == 1 else compiler[1]
    compiler = compiler[0]
    comp_version_suf = '' if comp_version == '' else '-' + comp_version
    home = os.environ['HOME']

    storage['path'] = {}
    storage['path']['dependencies'] = os.path.join(home, 'deps')
    storage['path']['prefix'] = os.path.join(home, 'prefix')
    if not os.path.isdir('.git'):
        raise RuntimeError('You must be in repository root')
    storage['path']['src-dir'] = os.getcwd()
    storage['path']['env'] = []

    storage['build-libs'] = {}
    storage['build-libs']['benchmark'] = config['os'] in {'windows', 'macos'}
    storage['build-libs']['googletest'] = True

    storage['pkg'] = {}
    storage['cmd'] = {}

    if config['os'] == 'windows':
        if comp_version != '':
            warnings.warn('Versioned compilers are not supported on Windows')
        storage['cmd']['cc'] = cc_names[compiler]
        storage['cmd']['cxx'] = cxx_names[compiler]
        # Clang-tidy is disabled on Windows, as it cannot read rsp files now
        storage['cmd']['clang-tidy'] = None
    elif config['os'] == 'linux' or config['os'] == 'ubuntu':
        storage['cmd']['cc'] = cc_names[compiler] + comp_version_suf
        storage['cmd']['cxx'] = cxx_names[compiler] + comp_version_suf
        storage['cmd']['clang-tidy'] = 'clang-tidy-11'
        storage['pkg']['compiler'] = \
            linux_pkg_names[compiler] + comp_version_suf
        storage['pkg']['clang-tidy'] = 'clang-tidy-11'
    elif config['os'] == 'macos':
        storage['cmd']['cc'] = cc_names[compiler] + comp_version_suf
        storage['cmd']['cxx'] = cxx_names[compiler] + comp_version_suf
        storage['pkg']['compiler'] = \
            macos_pkg_names[compiler] + comp_version_suf.replace('-', '@')

        llvm_ver = '11'
        if compiler == 'clang' and comp_version != '':
            llvm_ver = comp_version

        clang_path = '/usr/local/opt/llvm@' + llvm_ver + '/bin'
        storage['cmd']['clang-tidy'] = os.path.join(clang_path, 'clang-tidy')
        storage['pkg']['clang-tidy'] = 'llvm@' + llvm_ver

        storage['path']['env'] += [clang_path]

        if compiler == 'clang':
            storage['cmd']['cc'] = \
                os.path.join(clang_path, cc_names[compiler] + comp_version_suf)
            storage['cmd']['cxx'] = \
                os.path.join(clang_path, cxx_names[compiler])
    else:
        raise RuntimeError('OS not supported')

    os.mkdir(storage['path']['dependencies'])
    os.mkdir(storage['path']['prefix'])


def install_deps(config, storage, args):
    if config['os'] == 'windows':
        pexec(['choco', 'install', 'mingw', '--version=8.1.0'])
        pexec(['choco', 'install', 'cmake'])
    elif config['os'] == 'linux' or config['os'] == 'ubuntu':
        pexec(['sudo', 'apt', 'install', storage['pkg']['compiler'],
               storage['pkg']['clang-tidy'], 'cmake', 'libbenchmark-dev',
               'libjsoncpp-dev'])
    elif config['os'] == 'macos':
        pexec(['brew', 'install', storage['pkg']['compiler'],
               storage['pkg']['clang-tidy'], 'cmake', 'jsoncpp'])
    else:
        raise RuntimeError('OS not supported')


def build_deps(config, storage, args):
    libs = {
        'googletest': {
            'repo': 'https://github.com/google/googletest/',
            'branch': 'release-1.10.0',
            'flags': []
        },
        'benchmark': {
            'repo': 'https://github.com/google/benchmark/',
            'branch': 'v1.5.5',
            'flags': ['-DBENCHMARK_ENABLE_TESTING=OFF']
        }
    }

    for lib in libs:
        if not storage['build-libs'][lib]:
            continue
        lib_info = libs[lib]
        pexec(['git', 'clone', '--branch', lib_info['branch'], lib_info['repo']],
              cwd=storage['path']['dependencies'])
        build_dir = os.path.join(storage['path']['dependencies'], lib, 'build')
        os.mkdir(build_dir)
        cmake_args = \
            gen_cmake_command(config, storage, '..') + lib_info['flags']
        pexec(cmake_args, cwd=build_dir)
        cmake_build(config, storage, build_dir)
        pexec(['cmake', '--install', '.'], cwd=build_dir)


def build(config, storage, args):
    diagnostic = args.diagnostic

    build_dir = 'build-dgn' if diagnostic else 'build'
    build_dir = os.path.join(storage['path']['src-dir'], build_dir)
    os.mkdir(build_dir)

    cmake_args = gen_cmake_command(config, storage, '..')
    cmake_args += [
        '-DUSE_NO_EXCEPTIONS=ON',
        '-DUSE_BMI1=' + bool_to_onoff(config['bmi1']),
        '-DUSE_BMI2=' + bool_to_onoff(config['bmi2']),
        '-DUSE_BUILTIN_JSONCPP=' + bool_to_onoff(config['builtin-jsoncpp'])
    ]
    if diagnostic:
        cmake_args += ['-DUSE_SEARCH_DIAGNOSTICS=ON']
    else:
        if 'clang-tidy' in storage['cmd'] and \
                storage['cmd']['clang-tidy'] is not None:
            clang_tidy = list_or_str(storage['cmd']['clang-tidy'])
            clang_tidy += ['--warnings-as-errors=*']
            cmake_args += ['-DCMAKE_CXX_CLANG_TIDY=' + ';'.join(clang_tidy)]

    pexec(cmake_args, cwd=build_dir)
    cmake_build(config, storage, build_dir)


def test(config, storage, args):
    diagnostic = args.diagnostic

    build_dir = 'build-dgn' if diagnostic else 'build'
    build_dir = os.path.join(storage['path']['src-dir'], build_dir)

    ctest_args = ['ctest']
    if diagnostic:
        ctest_args += ['-R', '^smoke$']
    pexec(ctest_args, cwd=build_dir)


parser = argparse.ArgumentParser(description='Tool to build the program in CI')
parser.add_argument(
    '-c', '--config', help='configuration file',
    action='store', type=argparse.FileType('r'), required=True)
parser.add_argument(
    '-s', '--storage', help='storage file',
    action='store', type=str, required=True)
subparsers = parser.add_subparsers(help='command to execute', dest='cmd')
subparsers.add_parser('configure', help='populate storage with configuration')
subparsers.add_parser('install', help='install dependencies')
subparsers.add_parser('build-dep', help='build dependencies')
p_build = subparsers.add_parser('build', help='build project')
p_build.add_argument('-d', '--diagnostic',
                     action='store_true', help='build in diagnostic mode')
p_test = subparsers.add_parser('test', help='test project')
p_test.add_argument('-d', '--diagnostic',
                     action='store_true', help='build in diagnostic mode')

args = parser.parse_args()

config = json.load(args.config)
try:
    storage = json.load(open(args.storage, 'r'))
except FileNotFoundError:
    storage = {}

if 'path' in storage and 'env' in storage['path']:
    os.environ['PATH'] = os.environ['PATH'] + os.pathsep + \
        os.pathsep.join(storage['path']['env'])

funcs = {
    'configure': configure,
    'install': install_deps,
    'build-dep': build_deps,
    'build': build,
    'test': test
}
funcs[args.cmd](config, storage, args)

open(args.storage, 'w').write(json.dumps(storage, indent=2))
