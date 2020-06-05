#pragma once
#include <string>
#include "Mamals/Mamals.h"

namespace Mamals
{
namespace Canidae
{

/**
 * A Dog barks
 */
class Dog: public IMamal
{
public:
  /**
   * Let the dog bark!
   * @return: as string with a bark
   */
  std::string bark();

  inline MamalId GetMamalId() { return MamalId::Dog; };
};
}

}
