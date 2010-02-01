Global functions
======================

The following functions are imported into the global namespace
of all scripts that are running inside widelands. They provide convenient
access to other scripts in other locations, localisation features and more.


    .. function:: use(ns, script)

        Includes the script referenced at the caller location. Use this
        to factor your scripts into smaller parts.

        :arg ns:
            The namespace were the imported script resides. Can be any of
                :const:`maps`
                    The script is in the ``scripting/`` directory of the current map.

        :type ns: :class:`string`
        :arg script: The filename of the string without the extension ``.lua``.
        :type script: :class:`string`
        :returns: :const:`nil`

