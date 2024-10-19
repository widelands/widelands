game.desired_speed = 1000

run(function()

   assert_equal(true, wl.ui:shortcut_exists("fullscreen"))
   assert_equal(false, wl.ui:shortcut_exists("this_does_not_exist"))

   wl.ui.MapView():create_child({
      widget   = "unique_window",
      registry = "uipluginstestwindow",
      name     = "uipluginstestwindow",
      title    = "UI Plugins Test",
      tooltip = "Tooltip for window",
      x        = 30,
      y        = wl.ui.MapView().height // 4,
      content  = {
         widget      = "box",
         orientation = "vert",
         name        = "main_box",
         tooltip = "Tooltip for mainbox",
         children    = {
            {
               widget = "textarea",
               name   = "sltext",
               font   = "wui_info_panel_paragraph",
               text   = "Single line text area",
               tooltip = "Tooltip for textarea",
            },
            {
               widget = "space",
               value  = 10,
            },
            {
               widget = "box",
               orientation = "horz",
               name   = "hbox",
               tooltip = "Tooltip for hbox",
               children = {
                  {
                     widget      = "box",
                     orientation = "vert",
                     resizing    = "expandboth",
                     name   = "vbox1",
                     tooltip = "Tooltip for vbox1",
                     children    = {
                        {
                           widget = "checkbox",
                           name   = "checkbox",
                           title  = "This is a CheckBox",
                           state  = true,
                           tooltip = "Tooltip for checkbox",
                        },
                        {
                           widget = "space",
                           value  = 10,
                        },
                        {
                           widget = "button",
                           name   = "button",
                           title  = "Click Me",
                           tooltip = "Tooltip for button",
                           on_click = [[ button_clicked() ]]
                        },
                        {
                           widget = "space",
                           value  = 10,
                        },
                        {
                           widget = "radiogroup",
                           state  = 3,
                           buttons = {
                              {
                                 name = "radio0",
                                 icon = "images/wui/fieldaction/menu_tab_buildsmall.png",
                                 tooltip = "Tooltip for radio0",
                              },
                              {
                                 name = "radio1",
                                 icon = "images/wui/fieldaction/menu_tab_buildmedium.png",
                                 tooltip = "Tooltip for radio1",
                              },
                              {
                                 name = "radio2",
                                 icon = "images/wui/fieldaction/menu_tab_buildbig.png",
                              },
                              {
                                 name = "radio3",
                                 icon = "images/wui/fieldaction/menu_tab_buildmine.png",
                              },
                              {
                                 name = "radio4",
                                 icon = "images/wui/fieldaction/menu_tab_buildport.png",
                              },
                           }
                        },
                        {
                           widget = "space",
                           value  = 10,
                        },
                        {
                           widget = "multilineeditbox",
                           name   = "mledit",
                           text   = "Enter many lines of text here",
                           tooltip = "Tooltip for multilineeditbox",
                           warning = true,
                           w = 200,
                           h = 100,
                           resizing = "expandboth",
                        },
                        {
                           widget = "space",
                           value  = 10,
                        },
                        {
                           widget = "listselect",
                           name   = "listselect",
                           datatype = "string",
                           h = 100,
                           resizing = "expandboth",
                           tooltip = "Tooltip for listselect",
                           entries = {
                              {
                                 value = "first",
                                 label = "First Item",
                                 tooltip = "Tooltip for i1",
                                 hotkey = "quicknav_gui",
                              },
                              {
                                 value = "second",
                                 label = "2nd Item",
                                 tooltip = "Tooltip for i2",
                                 enable = false,
                              },
                              {
                                 value = "third",
                                 label = "3. Item",
                                 tooltip = "Tooltip for i3",
                              },
                           },
                        },
                     }
                  },
                  {
                     widget = "space",
                     value = 10,
                  },
                  {
                     widget = "slider",
                     orientation = "vert",
                     name = "vslider",
                     tooltip = "Tooltip for vslider",
                     min    = 64,
                     max    = 512,
                     value  = 128,
                     dark   = true,
                     resizing = "expandboth",
                     w = 30,
                     h = 100,
                  },
                  {
                     widget = "space",
                     value = 30,
                  },
                  {
                     widget      = "box",
                     orientation = "vert",
                     resizing    = "expandboth",
                     name        = "vbox2",
                     tooltip = "Tooltip for vbox2",
                     children    = {
                        {
                           widget = "spinbox",
                           name = "spinner",
                           unit_w = 300,
                           label  = "Spin box:",
                           units  = "percent",
                           min    = 0,
                           max    = 100,
                           value  = 15,
                           step_size_big = 25,
                           replacements = {
                              {
                                 value = 1,
                                 replacement = "one"
                              },
                              {
                                 value = 50,
                                 replacement = "fifty"
                              }
                           },
                           tooltip = "Tooltip for spinbox",
                           resizing = "fullsize",
                        },
                        {
                           widget = "inf_space",
                        },
                        {
                           widget = "discrete_slider",
                           name = "dslider",
                           tooltip = "Tooltip for discrete slider",
                           labels = {
                              "Zero",
                              "One",
                              "Two",
                              "Three",
                              "Four",
                              "Five",
                           },
                           value  = 3,
                           dark   = false,
                           resizing = "fullsize",
                           h = 40,
                        },
                        {
                           widget = "inf_space",
                        },
                        {
                           widget = "tabpanel",
                           name = "tabpanel",
                           resizing = "fullsize",
                           tooltip = "Tooltip for tabpanel",
                           active = "tab_table",
                           tabs = {
                              {
                                 name = "tab_edit",
                                 title = "Edit Box",
                                 panel = {
                                    widget = "editbox",
                                    name   = "sledit",
                                    text   = "Enter text here",
                                    password = true,
                                    resizing = "fullsize",
                                    tooltip = "Tooltip for editbox",
                                 },
                              },
                              {
                                 name = "tab_dd",
                                 title = "Dropdown",
                                 panel = {
                                    widget = "dropdown",
                                    name   = "dropdown",
                                    datatype = "string",
                                    label = "Select something",
                                    tooltip = "Tooltip for dropdown",
                                    max_list_items = 8,
                                    button_dimension = 24,
                                    button_style = "secondary",
                                    type = "textual",
                                    resizing = "fullsize",
                                    entries = {
                                       {
                                          value = "first",
                                          label = "First Item",
                                          tooltip = "Tooltip for i1",
                                       },
                                       {
                                          value = "second",
                                          label = "2nd Item",
                                          tooltip = "Tooltip for i2",
                                       },
                                       {
                                          value = "third",
                                          label = "3. Item",
                                          tooltip = "Tooltip for i3",
                                          hotkey = "game_showhide_census",
                                       },
                                    },
                                    on_selected = [=[ dropdown_changed() ]=]
                                 },
                              },
                              {
                                 name = "tab_table",
                                 title = "Table",
                                 panel = {
                                    widget = "table",
                                    name   = "table",
                                    datatype = "int",
                                    resizing = "fullsize",
                                    tooltip = "Tooltip for table",
                                    multiselect = true,
                                    h = 100,
                                    columns = {
                                       {
                                          title = "Resizing Column",
                                          w = 10,
                                          align = "left",
                                          flexible = true,
                                       },
                                       {
                                          title = "Center",
                                          w = 100,
                                       },
                                       {
                                          title = "F",
                                          w = 50,
                                          align = "left",
                                       },
                                    },
                                    rows = {
                                       {
                                          value = 1,
                                          text_0 = "One",
                                          text_1 = "1",
                                          icon_2 = "images/ui_basic/caret_wui.png",
                                       },
                                       {
                                          value = 5,
                                          text_0 = "Five",
                                          text_1 = "5",
                                          text_2 = "V",
                                       },
                                       {
                                          value = 10,
                                          text_0 = "Ten",
                                          text_1 = "10",
                                          text_2 = "X",
                                       },
                                       {
                                          value = 50,
                                          text_0 = "Fifty",
                                          text_1 = "50",
                                          text_2 = "L",
                                       },
                                       {
                                          value = 100,
                                          text_0 = "One Hundred",
                                          text_1 = "100",
                                          text_2 = "C",
                                       },
                                       {
                                          value = 500,
                                          text_0 = "Five Hundred",
                                          text_1 = "500",
                                          text_2 = "D",
                                       },
                                       {
                                          value = 1000,
                                          text_0 = "One Thousand",
                                          text_1 = "1000",
                                          text_2 = "M",
                                       },
                                    },
                                 },
                              },
                           },
                        },
                     }
                  },
                  {
                     widget = "space",
                     value = 10,
                  },
                  {
                     widget = "multilinetextarea",
                     name = "mltxt",
                     w = 300,
                     text = "",
                     scroll_mode = "normal",
                     resizing = "expandboth",
                     tooltip = "Tooltip for multilinetextarea",
                  },
               },
            },
            {
               widget = "progressbar",
               name = "progress",
               orientation = "horz",
               total = 100,
               state = 0,
               show_percent = false,
               h = 20,
               resizing = "fullsize",
               tooltip = "Tooltip for progressbar",
            },
         }
      }
   })

   sleep(500)

   local window = wl.ui.MapView():get_child("uipluginstestwindow")
   assert_not_nil(window, "window was not created")

   local multilinetextarea = window:get_child("mltxt")
   local progressbar       = window:get_child("progress")
   local sleditbox         = window:get_child("sledit")
   local mleditbox         = window:get_child("mledit")
   local spinbox           = window:get_child("spinner")
   local vslider           = window:get_child("vslider")
   local dslider           = window:get_child("dslider")
   local checkbox          = window:get_child("checkbox")
   local button            = window:get_child("button")
   local radiogroup        = window:get_child("radio0")
   local dropdown          = window:get_child("dropdown")
   local listselect        = window:get_child("listselect")
   local tabpanel          = window:get_child("tabpanel")
   local table             = window:get_child("table")
   assert_not_nil(multilinetextarea, "mltxt was not created")
   assert_not_nil(progressbar, "progress was not created")
   assert_not_nil(sleditbox, "sledit was not created")
   assert_not_nil(mleditbox, "mledit was not created")
   assert_not_nil(spinbox, "spinner was not created")
   assert_not_nil(vslider, "vslider was not created")
   assert_not_nil(dslider, "dslider was not created")
   assert_not_nil(checkbox, "checkbox was not created")
   assert_not_nil(button, "button was not created")
   assert_not_nil(radiogroup, "radio0 was not created")
   assert_not_nil(dropdown, "dropdown was not created")
   assert_not_nil(listselect, "listselect was not created")
   assert_not_nil(tabpanel, "tabpanel was not created")
   assert_not_nil(table, "table was not created")
   assert_nil(window:get_child("foobarbaz"), "Child should not exist but does")

   assert_equal("UI Plugins Test", window.title)
   assert_equal(100, progressbar.total)
   assert_equal(true, sleditbox.password)
   assert_equal("Enter many lines of text here", mleditbox.text)
   assert_equal(15, spinbox.value)
   assert_equal(64, vslider.min_value)
   assert_equal(3, dslider.value)
   assert_equal(true, checkbox.state)
   assert_equal(true, button.enabled)
   assert_equal(3, radiogroup.state)
   assert_equal(3, dropdown.no_of_items)
   assert_equal("string", listselect.datatype)
   assert_equal(2, tabpanel.active)
   assert_equal(7, table.no_of_rows)

   window.title = "Changed title"
   progressbar.total = 50
   sleditbox.password = false
   mleditbox.text = "Updated text"
   spinbox.value = 17
   vslider.min_value = 20
   dslider.value = 0
   checkbox.state = false
   button.enabled = false
   radiogroup.state = 2
   dropdown:add("New Item", "fourth")
   listselect:add("New Item", "fourth", nil, "", true, 2, true, "game_msg_filter_seafaring")
   tabpanel.active = 1
   table:add(1234, false, true, {{text = "Invalid"}, {text = "1234"}, {icon = "images/ui_basic/menu_help.png"}})

   sleep(500)

   assert_equal("Changed title", window.title)
   assert_equal(50, progressbar.total)
   assert_equal(false, sleditbox.password)
   assert_equal("Updated text", mleditbox.text)
   assert_equal(17, spinbox.value)
   assert_equal(20, vslider.min_value)
   assert_equal(0, dslider.value)
   assert_equal(false, checkbox.state)
   assert_equal(false, button.enabled)
   assert_equal(2, radiogroup.state)
   assert_equal(4, dropdown.no_of_items)
   assert_equal("listselect", listselect.name)
   assert_equal(1, tabpanel.active)
   assert_equal(8, table.no_of_rows)

   assert_equal("first", dropdown:get_value_at(1))
   assert_equal("2nd Item", dropdown:get_label_at(2))
   assert_equal("Tooltip for i3", dropdown:get_tooltip_at(3))
   assert_equal("second", listselect:get_value_at(2))
   assert_equal("3. Item", listselect:get_label_at(3))
   assert_equal("Tooltip for i1", listselect:get_tooltip_at(1))
   assert_equal(0, listselect:get_indent_at(1))
   assert_equal(false, listselect:get_enable_at(2))
   assert_equal(true, listselect:get_enable_at(3))

   button.enabled = true
   sleep(1000)

   dropdown:open()
   dropdown:highlight_item(2)
   dropdown:select()

end)

function dropdown_changed()
   local window = wl.ui.MapView():get_child("uipluginstestwindow")

   assert_equal("third", window:get_child("dropdown").selection)

   window:get_child("button"):click()
end

function button_clicked()
   print("# All Tests passed.")
   wl.ui.MapView():close()
end
