#!/usr/bin/python

strip_comments_and_strings = True


whitelist = [
    'boost::format',
    'boost::signals2',
    'boost::uuids',
    'boost::asio',
]

def evaluate_matches(lines, fn):
    errors = []

    for lineno, line in enumerate(lines):
        if line.count('boost::'):
            whitelisted = False
            for w in whitelist:
                if w in line:
                    whitelisted = True
                    break
            if not whitelisted:
                offending = line[line.find('boost::'):]
                offending = offending[0:min(offending.find(' '), offending.find('(')):]
                errors.append(
                    (fn, lineno+1, "'{}' is deprecated.".format(offending)))

    return errors
# /end evaluate_matches


forbidden = [
    'table_->set_column_compare(0, boost::bind(&LoadOrSaveGame::compare_save_time, this, _1, _2));',
    'table_->set_column_compare(0, boost::bind(&LoadOrSaveGame::compare_save_time, boost::ref(*this), _1, _2));',
    'boost::trim(filename);',
]

allowed = [
    'table_->set_column_compare(0, [this](uint32_t a, uint32_t b) { compare_save_time(a, b); });',
    'str = (boost::format("foo %1%") % 5).str();',
]
