# based on http://protips.readthedocs.io/link-roles.html

from docutils import nodes
#from repo_util import run_cmd_get_output

#def get_github_rev():
#    path = run_cmd_get_output('git rev-parse --short HEAD')
#    tag  = run_cmd_get_output('git describe --exact-match')
#    print 'Git commit ID: ', path
#    if len(tag):
#        print 'Git tag: ', tag
#        path = tag
#    return path

def setup(app):
    baseurl = 'https://github.com/jeras/readthedocs-source-links'
    #rev = get_github_rev()
    rev = 'master'
    app.add_role('source', autolink('{}/blob/{}/%s'.format(baseurl, rev)))

def autolink(pattern):
    def role(name, rawtext, text, lineno, inliner, options={}, content=[]):
        url = pattern % (text,)
        node = nodes.reference(rawtext, text, refuri=url, **options)
        return [node], []
    return role
