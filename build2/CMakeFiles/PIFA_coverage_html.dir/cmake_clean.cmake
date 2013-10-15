FILE(REMOVE_RECURSE
  "CMakeFiles/PIFA_coverage_html"
  "PIFA_coveragereport"
  "PIFA_coverage.info"
  "PIFA_coverage.raw.info"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/PIFA_coverage_html.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
