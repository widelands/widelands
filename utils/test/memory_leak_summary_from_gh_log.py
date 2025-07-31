#!/bin/env python3
"""write github job summary from github log.

pipe to this script input like from:
- unzip -p .../wl_logs_run_41823906582.zip
    '10_testsuite _ testsuite (Debug, ubuntu-24.04, g++-13).txt'
    # zip fetched from github, with "download log archive"
- curl https://github.com/widelands/widelands/commit/$C_HASH/checks/$JOB_NUM_ID/logs
    # one way to get a raw log by url
- GITHUB_ACTION=dummy ./regression_test.py
- gh run view --log --job 456789  # maybe, unchecked
- cat /tmp/widelands_regression_test_XXX/lsan_XX.XXX.txt  # file created by ./regression_test.py

Or give a file in this format as argument.

If not running on GitHub, those variables might be helpful:
WLRT_COMPILE_DIR: Name of the directory widelands was compiled in. Default is widelands.
                  Origins are not found in traceback if this value is wrong.
GITHUB_SHA: commit hash for widelands. Used in links for searching or creating issues.
WLRT_LOG_URL: url to the log, used in one link

"""

import enum
import sys
import os
import re


class GithubASan:
    summary_file = None  # path of github summary file
    found_local_tb = True  # true if not looking for our line in current traceback
    test_script = None
    ansi_escape_pattern = None  # for remove_ansi_escape_sequences()
    counted_leaks = None  # counts memory leaks from one run of widelands
    leaks_by_origin = {}  # memory leaks grouped by origin

    @classmethod
    def github_asan_line(cls, text):
        """annotates asan summary on GitHub and writes more info to the steps
        summary file."""
        # GitHub only supports 10 error and 10 warning annotations per step. This might be too few.
        # Therefore only annotate the summary line of ASan and write other info to the steps summary
        def write_summary(arg1, *args):
            """write to github summary file (markdown format)"""
            with open(cls.summary_file, 'a') as summary_file:
                summary_file.write(arg1)
                for arg in args:
                    summary_file.write(arg)

        def remove_ansi_escape_sequences(text):
            """removes ansi escape sequences (because github summary does not
            support them)"""
            if not cls.ansi_escape_pattern:
                # \x1b is the escape character, the other one the textual representation used by gh
                cls.ansi_escape_pattern = re.compile(r'[\x1b\u241b][^m]*m')
            return cls.ansi_escape_pattern.sub('', text)

        if 'ERROR: ' in text and cls.summary_file:
            if cls.counted_leaks is None:
                goto_txt = 'go to the log'
                if os.getenv('GITHUB_ACTION'):
                    goto_txt = goto_txt + ' (from menu `...` on top right of this text block)'
                elif os.getenv('WLRT_LOG_URL'):
                    goto_txt = f'''({goto_txt})[{os.getenv('WLRT_LOG_URL')}]'''
                else:
                    goto_txt = goto_txt + ' (you gave as input)'
                write_summary(
                    '## memory leaks <a name=memory-leak></a>\n\n> [!TIP]\n'
                    f'> To find the full traceback, {goto_txt} '
                    'and copy the line from the traceback here in the log\'s search field.\n')
            write_summary('\n### ', remove_ansi_escape_sequences(text))
            write_summary('triggered by test ', cls.test_script or '???', '\n\n')
            cls.counted_leaks = 0
        if 'SUMMARY' in text:
            if cls.summary_file and cls.counted_leaks > 0:
                summary_tag = f'\n<summary>{cls.counted_leaks} leaks</summary>\n'
                write_summary(summary_tag, '</details>\n', text)
            return '::warning title=ASan error::' + text
        if 'irect leak of ' in text and cls.summary_file:  # Direct or Indirect leak
            if cls.counted_leaks == 0:
                write_summary('<details>\n\n')  # needs empty line for the following list
            write_summary('+ ', remove_ansi_escape_sequences(
                text.replace(' allocated from:', '', 1)))
            cls.found_local_tb = False
            cls.counted_leaks += 1
        compile_d = os.getenv('WLRT_COMPILE_DIR', 'widelands')  # default is for GitHub
        if not cls.found_local_tb and '/third_party/' not in text and \
                (f'/{compile_d}/src/' in text or ' src/' in text):
            cls.found_local_tb = True
            if ' src/' in text:  # stipped by ASAN_OPTIONS='strip_path_prefix=/...
                f_name_line = 'src/' + text.rsplit(' src/', 1)[1].rstrip()
            else:
                f_name_line = text.rsplit(f'/{compile_d}/', 1)[1].rstrip()
            if cls.summary_file:
                # add code part in `` to avoid invalid html
                to_write = ')`'.join(text.rsplit(')', 1)).replace(
                    ' in ', '` in `', 1).replace('#', '`#', 1)
                write_summary('    our origin: ', to_write.replace(
                    f_name_line, f'[{f_name_line}](<#user-content-{f_name_line.lower()}> "to grouped")'))
                data = {'tb': to_write, 'test': cls.test_script}
                cls.leaks_by_origin.setdefault(f_name_line, []).append(data)
            if os.getenv('WLRT_ANNOTATE_LINE'):  # with strategy.job-index == 0 from workflow
                # annotate only one test job to not have many dublicate messages on one line
                # unfortunately the jobs can not coordinate
                f_name, line_no = f_name_line.rsplit(':', 2)[:2]
                return f'::notice file={f_name}, line={line_no}::memory leak from {f_name_line}\n' \
                    f'{text}'
            else:
                # only mark in log
                return f'::notice::memory leak from {f_name_line}\n{text}'
        return text

    @classmethod
    def set_test_script(cls, test_script):
        cls.test_script = test_script

    @classmethod
    def set_summary_file(cls, summary_file):
        cls.summary_file = summary_file

    @classmethod
    def summarize_origins(cls):
        def to_query_str(txt):
            """only escape what is needed for our case."""
            return txt.translate({ord(':'): '%3A', ord('"'): '%22', ord('&'): '%26'})

        def key_path_of_tb(data):
            """for sorting by path in traceback line (ignoring memory
            address)"""
            # sort by: called code, then test
            return data['tb'].split(' in ', 1)[-1], data['test']

        def key_file_line_col(file_line_col):
            """for sorting by line numerically."""
            return [int(e) if e.isdecimal() else e for e in file_line_col.rsplit(':', 2)]

        if not cls.leaks_by_origin or not cls.summary_file:
            return
        with open(cls.summary_file, 'a') as summary_file:
            summary_file.write('\n\n## memory leaks by origin\n\n')
            for origin in sorted(cls.leaks_by_origin, key=key_file_line_col):
                search_on_gh = ('/' + os.getenv('GITHUB_REPOSITORY', 'widelands/widelands') +
                                '/issues?q=' + to_query_str(
                    f'is:issue state:open label:"memory & performance" '
                    f'''"{origin.rsplit(':', 2)[0]}"'''))
                summary_file.write(
                    f'\n<details><summary>\n\n#### {origin} <a name="{origin}"></a>\n</summary>\n\n'
                    f'[search issue on gh](<{search_on_gh}>)\n')
                for data in sorted(cls.leaks_by_origin[origin], key=key_path_of_tb):
                    summary_file.write(
                        f'''+{data['tb']}    triggered by {data['test'] or '???'}\n''')
                summary_file.write('</details>\n')
        cls.leaks_by_origin = {}
        summary_hint = '\nsee about memory leaks in job summary on'
        if os.getenv('GITHUB_RUN_ID'):
            print(summary_hint,
                  os.getenv('GITHUB_SERVER_URL', 'https://github.com') + '/' +
                  os.getenv('GITHUB_REPOSITORY', 'widelands/widelands') +
                  '/actions/runs/' + os.getenv('GITHUB_RUN_ID') +
                  '#user-content-memory-leak')
        elif cls.summary_file and not cls.summary_file.startswith('/dev/'):
            print(summary_hint, cls.summary_file, file=sys.stderr)


