#!/usr/bin/python

"""These were added to find stuff that makes the code "even more annoying to
read" for Nicolai. It detects uses of the const keyword inside casts, that
have.

no effect, such as "static_cast<const uint32_t>" or "dynamic_cast<T * const>".
"""

error_msg = "Remove the unneeded 'const' from the cast!"

regexp = r"""(?x)
   _cast<\s*                 # '_cast<'
   ((
       const\s+              # 'const   '
       [_a-zA-Z]\w+\s*       # Match an identifier + whitespace
       (
           (::)?\s*          # ::
           [_a-zA-Z]\w+\s*   # Identifier
       )*
       >
   )|(
       [_a-zA-Z]\w+\s*   # Identifier
       (
            \**\s*            # * + whitespace
            const\s*
       )+
       >
   ))
"""

forbidden = [
    '_cast<  const  fgh   >',
    '_cast<  const  fgh ::  ser   >',
    '_cast<   ghf   *  const   >',
    '_cast<   ghf   *  const  *const >',
    '_cast<   ghf  const  *  const   >',
    '_cast<   ghf  const  *  const   *const>',
    '_cast<   ghf**const>',
]

allowed = [
    '<Editor_Player_Menu, Widelands::Player_Number const>',
    '_cast< foo>',
    '_cast< foo *>',
    '_cast< const foo *>',
    '_cast< foo const *>',
    '_cast< foo const * const *>',
    '_cast< foo const * const * const *>',
    '_cast< foo const * * *>',
]
