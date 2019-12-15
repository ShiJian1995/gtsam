set (excluded_examples
    # fileToExclude.cpp
)

# Note: do not use "-" or "." in example project names to avoid MSVC to think
# names are duplicated.
gtsamAddExamplesGlob("*.cpp" "${excluded_examples}" "gtsam_unstable" "gtsam_unstable_discrete")
