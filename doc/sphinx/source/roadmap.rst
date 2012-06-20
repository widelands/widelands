Roadmap
=======

The Lua implementation in widelands is very young. Until build-16 it is to be
extended and changed without any notice. Future version will (try) to change
the API either in a backwards compatible way or add deprecation warnings to
function that have changed or will be removed for at least one build. So every
feature that is in build-16 will also be in build-17 but might be changed or
removed in build-18. 

New features will be implemented on a per need basis. If you find that the
current Lua abstractions can't do anything you want to do, please file a bug
report and chances are someone will implement these features.

Currently unimplemented features
--------------------------------

These are feature which are currently not implemented. If they will be
implemented depends on how useful they are. If you want anything implemented,
please open a bug report and this will be pushed. 

* Access to player stock via ``Player.get_wares`` and ``get_workers``.
* Access to constructionsites via ``Player.get_buildings``.
* Creating busy roads and second carries on them.
* Allow and forbid worker types. Currently all workers are always allowed.
* Access to bobs, workers. Directly control worker's working stack. This would
  allow to send bobs to specific map locations via the API. 

