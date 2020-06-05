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
#ifndef CAT_SOUNDS_MEOW_HPP
#define CAT_SOUNDS_MEOW_HPP

#include <string>

#pragma once
#include <string>
#include "Mamals/Mamals.h"

namespace Mamals
{
namespace Felinae
{

/**
 * A Cat meows
 */
class Cat: public IMamal
{
public:
  std::string meow();
  inline MamalId GetMamalId() { return MamalId::Cat; };
};
}

}

#endif
