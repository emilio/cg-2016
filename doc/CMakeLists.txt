if(NOT EXISTS ${PANDOC})
  find_program(PANDOC pandoc)
  if(NOT EXISTS ${PANDOC})
    message(WARNING "Pandoc not found, no documentation generation then")
    return()
  endif()
endif()

add_custom_target(docs ALL
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/main.pdf"
          "${CMAKE_CURRENT_BINARY_DIR}/surfaces.pdf"
  VERBATIM
)

file(GLOB DOC_FILES "*.md")

foreach(doc_file ${DOC_FILES})
  get_filename_component(doc_pdf_name ${doc_file} NAME_WE)
  add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${doc_pdf_name}.pdf"
    DEPENDS "${doc_file}"
    COMMAND "${PANDOC}" "--from=markdown" "--latex-engine=xelatex" "--to=latex" "${doc_file}" -o "${doc_pdf_name}.pdf"
  )
endforeach()
