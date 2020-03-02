#!/usr/bin/python -tt


import re
from collections import defaultdict


class EvalMatches(object):
    _indent_words = re.compile(
        r'\b(for|define|typedef|else|do|while|return|if|new|const)\b\s*$')
    _regexp = re.compile(r'^(([\t ]*).*$)', re.MULTILINE)

    def __init__(self):
        pass

    def _handle_closing_curly_brace(self, cor_lineno, cor_indent, lineno, indent, errors, fn):
        if len(cor_indent) != len(indent):
            errors.append((fn, cor_lineno, 'Invalid indent for {} braces'))
            errors.append(
                (fn, cor_lineno, 'Opening brace has an indent of %i' % len(cor_indent)))
            errors.append(
                (fn, lineno, 'Closing brace has an indent of %i' % len(indent)))

    def _handle_closing_round_brace(self, cor_lineno, cor_indent, lineno, indent, errors, fn):
        if len(indent) <= len(cor_indent) or \
           not indent.startswith(cor_indent):
            errors.append(
                (fn, lineno, 'Closing parenthesis without matching leading alignment')
            )

    def __call__(self, lines, fn):
        data = ''.join(lines)
        indents = self._regexp.findall(data)

        errors = []

        in_macro = False
        allowed_indent = '\t'*100
        required_indent = ''
        last_indent = ''
        opening_braces = defaultdict(list)

        bracesets = (
            ('{', '}'),
            ('(', ')'),
        )

        closing_brace_handlers = {
            '{': self._handle_closing_curly_brace,
            '(': self._handle_closing_round_brace,
        }
        brace_diff = {}

        for lineno, (line, indent) in enumerate(indents):
            lineno += 1
            if not len(line):
                indent = ''
            line = line.strip()

            if in_macro or (len(line) and line[0] == '#'):  # Skip macros
                indent = ''
                line = ''
                in_macro = True

            if len(line) and line[-1] != '\\':
                in_macro = False

            # print "line:", line
            # print "  indent: '%s'" % indent.replace('\t','\\t')
            # print "  required_indent: '%s'" % required_indent.replace('\t','\\t')
            # print "  allowed_indent: '%s'" % allowed_indent.replace('\t','\\t')

            if len(line):
                if line[0] == '(':  # param lists can be indented twice
                    allowed_indent += '\t\t'
                    required_indent = last_indent + '\t'
                # } can be at last indent too
                elif line[0] == '}' and len(required_indent):
                    required_indent = required_indent[:-1]
                # { can be at two done (at parameter definitions)
                elif line[0] == '{' and len(required_indent):
                    required_indent = required_indent[:-2]

            nr_unmatched_opening_braces = 0

            # Check for braces
            for (opening, closing) in bracesets:
                n_opening = line.count(opening)
                n_closing = line.count(closing)

                diff = n_opening - n_closing

                brace_diff[opening] = diff

                if diff > 0:  # Opening brace
                    if opening == '(':
                        nr_unmatched_opening_braces = diff
                    for i in range(diff):
                        opening_braces[opening].append(
                            (lineno, indent)
                        )
                elif diff < 0:  # Closing brace
                    try:
                        for i in range(-diff):
                            cor_lineno, cor_indent = \
                                opening_braces[opening].pop()

                            closing_brace_handlers[opening](
                                cor_lineno, cor_indent,
                                lineno, indent, errors, fn
                            )
                    except IndexError:  # Pop from empty list. Unbalanced paren.
                        pass

            cindent = len(indent)

            if len(line) and cindent < len(required_indent) and not brace_diff['('] and not (line[-1] in ';,*/+-&:|'):
                errors.append((fn, lineno, 'Indentation is too shallow'))

            # Check indent relative to previous line
            if cindent > len(allowed_indent):
                errors.append(
                    (fn, lineno, 'Indentation is too deep (allowed are %i)' % len(allowed_indent)))
            else:
                minlen = min(len(allowed_indent), cindent)
                for current, allowed in zip(indent[:minlen], allowed_indent[:minlen]):
                    if current == allowed:
                        continue
                    if current == '\t' and allowed == ' ':
                        continue
                    errors.append(
                        (fn, lineno, 'Indent is invalid. Check for tab/spaces mixup'))
                    break

            # Empty lines are ignored
            if len(line):
                # Otherwise, indent is only allowed to increase, when we have
                # a { or a ) as last character or some conditional keyword
                # on the line

                if nr_unmatched_opening_braces:
                    allowed_indent = indent + ' ' * nr_unmatched_opening_braces + '\t'
                    required_indent = indent
                elif line[-1] in ':=),?+-*/{':
                    allowed_indent = indent + '\t'
                    required_indent = ''
                elif line[-1] == ';' and brace_diff['(']:
                    allowed_indent = indent + '\t'
                    required_indent = ''
                elif self._indent_words.search(line) and line[-1] != ';':
                    allowed_indent = indent + '\t'
                    required_indent = indent + '\t'
                else:
                    allowed_indent = indent
                    required_indent = ''

            # print "  allowed_indent: '%s'" % allowed_indent.replace('\t','\\t')
            # print "  required_indent: '%s'" % required_indent.replace('\t','\\t')

            last_indent = indent

        # print "errors:", errors

        return errors


strip_comments_and_strings = True

evaluate_matches = EvalMatches()


