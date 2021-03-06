// Copyright 2010 Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// * Neither the name of Google Inc. nor the names of its contributors
//   may be used to endorse or promote products derived from this software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "cli/cmd_about.hpp"

#include <cstdlib>
#include <fstream>
#include <utility>
#include <vector>

#include "cli/common.ipp"
#include "utils/cmdline/exceptions.hpp"
#include "utils/cmdline/parser.ipp"
#include "utils/cmdline/ui.hpp"
#include "utils/defs.hpp"
#include "utils/env.hpp"
#include "utils/format/macros.hpp"
#include "utils/fs/path.hpp"
#include "utils/sanity.hpp"

#if defined(HAVE_CONFIG_H)
#   include "config.h"
#endif

namespace cmdline = utils::cmdline;
namespace config = utils::config;
namespace fs = utils::fs;

using cli::cmd_about;


namespace {


/// Print the contents of a document.
///
/// If the file cannot be opened for whatever reason, an error message is
/// printed to the output of the program instead of the contents of the file.
///
/// \param ui Object to interact with the I/O of the program.
/// \param file The file to print.
///
/// \return True if the file was printed, false otherwise.
static bool
cat_file(cmdline::ui* ui, const fs::path& file)
{
    std::ifstream input(file.c_str());
    if (!input) {
        ui->err(F("Failed to open %s") % file);
        return false;
    }

    std::string line;
    while (std::getline(input, line).good())
        ui->out(line);
    input.close();
    return true;
}


}  // anonymous namespace


/// Default constructor for cmd_about.
cmd_about::cmd_about(void) : cli_command(
    "about", "[authors|license|version]", 0, 1,
    "Shows general program information")
{
}


/// Entry point for the "about" subcommand.
///
/// \param ui Object to interact with the I/O of the program.
/// \param cmdline Representation of the command line to the subcommand.
/// \param unused_user_config The runtime configuration of the program.
///
/// \return 0 if everything is OK, 1 if any of the necessary documents cannot be
/// opened.
int
cmd_about::run(cmdline::ui* ui, const cmdline::parsed_cmdline& cmdline,
               const config::tree& UTILS_UNUSED_PARAM(user_config))
{
    const fs::path docdir(utils::getenv_with_default(
        "KYUA_DOCDIR", KYUA_DOCDIR));

    bool success = true;

    if (cmdline.arguments().empty()) {
        ui->out(PACKAGE " (" PACKAGE_NAME ") " PACKAGE_VERSION);
        ui->out("");
        ui->out("License terms:");
        ui->out("");
        success &= cat_file(ui, docdir / "COPYING");
        ui->out("");
        ui->out("Brought to you by:");
        ui->out("");
        success &= cat_file(ui, docdir / "AUTHORS");
        ui->out("");
        ui->out(F("Homepage: %s") % PACKAGE_URL);
    } else {
        const std::string& topic = cmdline.arguments()[0];

        if (topic == "authors") {
            success &= cat_file(ui, docdir / "AUTHORS");
        } else if (topic == "license") {
            success &= cat_file(ui, docdir / "COPYING");
        } else if (topic == "version") {
            ui->out(PACKAGE " (" PACKAGE_NAME ") " PACKAGE_VERSION);
        } else {
            throw cmdline::usage_error(F("Invalid about topic '%s'") % topic);
        }
    }

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
