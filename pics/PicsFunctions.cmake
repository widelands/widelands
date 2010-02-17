file (GLOB_RECURSE PNG_LIST *.png)
file (GLOB_RECURSE JPG_LIST *.jpg)

set (WL_TOOLS_PNG_FOUND false)

if (WIN32 OR APPLE)
  message ("The \"pics\" target has not yet been tested on Windows (and Darwin), only on Linux. It is highly likely that find_program does not find optipng and/or pngrewrite. You are welcome to test and correct it in pics/PicsFunctions.cmake or report back.")
endif (WIN32 OR APPLE)

find_program (WL_TOOLS_PNGREWRITE pngrewrite)
if (NOT WL_TOOLS_PNGREWRITE STREQUAL "WL_TOOLS_PNGREWRITE-NOTFOUND")
  message("pngrewrite is ${WL_TOOLS_PNGREWRITE}")
  set (WL_TOOLS_PNG_FOUND true)
else (NOT WL_TOOLS_PNGREWRITE STREQUAL "WL_TOOLS_PNGREWRITE-NOTFOUND")
  message ("pngrewrite not found")
endif (NOT WL_TOOLS_PNGREWRITE STREQUAL "WL_TOOLS_PNGREWRITE-NOTFOUND")

find_program (WL_TOOLS_OPTIPNG optipng)
if (NOT WL_TOOLS_OPTIPNG STREQUAL "WL_TOOLS_OPTIPNG-NOTFOUND")
  message("optipng is ${WL_TOOLS_OPTIPNG}")
  set (WL_TOOLS_PNG_FOUND true)
else (NOT WL_TOOLS_OPTIPNG STREQUAL "WL_TOOLS_OPTIPNG-NOTFOUND")
  message ("optipng not found")
endif (NOT WL_TOOLS_OPTIPNG STREQUAL "WL_TOOLS_OPTIPNG-NOTFOUND")

foreach (jpg_tmp ${JPG_LIST})
  get_filename_component(jpg_tmp_filename ${jpg_tmp} NAME)
  execute_process (RESULT_VARIABLE copyret COMMAND cmake -E copy ${jpg_tmp} ${WL_CURRENT_BINARY_DIR}/${jpg_tmp_filename})
endforeach (jpg_tmp ${JPG_LIST})

foreach (png_tmp ${PNG_LIST})
  get_filename_component(png_tmp_filename ${png_tmp} NAME)
  execute_process (RESULT_VARIABLE copyret COMMAND cmake -E copy ${png_tmp} ${WL_CURRENT_BINARY_DIR}/${png_tmp_filename})
  if (WL_TOOLS_PNG_FOUND)
    if (NOT WL_TOOLS_PNGREWRITE STREQUAL "WL_TOOLS_PNGREWRITE-NOTFOUND")
      execute_process (COMMAND ${WL_TOOLS_PNGREWRITE} ${WL_CURRENT_BINARY_DIR}/${png_tmp_filename} ${WL_CURRENT_BINARY_DIR}/${png_tmp_filename})
    endif (NOT WL_TOOLS_PNGREWRITE STREQUAL "WL_TOOLS_PNGREWRITE-NOTFOUND")

    if (NOT WL_TOOLS_OPTIPNG STREQUAL "WL_TOOLS_OPTIPNG-NOTFOUND")
      execute_process (COMMAND ${WL_TOOLS_OPTIPNG} -q -zc1-9 -zm1-9 -zs0-3 -f0-5 ${WL_CURRENT_BINARY_DIR}/${png_tmp_filename})
    endif (NOT WL_TOOLS_OPTIPNG STREQUAL "WL_TOOLS_OPTIPNG-NOTFOUND")
  endif (WL_TOOLS_PNG_FOUND)
endforeach (png_tmp ${PNG_LIST})

if (NOT WL_TOOLS_PNG_FOUND)
  message(FATAL_ERROR "Neither pngrewrite nor optipng installed, failing")
endif (WL_TOOLS_PNG_FOUND)

