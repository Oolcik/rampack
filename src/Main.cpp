//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cstdlib>
#include <string>
#include <iostream>

#include "frontend/modes/HelpMode.h"
#include "frontend/modes/CasinoMode.h"
#include "frontend/modes/PreviewMode.h"
#include "frontend/modes/ShapePreviewMode.h"
#include "frontend/modes/TrajectoryMode.h"
#include "utils/Logger.h"
#include "utils/Utils.h"


namespace {
    Logger logger(std::cout);

    void logger_terminate_handler() {
        logger.error() << "Terminate called after throwing an instance of ";
        try {
            std::rethrow_exception(std::current_exception());
        } catch (const std::exception &ex) {
            logger << demangle(typeid(ex).name()) << std::endl;
            logger << "what(): " << ex.what() << std::endl;
        }
        std::abort();
    }

    int handle_commands(const std::string &cmd, const std::string &mode, int argc, char **argv) {
        if (mode == "-h" || mode == "--help")
            return HelpMode(logger).main(argc, argv);
        else if (mode == "casino")
            return CasinoMode(logger).main(argc, argv);
        else if (mode == "preview")
            return PreviewMode(logger).main(argc, argv);
        else if (mode == "shape-preview")
            return ShapePreviewMode(logger).main(argc, argv);
        else if (mode == "trajectory")
            return TrajectoryMode(logger).main(argc, argv);
        logger.error() << "Unknown mode " << mode << ". See " << cmd << " --help" << std::endl;
        return EXIT_FAILURE;
    }
}


int main(int argc, char **argv) {
    std::set_terminate(logger_terminate_handler);

    std::string cmd(argv[0]);
    if (argc < 2) {
        logger.error() << "Usage: " << cmd << " [mode] (mode dependent parameters). " << std::endl;
        logger << "Type " << cmd << " --help to see available modes" << std::endl;
        exit(EXIT_FAILURE);
    }

    // We now shift the arguments, and pretend, that the new first (command) is "cmd mode"
    // Different modes can then parse the arguments separately and will think that the whole argv[0] is "cmd mode"
    std::string mode(argv[1]);
    std::string cmdAndMode = cmd + " " + mode;
    argv++;
    argc--;
    argv[0] = cmdAndMode.data();

    // Try and rethrow exception to let the destructors be called during stack unwinding, but then use the general
    // terminate handler. This however does not work for exceptions thrown from threads other than master.
    // This is enabled for the Release build - in the Debug build we want to know from where the exception is thrown
    // and using rethrowing erases this information, and we do not need destructors to be called, since the execution
    // is only paused, so we have ana access to everything anyway.
#ifdef NDEBUG
    try {
        return handle_commands(cmd, mode, argc, argv);
    } catch (const std::exception &) {
        throw;
    }
#else
    return handle_commands(cmd, mode, argc, argv);
#endif
}