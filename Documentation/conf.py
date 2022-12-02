# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'vAmiga'
copyright = '2022, Dirk W. Hoffmann'
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

#html_theme = 'karma_sphinx_theme'
#html_theme = 'sphinx_rtd_theme'
html_theme = 'sphinx_book_theme'
html_theme_options = {
    "repository_url": "https://dirkwhoffmann.github.io/vAmiga/",
    "use_repository_button": True,
    "logo_only": True,
}
html_logo = "images/icon.png"
html_title = "vAmiga 2.2"
html_static_path = ["_static"]
html_css_files = ["custom.css"]