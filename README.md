# FreddyyBot
![Alt text](assets/16-bit-and-32-bit-chess-piece-sets-inspired-by-8-bit-piece-v0-fmf0qlihhzna1.png)
Credit: MattSteeleArtworks on Reddit

FreddyBot is my homemade chess engine written from scratch in C++.

[Think you can beat it? Send a challenge on Lichess!](https://lichess.org/@/FreddyyBot)

![Alt text](assets/challenge.png)

Click ```Challenge to a Game``` and choose a format and time control. FreddyBot only accepts challenges to standard chess games, but you can select any time control. Note you must create a Lichess account.

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

* [Sebastian Lague](https://www.youtube.com/watch?v=U4ogK0MIzqk&ab_channel=SebastianLague)
* [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page)
* [Lichess-bot](https://github.com/lichess-bot-devs/lichess-bot)
