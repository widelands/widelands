Compiling Widelands
===================

The latest information about compiling Widelands can be found in the Wiki (https://wl.widelands.org/wiki/BuildingWidelands/).

Additional plattform dependant information for building Widelands under:
   - Windows: https://wl.widelands.org/wiki/BuildingWidelandsUnderWindows/
   - Mac: https://wl.widelands.org/wiki/BuildingWidelandsMac/
can also be found in the wiki.


If you are a developer, you might (optionally!) make use of the following software:
   - ctags
   - astyle
     astyle does not layout correctly when structs are used like public classes (C++)
     and not in the traditional C sense. If you decide to use this, please
     double-check the results before committing
   - pngrewrite and optipng
     Used to make PNG images more compact. You most certainly don't need this;
     compactification is a once-a-year, one-person-only job. If you don't know
     already that you will be doing it, forget about it.
   - Krazy
     A sourcecode checker from the KDE project, subversion repository available at
     svn://anonsvn.kde.org/home/kde/trunk/quality/krazy2
   - Doxygen
     Used to generate source code documentation.


Windows
-------
If you're searching for a good SVN tool for windows, we recommend Tortoise SVN.
Check http://tortoisesvn.sourceforge.net.

InnoSetup
^^^^^^^^^
Since Build10 we support an Innosetup file, which can be used for compiling a Setup
(like the official Widelands-Setup available on sourceforge.net-mirrors).
Innosetup can be downloaded from http://www.jrsoftware.org

If you've installed InnoSetup, you just need to open [Widelands]/build/win32/Widelands.iss.
You might change few settings or directly start packing/compiling the setup.

.. note:: Please check if all needed \*.dll-files are in [Widelands]-directory during Setup packing/compile.
          Else your setup might be useless :-?