def create_summary_one_runner(in_file):
    class ReadingMode(enum.Enum):
        BEGIN = enum.auto()
        INSIDE = enum.auto()
        RT_HEADER = enum.auto()
        RT_LOGS = enum.auto()
        RT_HEADER_L = enum.auto()  # when log is local
        RT_LOGS_L = enum.auto()
        LSAN = enum.auto()

    def get_log_line_gh(line):
        """get the normal log line.

        without timestamp and github label

        """
        if 'Z ##[' in line:
            line, cnt = re.subn(r'^.*Z ##\[[^]]+\]', '', line, 1)
            if cnt > 0:  # if it really was with github label
                return line
        return line.split('Z ', 1)[-1]

    def get_log_line_local(line):
        """for when input is not from github."""
        if '::' in line and line.startswith('::'):
            idx = line.find('::', 2)
            if idx > 0:
                return line[idx + 2:]  # without ::warning xxx::
        return line

    get_log_line = get_log_line_gh
    if not os.getenv('GITHUB_STEP_SUMMARY') and not '/dev/' in os.devnull:
        print('set environment variable GITHUB_STEP_SUMMARY to get output', sys.stderr)
        os.exit(19)  # exit with "no such device exists" (stdout file not found)
    step_summary_p = os.getenv('GITHUB_STEP_SUMMARY')
    if step_summary_p and os.path.isfile(step_summary_p) and os.stat(step_summary_p).st_size > 0:
        with open(step_summary_p, 'a') as summary_file:
            summary_file.write('\n\n\n\n')  # write separating empty lines
        print('WARNING: ' + step_summary_p + ' has content', file=sys.stderr)
    GithubASan.set_summary_file(os.getenv('GITHUB_STEP_SUMMARY', '/dev/stdout'))

    if in_file.isatty():
        print(f'  hint: program expects input on {in_file.name}', file=sys.stderr)

    r_mode: ReadingMode = ReadingMode.BEGIN
    for line in in_file:
        match r_mode:
            case ReadingMode.BEGIN:
                if '========' in line and 'Z ' not in line:  # directly a log as input
                    r_mode = ReadingMode.LSAN
                    get_log_line = get_log_line_local
                elif line != '\n':  # when not an empty line
                    r_mode = ReadingMode.INSIDE
            case ReadingMode.INSIDE:
                if ' ##[group]Run ' in line and 'regression_test.py' in line:
                    r_mode = ReadingMode.RT_HEADER
                elif ' tests with ' in line and line.startswith('Will run '):
                    r_mode = ReadingMode.RT_HEADER_L
            case ReadingMode.RT_HEADER | ReadingMode.RT_HEADER_L:
                if '------' in line:
                    if r_mode == ReadingMode.RT_HEADER_L:
                        r_mode = ReadingMode.RT_LOGS_L
                        get_log_line = get_log_line_local
                    else:
                        r_mode = ReadingMode.RT_LOGS
            case ReadingMode.RT_LOGS | ReadingMode.RT_LOGS_L:
                if ' ##[group] ' in line and 'lsan' in line:
                    # matching on "lsan.245:" and "lsan_01.732:"
                    r_mode = ReadingMode.LSAN
                elif '::group:: ' in line and 'lsan' in line:
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
                elif '::endgroup::' in line:
                    r_mode = ReadingMode.RT_LOGS_L
                else:
                    GithubASan.github_asan_line(get_log_line(line))
    GithubASan.summarize_origins()


def main():
    in_file = None
    if len(sys.argv) == 1:
        pass
    elif sys.argv[1] in ('-h', '--help'):
        print(__doc__)
        return
    elif sys.argv[1] != '-':
        in_file = open(sys.argv[1])
    try:
        create_summary_one_runner(in_file or sys.stdin)
    finally:
        in_file and in_file.close()


if __name__ == '__main__':
    main()
