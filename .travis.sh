set -ex

# Create build folder.
mkdir build
cd build

case "$1" in
build)
   if [ "$TRAVIS_OS_NAME" = linux ]; then
      cmake .. -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DOPTION_BUILD_TRANSLATIONS=$BUILD_TRANSLATIONS -DOPTION_BUILD_WEBSITE_TOOLS=$BUILD_WEBSITE_TOOLS -DOPTION_ASAN="OFF" -DOPTION_BUILD_CODECHECK="OFF"
   else
      # gettext requires special treatment on MacOS
      cmake .. -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DOPTION_BUILD_TRANSLATIONS=$BUILD_TRANSLATIONS -DOPTION_BUILD_WEBSITE_TOOLS=$BUILD_WEBSITE_TOOLS -DOPTION_ASAN="OFF" -DOPTION_BUILD_CODECHECK="OFF" -DCMAKE_EXE_LINKER_FLAGS="-L/usr/local/opt/gettext/lib -lintl" -DCMAKE_CXX_FLAGS="-I/usr/local/opt/gettext/include"
   fi
   # Do the actual build.
   make -k -j3

   # Run the regression suite only if compiling didn't take too long (to avoid timeouts).
   # On macOS it always fails with a broken GL installation message, so we omit it.
   if [ "$TRAVIS_OS_NAME" = linux ]; then
      cd ..
      ./regression_test.py -b build/src/widelands
      if [ "$BUILD_WEBSITE_TOOLS" = ON ]; then
         mkdir temp_web
         build/src/website/wl_map_object_info temp_web
         build/src/website/wl_map_info data/maps/Archipelago_Sea.wmf
      fi
   fi
   ;;
codecheck)
   cmake .. -DCMAKE_BUILD_TYPE:STRING="Debug"
   # Run the codecheck test suite.
   pushd ../cmake/codecheck
   ./run_tests.py
   popd

   # Any codecheck warning is an error. Keep the codebase clean!!
   # Suppress color output.
   TERM=dumb make -j1 codecheck 2>&1 | tee codecheck.out
   if grep '^[/_.a-zA-Z]\+:[0-9]\+:' codecheck.out; then
      echo "You have codecheck warnings (see above) Please fix."
      exit 1 # CodeCheck warnings.
   fi
   ;;
documentation)
   # Any warning is an error.
   pushd ../doc/sphinx
   mkdir source/_static
   ./extract_rst.py
   sphinx-build -W -b json -d build/doctrees source build/json
   popd
   ;;
includes)
   # Check for superfluous includes and forward declarations
   pushd ../src
   ../utils/find_unused_includes.py
   popd
   ;;
clang-tidy)
   # Check for clang-tidy warnings that were cleaned up previously.
   # We only check for missing optional braces at this point.
   # We can add more checks later when we have cleaned up more.
   cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
   python ../utils/run-clang-tidy.py -checks=-*,*braces*,cert*,clang-analyzer*,cppcoreguidelines-pro-type-static-cast-downcast,google-readability-casting,performance* > ../clang-tidy.log
   pushd ..
   utils/check_clang_tidy_results.py clang-tidy.log
   pushd build
   ;;
esac
