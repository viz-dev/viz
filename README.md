Viz Core integration/staging tree
=====================================

https://viz.wiki

What is Viz?
----------------

Viz is an experimental digital currency that allows instant payments through
a peer-to-peer network as well as through hidden messages in image files. The
VIZ browser extension uncovers a second layer of the internet hidden from plain
sight. The web is a treasure hunt and VIZ is your map.

Viz Core is the name of open source software which enables the use of this currency.

For more information, as well as an immediately useable, binary version of
the Viz Core software, see [https://viz.wiki/](https://viz.wiki/).

License
-------

Viz Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/viz-project/viz/tags) are created
regularly to indicate new official, stable release versions of Viz Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

Developer IRC can be found on Freenode at #viz-dev.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

We only accept translation fixes that are submitted through [Bitcoin Core's Transifex page](https://www.transifex.com/projects/p/bitcoin/).
Translations are converted to Viz periodically.

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.
