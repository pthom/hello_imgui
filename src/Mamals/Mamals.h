#pragma once

namespace Mamals
{
/**
 * list of possible Mamals
 */
enum MamalId
{
  Dog,
  Cow,
  Cat,
  Rabbit
};

/**
 * Parent class of the Mamals
 */
class IMamal
{
public:
  virtual ~IMamal() = default;
  virtual MamalId GetMamalId() = 0;
};

}