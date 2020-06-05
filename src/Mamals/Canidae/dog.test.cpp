/*
 * Copyright (c) 2020 Kai Szuttor.
 *
 * This file is part of CPP Project Template
 * (see https://github.com/kaiszuttor/cpp_project_template).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <doctest/doctest.h>
#include <iostream> // toolchain issues on osx: https://github.com/onqtam/doctest/issues/356

#include "Mamals/Canidae/dog.h"

TEST_CASE( "bark" ) {
  using namespace Mamals::Canidae;
  Dog dog;
      REQUIRE( dog.bark() == "Wof" );
}