#################
# ALLOWED TESTS #
#################
allowed = [
    """Hallo {
\tAnything;
}""",
    '\t\tHi\n# Macro is ignored\n\t\tIs ok',
    '\t\t\t\n\n\t',  # Empty line is fine
    '\t\t\t\n\n\t\t\t',  # in fact, indent is ignored here

    # Test nicolais indenting style for parameter lists
    """void function
\t\t(int param1, int param2)
{
\treturn 0;
}""",

    # Correct alignment
    """if
\t(a < b &&
\t b > a)""",

    # Another correct alignement
    """\t(new Cmd_Call_Economy_Balance
\t \t(game->get_gametime() + delta, this, m_request_timerid))
""",

    # Indent after :
    """
public:
\tRouter();
""",

    # Indent after =
    """
\tint blah =
\t\t300+50;
""",

    # Indent after return
    '\treturn\n\t\tm_router->find_route(start, end, route, wait, cost_cutoff, map, nodes);',

    # Looks awful, but is valid
    """
\t\t\telse if
\t\t\t\t(tribe.get_ware_descr(ware_type)->default_target_quantity()
\t\t\t\t ==
\t\t\t\t std::numeric_limits<uint32_t>::max())
\t\t\t\tlog
""",

    # Indent after if
    """
\t\tif (2 <= version)
\t\t\ttry {
""",

    # Double indent for double open braces
    """
\t((player->get_buildcaps(unusable_fields.front())
\t  & BUILDCAPS_SIZEMASK) != 0)
""",

    # Ignore content of strings
    # Deliberately with syntax error on second line
    """
if
\t("type is %u but must be one of {%u (WARE), %u (WORKER), "
\t "blah },
\t a, b)
""",

    # Ignore content of strings
    """
if
\t("type is %u but must be one of {%u (WARE), %u (WORKER), "
\t "blah }",
\t a, b)
""",

    # ; is also an indent character
    """
\tfor
\t\t(uint32_t i = 0; i < size;
\t\t ++i, i += i == static_cast<uint32_t>(n)) {
""",

    # , is also an indent character
    """\tRoutingNode() : mpf_cycle(0), mpf_heapindex(0),
\t\tmpf_realcost(0), mpf_backlink(0), mpf_estimate(0) {}""",

    # = is also an indent character, do not confused by the comment
    """\tconst bool is_selected = //  Is this entry selected?
\t\tm_table.has_selection() and m_table.get_selected() == i.value();
""",

    # ? is also an indent char
    """\tbo.type = bld.get_ismine() ?
\t\tBuildingObserver::MINE : BuildingObserver::PRODUCTIONSITE;
""",

    # Checking some problems with comments
    """
/*************/
/* Functions */
/*************/
\tvoid _remove_flag(Flag &);
""",

    # This occurred as bug
    """\tint32_t start_walk(Game & game,
\t\tWalkingDir dir) """,

    # Not too shallow
    """\tuint32_t const attack =
\t\tattacker->get_min_attack() +
\t\t(game.logic_rand()
\t\t %
\t\t (attacker->get_max_attack() - attacker->get_min_attack() - 1));""",

    """else
\t// In editor: play idle task forever
\tset_animation(egbase, descr().get_animation("idle"));
""",

    """int blah () {return 0815;}
int blah1() {return 0815;}""",

    """\tfloat f =
\t\tstatic_cast<float>(game.get_gametime() - m_walkstart)
\t\t/
\t\t(m_walkend - m_walkstart);""",

    """
\tdst.drawanim
\t\t(pos,
\t\t (g_gr->nr_frames(a) - 1) * FRAME_LENGTH,
\t\t get_owner());""",

    """else
{
\tHi""",

    """#ifndef NDEBUG
	(Game & game)
#else
	(Game &)
#endif""",

    """} else {
\tm_name     ->set_text("");
}""",

    # This was a problem with string substitution
    """
\tif
\t\t(a
\t\t (tail[1] == '\'' or tail[1] == '"'))
\t{
\t\treading_multiline = true;
\t\ttail += 2;
\t}""",


    # Not too deep
    """try {
\tanim = building().get_animation("build");
} catch (Map_Object_Descr::Animation_Nonexistent) {
\tanim = building().get_animation("idle");
}""",



    # Eriks tests
    # leading_whitespace_not_matching_previous_line.cc
    """float g(float, float) {
\tg
\t\t(0.0,
\t\t\tg(1.1, 2.2));
}""",
]

###################
# FORBIDDEN TESTS #
###################
forbidden = [
    """\tHallo {
\t\t\tAnything;
\t}""",

    # Should also fail when there is no whitespace in front
    """Hallo {
\t\tAnything;
}""",

    '\tHi\n\n\t\t\tHi',  # Empty line, wrong indent after

    # Opening brace has other leading whitespaces as closing brace
    """RoutingNodeNeighbour(RoutingNode* f, int32_t cost) :
\t\tm_cost(cost) {
\t}
""",

    # 4 Spaces == too much indent
    """
public:
    Router();
""",

    # Alignment error
    """if
\t(a < b &&
\tb > a)""",

    # Another alignment Error
    """
\tstart.mpf_estimate =
\t\tcost_calculator.calc_cost_estimate
\t\t (start.get_position(), end.get_position());
""",

    # Eriks tests
    # closing_parenthesis_without_matching_leading_alignment.cc
    """float g(float, float) {
\treturn
\t\t(0.0 +
\t\tg(1.1, 2.2));
}""",

    # leading_whitespace_not_matching_previous_line.cc
    """void f() {
\tfloat a;
   float b;
}""",


    # indentation_too_shallow
    """void f
(float)
{
}""",

    # indentation_too_deep
    """float a;
\tfloat b;
""",

    """void f() {
\t\tfloat c;
}""",

    """}
\tvoid g() {
""",

    """void g() {
\t\t\t{
\t\t\t}
\t}""",
]
