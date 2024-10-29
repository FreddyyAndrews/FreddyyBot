#include "move_generator.h"
#include <iostream>


MoveGenerator::MoveGenerator()
{
    std::cout << "Size of move: " << sizeof(Move);
    move_heap = new Move[218];
}

MoveGenerator::~MoveGenerator()
{
    delete[] move_heap;
}