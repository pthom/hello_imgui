Welcome to Cpp Project Template's documentation!
================================================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

:ref:`genindex`

About this doc
==============

This documentation was generated using

- Doxygen : automatic doc extraction from the code
- breathe + bridge between the Sphinx and Doxygen
- Sphinx: advanced documentation system
- ReadTheDocs: nice interface (and onlne host) for the doc

Just build the target "Sphinx" in order to generate it!


It is based on `an article by Sy Brand <https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/>`_

Requirements:

- doxygen ( `sudo apt-get install doxygen`)
- sphinx ( `sudo pip3 install sphinx`)
- breathe ( `sudo pip3 install breathe`)
- sphinx read the docs thme ( `sudo pip3 install sphinx_rtd_theme`)


APIS
====


Mamals base interface
----------------------

In the namespace Mamals, we have a ``MamalId`` enum:

.. doxygenenum:: Mamals::MamalId

And a base class ``IMamal``:

.. doxygenclass:: Mamals::IMamal
   :members:


Mamals::Canidae namespace
-------------------------

.. doxygennamespace:: Mamals::Canidae
   :members:
   :undoc-members:


Mamals::Felinae namespace
-------------------------

.. doxygennamespace:: Mamals::Felinae
   :members:
   :undoc-members:

