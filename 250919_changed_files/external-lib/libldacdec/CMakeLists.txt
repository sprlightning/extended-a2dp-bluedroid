cmake_minimum_required(VERSION 3.15)

idf_component_register(SRCS src/bit_allocation.c
                            src/bit_reader.c
							src/huffCodes.c
							src/imdct.c
							src/libldacdec.c
							src/spectrum.c
							src/utility.c
                       INCLUDE_DIRS inc include
                       PRIV_INCLUDE_DIRS inc src)

target_compile_options(${COMPONENT_LIB} PRIVATE -Werror=implicit-function-declaration)

