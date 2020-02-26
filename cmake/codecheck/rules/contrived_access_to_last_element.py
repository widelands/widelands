#!/usr/bin/python


"""This catches clumsy expressions like some_container[some_container.size()

- 1] which should be written as *some_container.rbegin()
"""

error_msg = "Simplify \"some_container[some_container.size() - 1];\" to \"*some_container.rbegin();\"."

regexp = r"""(.*) *\[ *\1 *\. *size *\( *\) *- *1 *\]"""

forbidden = [
    'some_container[some_container.size() - 1];'
]

allowed = [
    '*some_container.rbegin();'
]
