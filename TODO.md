## Features
    * [N] Font size according to platform (mobile)
    * [ ] Docking / External windows ?
     
## iOS
* [X] Classer assets par repertoire
* [X] Info.plist pour SDL (avec rep SDL)
    Voir best practices dans https://github.com/forexample/testapp/blob/master/CMakeLists.txt
    https://developer.apple.com/library/archive/documentation/General/Reference/InfoPlistKeyReference/Articles/AboutInformationPropertyListFiles.html#//apple_ref/doc/uid/TP40009254-SW1
    Set icon ?
* [X] fontes dans bundle
* [X] Arnaque / SDL_Main ?
* [X] Submodule cmake-ios
* [X] Full screen raté
    See https://stackoverflow.com/questions/56212017/how-do-i-request-a-fullscreen-window-on-ios-using-sdl2
* [X] Set app Icon name
    CFBundleDisplayName -> le rendre configurable ?
* [X] Bug / product name...
* [X] Pas possible quitter ?
* [X] voir callbacks
      https://wiki.libsdl.org/SDL_EventType#Android.2C_iOS_and_WinRT_Events
* [X] CI Github
* [X] Handle pause/Resume / Add ifdef
* [X] transmit sdl events ?
* [ ] Doc / Build (+script ?)
* [ ] Doc / Custom icon and Info.plist (+ create custom icons !)
* [ ] Doc / callback suspend

## Emscripten
 * [ ] Doc: 
    tools/install_emscripten.sh
    emcmake cmake .. -DHELLOIMGUI_USE_SDL_OPENGL3=ON
 * [ ] CI with docker: see https://github.com/marketplace/actions/setup-emscripten
 * [ ] Link demo online!
 
## Doc

* [ ] Doc for new projet / add hello-imgui as submodule
* [ ] Build android
* [ ] Build iOS

## Test run + CI

* [ ] Win / OsX
* [ ] Android    
* [ ] IOS
See https://github.com/jurplel/install-qt-action
    
## qtimgui
* [ ] Issue qtimgui / Win
    