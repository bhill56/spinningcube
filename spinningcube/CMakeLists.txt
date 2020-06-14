
project(spinningcube VERSION 0.0.1 LANGUAGES CXX)

find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glbinding CONFIG REQUIRED)
find_package(globjects CONFIG REQUIRED)
find_package(cpplocate CONFIG REQUIRED)

add_executable(spin src/helloworld.cpp)
set_property(TARGET spin PROPERTY CXX_STANDARD 17)

target_link_libraries(spin PRIVATE glfw 
    GLEW::GLEW
    glm
    glbinding::glbinding
    globjects::globjects
    cpplocate::cpplocate
    ) 

install(TARGETS spin)
