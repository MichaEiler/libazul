include(GenerateExportHeader)

function (libazul_generate_export_header component_name)
    generate_export_header(azul_${component_name} EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/exports/include/azul/${component_name}/export.hpp")
    target_include_directories(azul_${component_name} PUBLIC ${CMAKE_BINARY_DIRECTORY}/exports/include)
    include_directories(${CMAKE_BINARY_DIR}/exports/include/)
    install(FILES ${CMAKE_BINARY_DIR}/exports/include/azul/${component_name}/export.hpp DESTINATION include/azul/${component_name}/)
endfunction()
