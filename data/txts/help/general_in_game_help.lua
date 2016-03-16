include "scripting/formatting.lua"
include "txts/help/common_helptexts.lua"

set_textdomain("texts")

return {
   title = _"Help",
   text =
      title(_"Help") ..
      rt(
         help_introduction() ..
         h2(_[[General game control]]) ..
         p(_[[To close an open window, right-click on it. To minimize/maximize a window, click on it with the middle mouse button or with the left mouse button while holding down Ctrl.]]) ..
         p(_[[If you hold Ctrl while building a road, flags will be placed automatically. If you hold Ctrl while removing a road, all flags up to the first junction are removed.]]) ..
         p(_[[You can skip confirmation windows for irreversible actions if you hold Ctrl while clicking on the action button.]]) ..

         h2(_[[Keyboard shortcuts]]) ..
         p(
            _"Page Up: increases game speed" .. "<br>" ..
            _"Page Down: decreases game speed" .. "<br>" ..
            _"Pause: pauses the game" .. "<br>" ..
            _"Space: toggles if building spaces are shown" .. "<br>" ..
            _"M: toggles minimap" .. "<br>" ..
            _"N: toggles messages (‘news’)" .. "<br>" ..
            _"C: toggles census" .. "<br>" ..
            _"S: toggles statistics" .. "<br>" ..
            _"I: toggles stock inventory" .. "<br>" ..
            _"O: toggles objectives" .. "<br>" ..
            _"B: toggles building statistics" .. "<br>" ..
            _"F: toggles fullscreen (if supported by the OS)" .. "<br>" ..
            _"Home: centers main mapview on starting location" .. "<br>" ..
            _"(Ctrl +) 0-9: Remember and go to previously remembered locations" .. "<br>" ..
            _",: goes to the previous location" .. "<br>" ..
            _".: goes to the next location" .. "<br>" ..
            _"F6: shows the debug console (only in debug-builds)" .. "<br>" ..
            _"Ctrl + F10: quits the game immediately" .. "<br>" ..
            _"Ctrl + Leftclick: skips confirmation dialogs" .. "<br>" ..
            _"Ctrl + F11: takes a screenshot") ..
         p(_[[In the message window, the following additional shortcuts are available:]]) ..
         p(
            _"0: shows all messages" .. "<br>" ..
            _"1: shows geologists’ messages only" .. "<br>" ..
            _"2: shows economy messages only" .. "<br>" ..
            _"3: shows seafaring messages only" .. "<br>" ..
            _"4: shows warfare messages only" .. "<br>" ..
            _"5: shows scenario messages only" .. "<br>" ..
            _"G: jumps to the location corresponding to the current message" .. "<br>" ..
            _"Delete: archives/restores the current message") ..

          h2(_[[Widelands Help]])
         .. help_tribal_encyclopedia() ..
         p(_"You can find more information about Widelands in the About -> Readme tab.") ..

         help_online_help()
      )
}
