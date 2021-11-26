#!/usr/bin/python

strip_comments_and_strings = True


whitelist = [
    'boost::format',
    'boost::io::format_error',
    'boost::uuids',
    'boost::asio',
    'boost::system::error_code',
    'boost::system::system_error',
]
whitelist_headers = [
    'boost/format',
    'boost/uuid',
    'boost/asio',
    'boost/version',
]


def evaluate_matches(lines, fn):
    errors = []

    for lineno, line in enumerate(lines):
        if line.count('#include') and line.count('boost/'):
            whitelisted = False
            for w in whitelist_headers:
                if w in line:
                    whitelisted = True
                    break
            if not whitelisted:
                errors.append(
                    (fn, lineno+1, 'Forbidden {}'.format(line[0:-1:])))
        elif line.count('boost::'):
            whitelisted = False
            for w in whitelist:
                if w in line:
                    whitelisted = True
                    break
            if not whitelisted:
                offending = line[line.find('boost::'):]
                pos1 = offending.find('(')
                pos2 = offending.find(' ')
                pos3 = offending.find(';')
                if pos1 <= 0:
                    pos1 = 1000
                if pos2 <= 0:
                    pos2 = 1000
                if pos3 <= 0:
                    pos3 = 1000
                offending = offending[0:min(pos1, pos2, pos3):]
                errors.append(
                    (fn, lineno+1, "'{}' is deprecated.".format(offending)))

    return errors
# /end evaluate_matches


forbidden = [
    'table_->set_column_compare(0, boost::bind(&LoadOrSaveGame::compare_save_time, this, _1, _2));',
    'table_->set_column_compare(0, boost::bind(&LoadOrSaveGame::compare_save_time, boost::ref(*this), _1, _2));',
    'boost::trim(filename);',
    '#include <boost/algorithm/string.hpp>',
]

allowed = [
    'table_->set_column_compare(0, [this](uint32_t a, uint32_t b) { compare_save_time(a, b); });',
    'str = (boost::format("foo %1%") % 5).str();',
    'std::string boost = "This function will boost performance.";',
    '#include <boost/format.hpp>',
]
