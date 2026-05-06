build in powershell: 
cmake -B build -G "Visual Studio 18 2026" -A x64 -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release

TODO:

-add cleaner png of the knob

-add a direction to the knob (left right or right left)                             OK
-add shadow to led outline                                                          OK
-add glow to led outline                                                            OK
-add outline to led outline                                                         OK
-when leds are off hide the green and the glow but show the outline of led outline  OK
-add text to knob                                                                   OK

-add vu meter
-add custom vu metter class with the figma design

-add output volume knob with apvts
