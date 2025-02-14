#!/bin/env python3

"""Validates github workflow files.

- checks if the called local workflows exist
- checks if the files checked for changes (by https://github.com/dorny/paths-filter) exist
- checks if listed files are in valid yaml string (happens while loading it)

For a troughout test the author recommends https://github.com/rhysd/actionlint/
this tool just does some additional tests.
"""

import fnmatch
import glob
import io
import os
import subprocess
import sys
import yaml


def file_in_git(file):
    'returns True if the file is tracked by git'
    cmd = ['git', 'ls-files', '--error-unmatch', file]
    fp = subprocess.run(cmd, stdout=subprocess.DEVNULL)
    return fp.returncode == 0


class CheckGithubYaml:
    def __init__(self):
        self.failures = 0
        self.checked_files = 0

    def _check_glob_path_exists(self, file, retry=True):
        try:
            i = glob.iglob(file, recursive=True)
            match1 = next(i)
            # because xxx/** just returns xxx/ without checking:
            if '**' not in file or os.path.exists(match1):
                return match1, i
        except StopIteration:
            if retry and '**' in file:
                # because **.xx is not interpretet as *.xx in any subdir
                return self._check_glob_path_exists(file.replace('**', '**/*'), False)
        return None

    def _check_glob_path_valid(self, file, ref):
        exists = False
        if os.path.exists(file) or self._check_glob_path_exists(file):
            exists = True
            if file_in_git(file):
                return True
        elif '!(' in file and ')' in file:
            # check each excluded part of !(xx|yy)
            l = file.find('!(')
            r = file.rfind(')')
            ex_parts = file[l+2:r].split('|')
            ex_patterns = []
            for ex_part in ex_parts:
                mfile = file[0:l] + ex_part + file[r+1:]
                self._check_glob_path_valid(mfile, ref)
                ex_patterns.append(mfile)
            # replace !(xx|yy) by * to check if any other match exists
            mfile = file[0:l] + '*' + file[r+1:]
            mi = self._check_glob_path_exists(mfile)
            if mi:
                match_path, i = mi
                while match_path:
                    for p in ex_patterns:
                        if fnmatch.fnmatch(match_path, p):  # glob matches with fnmatch
                            break
                    else:
                        if file_in_git(match_path):
                            return True  # found another match
                    match_path = next(i, None)
        self._report_path_invalid(file, exists, ref)
        return False

    def _check_path_valid(self, file, ref):
        exists = False
        if os.path.exists(file):
            exists = True
            if file_in_git(file):
                return True
        self._report_path_invalid(file, exists, ref)
        return False

    def _report_path_invalid(self, file, exists, ref):
        if exists:
            msg = 'untracked file:'
        else:
            msg = 'missing file:'
        if os.getenv('GITHUB_ACTION'):
            msg = f'::error file={ ref["file"] }::{ msg }'
        print(msg, file, 'from', ref)
        self.failures += 1

    def _check_filter_files(self, files: iter, ref: dict):
        for file in files:
            if isinstance(file, list):
                self._check_filter_files(file, ref)
            elif isinstance(file, dict):
                self._check_filter_files(file.values(), ref)
            else:
                self._check_glob_path_valid(file, ref)

    def _check_steps(self, steps: iter, yaml_path: str, path: str):
        for step in steps:
            if step.get('uses', '').startswith('dorny/paths-filter'):
                for k, files in yaml.safe_load(io.StringIO(step['with']['filters'])).items():
                    ref = {'file': path, 'in': f'{yaml_path}.steps.with.filters'}
                    self._check_filter_files(files, ref)
            elif (step.get('uses', '').startswith('./')):
                # check use of local action
                ref = {'file': path, 'in': f'{yaml_path}.steps.uses'}
                self._check_path_valid(step['uses'], ref)

    def run(self, path):
        with open(path) as f:
            w = yaml.safe_load(f)
        if w.get('jobs'):
            # it is a workflow file
            for j_name, j_data in w['jobs'].items():
                steps = j_data.get('steps')
                if steps:
                    self._check_steps(steps, f'jobs.{j_name}', path)
                elif (j_data['uses'].startswith('./')):
                    # check use of shared workflow
                    ref = {'file': path, 'in': 'jobs.uses'}
                    self._check_path_valid(j_data['uses'], ref)
            self.checked_files += 1
        elif w['runs'].get('using') == 'composite':
            # it is a composite action
            self._check_steps(w['runs'].get('steps', []), 'runs', path)
            self.checked_files += 1


if __name__ == '__main__':
    check = CheckGithubYaml()
    for arg in sys.argv[1:]:
        check.run(arg)
    if check.failures:
        print(f'{check.failures} failures in {check.checked_files} files 🟧')
        sys.exit(1)
    elif check.checked_files == 0:
        print('no file checked ⁉️')
    else:
        print(f'no failure in {check.checked_files} file(s) 😀')
