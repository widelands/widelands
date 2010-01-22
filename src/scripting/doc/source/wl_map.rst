:mod:`wl.map`
=============

.. module:: wl.map
   :synopsis: Provides access on maps and fields

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.map

MapObject
----------

.. class:: MapObject

    This is the base class for all Objects in widelands, including immovables
    and Bobs. This class can't be instantiated directly, but provides the base
    for all others.

    .. attribute:: serial

    The serial number of this object. Note that this value does not stay
    constant after saving/loading.
    
    .. method:: remove()

        Removes this object from the game immediately. If you want to destroy an
        object as if the player had see :func:`destroy`.

        :returns: :const:`nil`
    
BaseImmovable
-------------

.. class:: BaseImmovable

    Bases: :class:`MapObject`

    This is the base class for all Objects in widelands, including immovables
    and Bobs. This class can't be instantiated directly, but provides the base
    for all others.

