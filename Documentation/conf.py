# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'vAmiga'
copyright = '2023, Dirk W. Hoffmann'
author = 'Dirk W. Hoffmann'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.githubpages',
    'myst_parser'
]


templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']
source_parsers = {
    '.md': 'recommonmark.parser.CommonMarkParser',
}
source_suffix = ['.rst', '.md']

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'insipid'
html_theme_options = {
    'initial_sidebar_visibility_threshold': 0,
    'breadcrumbs': False,
}
#html_sidebars = {
#   '**': ['globaltoc.html', 'sourcelink.html', 'searchbox.html'],
#   'using/windows': ['windowssidebar.html', 'searchbox.html'],
#}
html_logo = "images/icon.png"
html_title = "vAmiga 2.4"
#html_static_path = ["_static"]
#html_css_files = ["custom.css"]