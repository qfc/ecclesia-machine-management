#!/bin/bash
#
# Fix includes of third-party jsoncpp headers.

# include "config.h"
perl -i -pe 's:^#\s*include "(\w+\.h)":#include "third_party/jsoncpp/\1":g' $(find . -iname '*.h' -or -iname '*.cc')

#include "third_party/jsoncpp/reader.h"
perl -i -pe 's:^#\s*include <json/(\w+\.h)>:#include "third_party/jsoncpp/\1":g' $(find . -iname '*.h' -or -iname '*.cc')

# include "json_internalarray.inl"
perl -i -pe 's:^#\s*include "(\w+\.inl)":#include "third_party/jsoncpp/\1.h":g' $(find . -iname '*.h' -or -iname '*.cc')

# include "third_party/jsoncpp/jsontest.h"
perl -i -pe 's:^#\s*include "third_party/jsoncpp/jsontest.h":#include "third_party/jsoncpp/unit_tests/jsontest.h":g' $(find . -iname '*.h' -or -iname '*.cc')
