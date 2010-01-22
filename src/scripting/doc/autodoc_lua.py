#!/usr/bin/env python
# encoding: utf-8

from docutils import nodes
from sphinx.util.compat import Directive, make_admonition

class todo(nodes.Admonition, nodes.Element):
    pass

class todolist(nodes.General, nodes.Element):
    pass

def visit_todo_node(self, node):
    self.visit_admonition(node)

def depart_todo_node(self, node):
    self.depart_admonition(node)

class TodolistDirective(Directive):

    def run(self):
        return [todolist('')]

class TodoDirective(Directive):

    has_content = True

    def run(self):
        env = self.state.document.settings.env

        targetid = "todo-%s" % env.index_num
        env.index_num += 1

        targetnode = nodes.target('','', ids=[targetid])

        ad = make_admonition(todo, self.name, [_('TODO')], self.options,
                self.content, self.lineno, self.content_offset,
                self.block_text, self.state, self.state_machine)

        if not hasattr(env, 'todo_all_todos'):
            env.todo_all_todos = []

        env.todo_all_todos.append({
            'docname': env.docname,
            'lineno': self.lineno,
            'todo': ad[0].deepcopy(),
            'target': targetnode
        })

        return [targetnode] + ad

def purge_todos(app, env, docname):
    if not hasattr(env, 'todo_all_todos'):
        return
    env.todo_all_todos = [ t for t in env.todo_all_todos if
                          t['docname'] != docname ]

def process_todo_nodes(app, doctree, fromdocname):

    env = app.builder.env

    for node in doctree.traverse(todolist):
        content= []
        for todo_info in env.todo_all_todos:
            para = nodes.paragraph()
            filename = env.doc2path(todo_info['docname'], base = None)

            description = (
                '(The original entry is located in %s:%i and can be found ' %
                (filename, todo_info['lineno']))
            para += nodes.Text(description, description)

            newnode = nodes.reference('','')
            innernode = nodes.emphasis(_('here'), _('here'))
            newnode['refdocname'] = todo_info['docname']
            newnode['refuri'] = app.builder.get_relative_uri(
                fromdocname, todo_info['docname'])
            newnode['refuri'] += '#' + todo_info['target']['refid']
            newnode.append(innernode)
            para += newnode
            para += nodes.Text('.)', '.)')

            content.append(todo_info['todo'])
            content.append(para)

        node.replace_self(content)

def setup(app):
    app.add_node(todolist)
    app.add_node(todo,
        html=(visit_todo_node, depart_todo_node),
        latex=(visit_todo_node, depart_todo_node),
        text=(visit_todo_node, depart_todo_node)
    )

    app.add_directive('todo', TodoDirective)
    app.add_directive('todolist', TodolistDirective)

    app.connect('doctree-resolved', process_todo_nodes)
    app.connect('env-purge-doc', purge_todos)
