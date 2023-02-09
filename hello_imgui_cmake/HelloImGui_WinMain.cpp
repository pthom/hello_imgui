#include <windows.h>


int main(int, char **);


INT WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, PSTR /*lpCmdLine*/, INT /*nCmdShow*/)
{
  int exit_result = main(0, NULL);
  return exit_result;
}
