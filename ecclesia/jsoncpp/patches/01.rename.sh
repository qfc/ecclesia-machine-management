#!/bin/bash
#
# Move jsoncpp files from their locations in the Git repository to their
# locations in //third_party/jsoncpp.
#
# This script assumes that you're using git5 and have the src/, include/, and
# test/ directories from the SVN repo tracked in git in
# google3/third_party/jsoncpp. If you're not using git, simply remove the "git"
# command from each line.
#
# Preparing looks something like this:
#
#     mkdir -p ~/tmp
#     git clone https://github.com/open-source-parsers/jsoncpp ~/tmp/jsoncpp
#     mkdir -p src include test
#     rsync -v -a --progress --delete ~/tmp/jsoncpp/src/ src/
#     rsync -v -a --progress --delete ~/tmp/jsoncpp/include/ include/
#     rsync -v -a --progress --delete ~/tmp/jsoncpp/test/ test/
#     git add src include test
#     git commit -m "tracked"
#

set -x

# Delete existing versions of the files.
git rm -f *.h *.cc || exit 1
git rm golden_tests/*.* || exit 1
git rm -r golden_tests/data || exit 1
git rm unit_tests/*.* || exit 1

# Restore google-specific files.
git reset -- testing.h testing_test.cc
git checkout testing.h testing_test.cc || exit 1

# Pull over files needed for unit testing.
mkdir -p unit_tests
git mv src/test_lib_json/jsontest.h unit_tests/jsontest.h || exit 1
git mv src/test_lib_json/jsontest.cpp unit_tests/jsontest.cc || exit 1
git mv src/test_lib_json/main.cpp unit_tests/jsoncpp_test.cc || exit 1

# Pull over files needed for golden testing.
mkdir -p golden_tests
git mv src/jsontestrunner/main.cpp golden_tests/test_runner.cc || exit 1
git mv test/runjsontests.py golden_tests/golden_test.py || exit 1
git mv test/data golden_tests/data || exit 1

# Delete everything else in test/. Handle *.pyc files specially, since git5
# ignores them by default.
git rm -f test/*.pyc || rm -f test/*.pyc || exit 1
git rm -r test/ || exit 1

# Pull over files we care about from src/.
git mv src/lib_json/*.h ./ || exit 1
git mv src/lib_json/json_reader.cpp json_reader.cc || exit 1
git mv src/lib_json/json_value.cpp json_value.cc || exit 1
git mv src/lib_json/json_valueiterator.inl json_valueiterator.inl.h || exit 1
git mv src/lib_json/json_writer.cpp json_writer.cc || exit 1

# We don't use the rest of src/.
git rm -r src/ || exit 1

# Pull over all the includes from include/.
git mv include/json/*.h ./ || exit 1
git rm -r include/ || exit 1
