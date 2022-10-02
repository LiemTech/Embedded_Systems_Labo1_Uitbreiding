cmake_minimum_required(VERSION 3.10)

# set the project name
project(Labo1)

include_directories(/usr/include/mariadb /usr/include/mariadb/mysql)
add_executable(Labo1 labo1.c)

target_link_libraries(Labo1 mariadb)

install (FILES labo1.h DESTINATION include)

#install (TARGETS Labo1 DESTINATION lib)