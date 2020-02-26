#!/usr/bin/python


"""
This detects redundant code like "if (a == true)" (shoud be "if (a)") and
"while (b = false)" (should be "while (not b)").
"""

error_msg = "Do not explicitly check for boolean state. Use if (a) instead of if (a == true)."

strip_comments_and_strings = True

regexp = r'''(?:[!=]= *(?:false|true)[^_0-9a-zA-Z])|(?:[^_0-9a-zA-Z](?:false|true) *[!=]=)'''

forbidden = [
    'if (abc == false)',
    'if (abc != false)',
    'if (false == abc)',
    'if (false != abc)',
    'if (abc == true)',
    'if (abc != true)',
    'if (true == abc)',
    'if (true != abc)',
]

allowed = [
    'if (abc)',
    'if (not abc)',
    'if (is_false == abc)',
    'if (is_false != abc)',
    'if (abc == falsevity)',
    'if (abc != falsevity)',
    'if (obstrue == abc)',
    'if (obstrue != abc)',
    'if (abc == true_thing)',
    'if (abc != true_thing)',
]
