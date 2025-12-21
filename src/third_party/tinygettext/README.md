tinygettext
===========

tinygettext is a minimal replacement for gettext written in C++. It
can read `.po` files directly and doesn't need `.mo` files generated
from `.po`. It also can read the `.po` files from arbitrary locations,
so it's better suited for non-Unix systems and situations in which one
wants to store or distribute `.po` files separately from the software
itself. It is licensed under
[zlib license](http://en.wikipedia.org/wiki/Zlib_License).

The latest version can be found at:

* https://github.com/tinygettext/tinygettext


Detecting the locale setting
----------------------------

Different operating systems store the default locale in different
places; a portable way to find it is provided by FindLocale:

* http://icculus.org/~aspirin/findlocale/


Projects using this library
---------------------------

* [0 A.D.](http://play0ad.com/)
* [ET:Legacy](https://www.etlegacy.com/)
* [LinCity-NG](https://github.com/lincity-ng/lincity-ng)
* [Me and My Shadow](http://meandmyshadow.sourceforge.net/)
* [Multi Theft Auto](http://www.multitheftauto.com/)
* [Pingus](http://pingus.seul.org/)
* [SuperTux](http://supertuxproject.org/)
* [SuperTuxKart](http://supertuxkart.net/)
* [Unvanquished](https://www.unvanquished.net/)
