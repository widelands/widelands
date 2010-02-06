#!/usr/bin/python


"""
This is something that should be caught before it reaches Nicolai's eyes. The
explicit use of "this->" is not allowed. But it should not catch things like
"do_this->now". It should also not catch things like
"(this->*(action->function))(g, state, action)", which seem to be needed in
some places.
"""

error_msg = "Do not use this->m_a, use m_a instead."

regexp=r"""(?:^|[^_a-zA-Z0-9])this *-> *[^*]"""

forbidden = [
    'this->m_b',
    '	 this->m_b',
    '	 this  ->   m_b',
]

allowed = [
    'm_b',
    'gthis->m_b',
    '(this->*(action->function))(g, state, action)',
]
