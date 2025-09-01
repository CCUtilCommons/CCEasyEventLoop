# Collect the Library Sources

set(CenterLoopsCore Centers/Application.cpp Centers/Application.h)
set(EventsCore Events/Signal.hpp)
set(UtilsSrc Utils/CTimer.cpp Utils/CTimer.h)

add_library(CoreLibrary ${CenterLoopsCore} ${EventsCore} ${UtilsSrc})
target_include_directories(CoreLibrary PUBLIC Centers Events Exceptions Utils)


