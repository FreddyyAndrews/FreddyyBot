#ifndef LOGGING
#define LOGGING

#include <fstream>
#include <string>

void increment_game_num();
int get_game_num();
std::ofstream open_log_file();

#endif // LOGGING
