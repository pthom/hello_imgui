# CPP project template

A skeleton for cmake-based c++ projects.

Build status: ![CI](https://github.com/KaiSzuttor/cpp_project_template/workflows/CI/badge.svg)

## Features

* [Github actions](https://help.github.com/en/actions) set up for continious integration
* Uses [doctest](https://github.com/onqtam/doctest) for testing
* Follows the [pitchfork project layout convention](https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs)
* Sample configurations for:
  * [clang-tidy](https://clang.llvm.org/extra/clang-tidy/)
  * [clang-format](https://clang.llvm.org/docs/ClangFormat.html)
* Sample .gitignore file (generated with a useful online tool [gitignore.io](http://gitignore.io))
* License header template in `data/header_template.txt`
* Tools for license header updating: `tools/license_headers.py`, that can be used like e.g. this:
  
  ``` bash
  ./tools/license_headers.py -y 2020 -t data/header_template.txt -o 'Kai Szuttor' -n 'CPP Project Template' -u 'https://github.com/kaiszuttor/cpp_project_template' -x tools
  ```
* CMake support for:
  * [Doxygen](http://www.doxygen.nl)
  * [clang-tidy](https://clang.llvm.org/extra/clang-tidy)
  * [cppcheck](https://github.com/danmar/cppcheck)
  * [ccache](https://ccache.dev)
  
* [Nice and searchable Docs](https://pthom-cpp-project-template.readthedocs.io/en/latest/) 
with a format resembling the [fmtlib](https://fmt.dev/latest/index.html) documentation
   