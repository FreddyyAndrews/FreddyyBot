# FreddyyBot

FreddyBot is my homemade chess engine written from scratch in C++.

[Think you can beat it? Give it a try!]()

## Getting Started

### Dependencies

* Project is designed to run on Ubuntu/WSL environments.

### Installing

* Clone the repo.
* Install g++ and make.
* Run ```Make run``` to launch the application.

### Executing program

The engine communciates on the terminal using the Universal Chess Interface.

Example:

```
position startpos moves e2e4 e7e5
go
Bestmove a1a2
position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
go
Bestmove a1a2
```
Provide a position to the engine using ```startpos``` followed by a sequence of moves or a fen string. Then provide the command ```go``` and the engine will return the best move.

The engine is connected to the Lichess API so you can play against it directly without interfacing with UCI yourself.

## Planned Improvements

* Better endgame evaluation.
* Better king safety evaluation.
* Better clock management.
* More efficient board representation (bitboards).

## License

This project is licensed under the MIT License - see the LICENSE.md file for details

## Acknowledgments

Inspiration, code snippets, etc.
* [Sebastian Lague](https://www.youtube.com/watch?v=U4ogK0MIzqk&ab_channel=SebastianLague)
* [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page)
