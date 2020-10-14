#!/usr/bin/python

strip_comments_and_strings = True


def evaluate_matches(lines, fn):
    errors = []

    for lineno, line in enumerate(lines):
        if line.count('boost::bind'):
            errors.append(
                (fn, lineno+1, "'boost::bind' is deprecated, use a lambda function instead."))
        if line.count('boost::ref'):
            errors.append(
                (fn, lineno+1, "There is no reason to use 'boost::ref'."))

    return errors
# /end evaluate_matches


forbidden = [
    'table_->set_column_compare(0, boost::bind(&LoadOrSaveGame::compare_save_time, this, _1, _2));'
    'table_->set_column_compare(0, boost::bind(&LoadOrSaveGame::compare_save_time, boost::ref(*this), _1, _2));'
]

allowed = [
    'table_->set_column_compare(0, [this](uint32_t a, uint32_t b) { compare_save_time(a, b); });'
]
