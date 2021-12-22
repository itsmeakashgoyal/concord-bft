#!/usr/bin/python
# -*- coding: utf-8 -*-

# Concord
#
# Copyright (c) 2018-2021 VMware, Inc. All Rights Reserved.
#
# This product is licensed to you under the Apache 2.0 license (the "License").
# You may not use this product except in compliance with the Apache 2.0 License.
#
# This product may include a number of subcomponents with separate copyright
# notices and license terms. Your use of these subcomponents is subject to the
# terms and conditions of the subcomponent's license, as noted in the LICENSE
# file.

from __future__ import print_function

'''Prepare a consolidated code coverage report for all apollo tests and gtests.
- Collate raw profiles into one indexed profile.
- Generate html reports for the given binaries.
Caution: The positional arguments to this script must be specified before any 
optional arguments, such as --restrict.
'''

import argparse
import os
import subprocess
import sys
import shutil


def change_permissions_recursive(path, mode):
    """
    Set permissions for all the files/folders under path
    :param path: Path to the folder
    :param mode: Permission
    """
    for root, dirs, files in os.walk(path, topdown=False):
        for directory in [os.path.join(root, d) for d in dirs]:
            os.chmod(directory, mode)
        for file in [os.path.join(root, f) for f in files]:
            os.chmod(file, mode)


def check_for_raw_profile_files(manifest_path):
    """
    Function is used to check for any .profraw files present.
    """
    manifest_file = open(manifest_path)
    profraw_ext = '.profraw'
    if(profraw_ext in manifest_file.read()):
        return True
    else:
        return False


def merge_raw_profiles(host_llvm_profdata, profile_data_dir, report_dir):
    """
    Function is used to generate the index profdata file after merging all profraw files.
    profdata file is used in the generation of code coverage report.
    """
    print('\n:: Merging raw profiles...', end='')
    sys.stdout.flush()
    manifest_path = os.path.join(report_dir, 'profiles.manifest')
    profdata_path = os.path.join(report_dir, 'Coverage.profdata')
    print("manifest_path: ", manifest_path)
    with open(manifest_path, 'w') as manifest:
        for (root, dirs, files) in os.walk(profile_data_dir):
            for file in files:
                if file.endswith('.profraw'):
                    manifest.write(os.path.join(root, file))
                    manifest.write('\n')
        manifest.close()
    if not check_for_raw_profile_files(manifest_path):
        print('\n:: No raw profiles present')
        sys.exit(1)

    merge_command = [
        host_llvm_profdata,
        'merge',
        '-sparse',
        '-f',
        manifest_path,
        '-o',
        profdata_path,
    ]
    merge_process = subprocess.run(merge_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                   universal_newlines=True)
    if merge_process.returncode != 0:
        print('Code coverage merge failed')
        print('  Stdout - {}'.format(merge_process.stdout.decode('utf-8')))
        print('  Stderr - {}'.format(merge_process.stderr.decode('utf-8')))
        sys.exit(1)
    os.remove(manifest_path)
    return profdata_path


def prepare_html_report(host_llvm_cov, profdata_path, report_dir, binaries,
                        restricted_dirs):
    """
    Function is used to generation of code coverage report.
    """
    print('\n:: Preparing html report for {0}...'.format(binaries), end='')
    sys.stdout.flush()
    objects = []
    for i, binary in enumerate(binaries):
        if i == 0:
            objects.append(binary)
        else:
            objects.extend(('-object', binary))
    index_page = os.path.join(report_dir, 'index.html')
    cov_command = [host_llvm_cov, 'show'] + objects + [
        '-format',
        'html',
        '-instr-profile',
        profdata_path,
        '-o',
        report_dir,
        '-show-line-counts-or-regions',
        '-Xdemangler',
        'c++filt',
        '-Xdemangler',
        '-n',
        '-project-title',
        'Concord-bft Apollo Code Coverage'
    ] + restricted_dirs
    cov_process = subprocess.run(cov_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                 universal_newlines=True)
    if cov_process.returncode != 0:
        print('\n::Code coverage report generation failed')
        print('Stdout - {}'.format(cov_process.stdout.decode('utf-8')))
        print('Stderr - {}'.format(cov_process.stderr.decode('utf-8')))
        sys.exit(1)

    with open(os.path.join(report_dir, 'summary.txt'), 'wb') as Summary:
        subprocess.check_call([host_llvm_cov, 'report'] + objects
                              + ['-instr-profile', profdata_path] + restricted_dirs, stdout=Summary)
    print('\n:: Merged Code Coverage Reports are in {}'.format(report_dir))
    print('\n:: Open browser on {}'.format(index_page))
    change_permissions_recursive(report_dir, 0o777)


def prepare_html_reports(host_llvm_cov, profdata_path, report_dir, binaries,
                         unified_report, restricted_dirs):
    if unified_report:
        print("\n::Unified report")
        prepare_html_report(host_llvm_cov, profdata_path, report_dir, binaries,
                            restricted_dirs)
    else:
        for binary in binaries:
            binary_report_dir = os.path.join(report_dir,
                                             os.path.basename(binary))
            prepare_html_report(host_llvm_cov, profdata_path, binary_report_dir,
                                [binary], restricted_dirs)


def get_commandline_args():
    """
    parse command line
    :return: args structure
    """
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('profile_data_dir',
                        help='Path to the directory containing the raw profiles')
    parser.add_argument('binaries', metavar='B', type=str, nargs='*',
                        help='Path to an instrumented binary')
    parser.add_argument('--preserve-profiles',
                        help='Do not delete raw profiles', action='store_true')
    parser.add_argument('--unified-report', action='store_true',
                        help='Emit a unified report for all binaries')
    parser.add_argument('--restrict', metavar='R', type=str, nargs='*',
                        default=[],
                        help='Restrict the reporting to the given source paths'
                        ' (must be specified after all other positional arguments)')
    return parser.parse_args()


if __name__ == '__main__':
    args = get_commandline_args()

    # default llvm-profdata and llvm-cov binary from docker image
    profdata_exe = 'llvm-profdata'
    cov_exe = 'llvm-cov'
    report_dir = os.path.join('build', 'codecoveragereport')

    if os.path.isdir(report_dir):
        # Delete all contents of a directory and ignore errors
        shutil.rmtree(report_dir, ignore_errors=True)
    os.makedirs(report_dir, exist_ok=True)

    profdata_path = merge_raw_profiles(profdata_exe,
                                       args.profile_data_dir,
                                       report_dir)
    if not len(args.binaries):
        print('\n:: No binaries specified, no work to do!')
        sys.exit(1)

    prepare_html_reports(cov_exe, profdata_path, report_dir,
                             args.binaries, args.unified_report, args.restrict)
    if not args.preserve_profiles:
        os.remove(profdata_path)

