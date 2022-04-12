#include "editor/tools/tool_conf.h"

#include "editor/editorinteractive.h"
#include "editor/tools/tool.h"

ToolConf::ToolConf()
  : tool_id(static_cast<ToolID>(0)),
    sel_radius(0),
    change_by(0),
    set_to(0),
    interval(0, 0) {
}

