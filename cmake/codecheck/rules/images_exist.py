#!/usr/bin/python3 -tt

"""Checks that images loaded via g_image_cache->get() exist.

Some images can't be checked, e.g. when they are being passed via a
function.
"""

import os.path
import re


FIND_LITERALS = re.compile(r"""g_image_cache->get\("(.*?)"\)""")
# We skip pointers and function calls here
FIND_VARIABLE_NAMES = re.compile(
    r"""g_image_cache->get\(([0-9a-zA-Z_]+?)\)""")

# Find the data path
base_path = os.getenv('WL_ROOT_DIR', os.getcwd())
data_path = os.path.join(base_path, 'data')
template_path = os.path.join(data_path, 'templates', 'default')

def evaluate_matches(lines, fn):
    errors = []
    variables = []

    for lineno, line in enumerate(lines, 1):
        matches = FIND_LITERALS.findall(line)
        if len(matches) > 0:
            # Check for images that are fetched by direct quotes
            for match in matches:
                if '"' in match:
                    # Let's not deal with assembled filenames like
                    # "images/wui/editor/editor_menu_tool_" pic ".png" or
                    # "images/" + image_basename + ".png" for now
                    continue

                image_file_1 = os.path.join(data_path, match)
                image_file_2 = os.path.join(template_path, match)
                if not (os.path.exists(image_file_1) and os.path.isfile(image_file_1)) and not (os.path.exists(image_file_2) and os.path.isfile(image_file_2)):
                    errors.append([fn, lineno, 'Image file does not exist: %s' % match])

        else:
            # Collect image variable names
            matches = FIND_VARIABLE_NAMES.findall(line)
            if len(matches) > 0:
                for match in matches:
                    variables.append(match)

    # Try to identify images that are fetched by variable name.
    # We won't be able to check everything, e.g. when the filename is passed via
    # a function parameter
    for variable in variables:
        found = False
        no_matches = 0
        found_on_line = 1
        FIND_VARIABLES = re.compile(r'\b' + variable + r"""\s*=\s*"(.+?)";""")
        for lineno, line in enumerate(lines, 1):
            matches = FIND_VARIABLES.findall(line)
            if len(matches) > 0:
                no_matches = no_matches + len(matches)
                for match in matches:
                    image_file_1 = os.path.join(data_path, match)
                    image_file_2 = os.path.join(template_path, match)
                    if (os.path.exists(image_file_1) and os.path.isfile(image_file_1)) or (os.path.exists(image_file_2) and os.path.isfile(image_file_2)):
                        found = True
                        break
                    else:
                        found_on_line = lineno
        if no_matches > 0 and not found:
            errors.append(
                [fn, found_on_line, 'Unable to find image file for variable: %s' % variable])

    return errors


# File this is called on is always called testdir/test.h
forbidden = [
    # Loading image file that does not exist
    'g_image_cache->get("FoofakeImage")',
]

allowed = [
    # Loading image file that exists
    'g_image_cache->get("images/logos/wl-ico-128.png")',
    'g_image_cache->get(kConstexpr)',
    'g_image_cache->get("images/" + "foo/nonexistent.png")',
    'g_image_cache->get(format("images/%s", "foo/nonexistent.png"))',
    'g_image_cache->get("loadscreens/ending.png")',
]
