#!/bin/env python3
"""write github job summary from github log

pipe to this script input like from:
- unzip -p .../wl_logs_run_41823906582.zip
    '10_testsuite _ testsuite (Debug, ubuntu-24.04, g++-13).txt'
    # zip fetched from github, with "download log archive"
- curl https://github.com/widelands/widelands/commit/$C_HASH/checks/$JOB_NUM_ID/logs
    # one way to get a raw log by url
- gh run view --log --job 456789  # maybe, unchecked
- cat /tmp/widelands_regression_test_XXX/lsan_XX.XXX.txt  # file created by ./regression_test.py
"""

import enum
import sys
import os


class GithubASan:
    found_local_tb = True
    test_script = None

    @classmethod
    def github_asan_line(cls, text):
        "annotates asan summary on GitHub and writes more info to the steps summary file"
        # GitHub only supports 10 error and 10 warning annotations per step. This might be too few.
        # Therefore only annotate the summary line of ASan and write other info to the steps summary
        def write_summary(arg1, *args):
            "write to github summary file"
            with open(os.getenv('GITHUB_STEP_SUMMARY'), 'a') as summary_file:
                summary_file.write(arg1)
                for arg in args:
                    summary_file.write(arg)

        if 'ERROR: ' in text and os.getenv('GITHUB_STEP_SUMMARY'):
            write_summary('### ', text)  # this text is colored
            write_summary('triggered by test ', cls.test_script, '\n\n')
        if 'SUMMARY' in text:
            if os.getenv('GITHUB_STEP_SUMMARY'):
                write_summary('\n', text)
            return '::warning title=ASan error::' + text
        if 'irect leak of ' in text and os.getenv('GITHUB_STEP_SUMMARY'):  # Direct or Indirect leak
            write_summary('+ ', text.replace(' allocated from:', '', 1))  # this text is colored
            cls.found_local_tb = False
        if not cls.found_local_tb and '/widelands/src/' in text and '/third_party/' not in text:
            if os.getenv('GITHUB_STEP_SUMMARY'):
                write_summary('    our origin: ', text)
            cls.found_local_tb = True
            return f'::notice title=memory leak origin::{text}'
        return text

    @classmethod
    def set_test_script(cls, test_script):
        cls.test_script = test_script


def create_summary_one_runner():
    class ReadingMode(enum.Enum):
        BEGIN = enum.auto()
        INSIDE = enum.auto()
        RT_HEADER = enum.auto()
        RT_LOGS = enum.auto()
        LSAN = enum.auto()

    def get_log_line(line):
        "get the log line without timestamp"
        return line.split('Z ', 1)[-1]

    r_mode: ReadingMode = ReadingMode.BEGIN
    for line in sys.stdin:
        match r_mode:
            case ReadingMode.BEGIN:
                if '========' in line:  # directly a log as input
                    r_mode = ReadingMode.LSAN
                elif line != '\n':  # when not an empty line
                    r_mode = ReadingMode.INSIDE
            case ReadingMode.INSIDE:
                if ' ##[group]Run ' in line and 'regression_test.py' in line:
                    r_mode = ReadingMode.RT_HEADER
            case ReadingMode.RT_HEADER:
                if '------' in line:
                    r_mode = ReadingMode.RT_LOGS
            case ReadingMode.RT_LOGS:
                if ' ##[group]' in line and 'lsan.' in line:
                    r_mode = ReadingMode.LSAN
                elif ': ' in line and ('Passed' in line or 'FAILED' in line or 'TIMED OUT' in line):
                    # this is before stdout
                    idx = line.find(': ')
                    if idx > 0:
                        test_name = line[idx+2:]
                    else:
                        test_name = None
                    GithubASan.set_test_script(test_name)
            case ReadingMode.LSAN:
                if ' ##[endgroup]' in line:
                    r_mode = ReadingMode.RT_LOGS
                else:
                    GithubASan.github_asan_line(get_log_line(line))


def main():
    create_summary_one_runner()


if __name__ == '__main__':
    main()
