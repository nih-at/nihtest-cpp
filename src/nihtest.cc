/*
  nihtest.cc -- main program
  Copyright (C) 2020 Dieter Baron and Thomas Klausner

  This file is part of nihtest, regression tests for command line utilities.
  The authors can be contacted at <nihtest@nih.at>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "nihtest.h"

#include <iostream>
#include <string>

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#else
#include "getopt_long.h"
#endif

#include "Exception.h"
#include "Test.h"
#include "Variables.h"

static const std::string usage_tail = " [-hqVv] [--keep-broken] [--no-cleanup] [--setup-only] [VARIABLE=VALUE ...] testcase\n";

static const std::string help_head = PACKAGE " by Dieter Baron and Thomas Klausner\n\n";

static const std::string version_string = PACKAGE " " VERSION "\n"
    "Copyright (C) 2020 Dieter Baron and Thomas Klausner\n"
    PACKAGE " comes with ABSOLUTELY NO WARRANTY, to the extent permitted by law.\n";

static const std::string help_tail = "\n"
    "  -h, --help         display this help message and exit\n"
    "      --keep-broken  keep sandbox if test fails\n"
    "      --no-cleanup   keep sandbox\n"
    "  -q, --quiet        don't print test results\n"
    "      --setup-only   set up sandbox, but don't run test\n"
    "  -v, --verbose      print detailed test results\n"
    "  -V, --version      display version number and exit\n";

enum {
    OPT_KEEP_BROKEN = 256,
    OPT_NO_CLEANUP,
    OPT_SETUP_ONLY
};

#define OPTIONS "hVv"

struct option options[] = {
    { "help", 0, 0, 'h' },
    { "version", 0, 0, 'V' },
    
    { "keep-broken", 0, 0, OPT_KEEP_BROKEN },
    { "no-cleanup", 0, 0, OPT_NO_CLEANUP },
    { "quiet", 0, 0, 'q' },
    { "setup-only", 0, 0, OPT_SETUP_ONLY },
    { "verbose", 0, 0, 'v' }
};
    

static void print_usage(std::ostream &stream);

int main(int argc, char *argv[]) {
    int c;
    auto variables = Variables(true);
    auto test = Test();
    
    setprogname(argv[0]);
    
    opterr = 0;
    while ((c = getopt_long(argc, argv, OPTIONS, options, 0)) != EOF) {
        switch (c) {
        case 'h':
            std::cout << help_head;
            print_usage(std::cout);
            std::cout << help_tail;
            exit(0);
            
        case 'V':
            std::cout << version_string;
            exit(0);
            
        case 'q': // quiet
            test.print_results = Test::NEVER;
            break;
            
        case 'v': // verbose
            test.print_results = Test::ALWAYS;
            break;
            
        case OPT_KEEP_BROKEN:
            if (test.keep_sandbox != Test::ALWAYS) {
                test.keep_sandbox = Test::WHEN_BROKEN;
            }
            break;
            
        case OPT_NO_CLEANUP:
            test.keep_sandbox = Test::ALWAYS;
            break;
            
        case OPT_SETUP_ONLY:
            test.keep_sandbox = Test::ALWAYS;
            test.run_test = false;
            break;
            
        default:
            print_usage(std::cerr);
            exit(1);
        }
    }
    
    while (optind < argc && strchr(argv[optind], '=') != NULL) {
        variables.add(argv[optind]);
        optind += 1;
    }
    
    if (optind != argc - 1) {
        print_usage(std::cerr);
        exit(1);
    }
    
    if (test.print_results != Test::NEVER && variables.is_set("VERBOSE")) {
        test.print_results = Test::ALWAYS;
    }
    if (test.keep_sandbox == Test::NEVER && variables.is_set("KEEP_BROKEN")) {
        test.print_results = Test::WHEN_BROKEN;
    }
    
    try {
        test.initialize(argv[optind], variables);
        exit(test.run());
    }
    catch (Exception e) {
        std::cerr << getprogname() << ": " << e.what() << "\n";
        exit(1);
    }
}

static void print_usage(std::ostream &stream) {
    stream << "Usage: " << getprogname() << usage_tail;
}
