#include <iostream>
#include <string>
#include <sys/wait.h>
#include "uci_manager.h"

int main()
{
    UCIManager manager;
    std::string white_engine_name, black_engine_name;

    std::cout << "Enter the engine to play white: ";
    std::getline(std::cin, white_engine_name);

    if (!manager.is_engine_available(white_engine_name))
    {
        std::cout << "That engine is not available" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Enter the engine to play black: ";
    std::getline(std::cin, black_engine_name);

    if (!manager.is_engine_available(black_engine_name))
    {
        std::cout << "That engine is not available" << std::endl;
        return EXIT_FAILURE;
    }

    pid_t white_pid = manager.start_engine(white_engine_name, true);
    pid_t black_pid = manager.start_engine(black_engine_name, false);

    manager.communicate_with_white();
    manager.communicate_with_black();

    waitpid(white_pid, NULL, 0);
    waitpid(black_pid, NULL, 0);

    return EXIT_SUCCESS;
}