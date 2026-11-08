#include <windows.h>
#include <stdlib.h>  // __argc, __argv: the command line, parsed by the C runtime


int main(int, char **);


INT WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, PSTR /*lpCmdLine*/, INT /*nCmdShow*/)
{
  int exit_result = main(__argc, __argv);
  return exit_result;
}
