import pandas as pd
import subprocess
import random
import csv
import re
import sys
from tqdm import tqdm

def send_command(engine, command):
    engine.stdin.write(command + "\n")
    engine.stdin.flush()

def get_bestmove(engine):
    while True:
        line = engine.stdout.readline().strip()
        if line.startswith("bestmove"):
            parts = line.split()
            if len(parts) >= 4 and parts[0] == "bestmove" and parts[2] == "ponder":
                return parts[1], parts[3]
            if len(parts) == 2 and parts[0] == "bestmove":
                return parts[1], None

def play_puzzle(engine, fen, moves_list):
    puzzle_moves = moves_list.split()

    # Apply the opponent's first move immediately
    # (the puzzle's first move belongs to the opponent)
    opponent_first_move = puzzle_moves[0]
    position_cmd = f"position fen {fen} moves {opponent_first_move}"
    send_command(engine, position_cmd)

    # The engine's solution now starts from the second move
    solution_moves = puzzle_moves[1:]

    engine_moves = []
    current_move_index = 0
    success = True

    # The loop now uses solution_moves instead of the entire puzzle_moves
    while current_move_index < len(solution_moves):
        # Build partial puzzle sequence to keep the position in sync
        partial_solution = solution_moves[:current_move_index]
        position_cmd = f"position fen {fen} moves {opponent_first_move}"
        if partial_solution:
            position_cmd += f" {' '.join(partial_solution)}"
        
        send_command(engine, position_cmd)
        send_command(engine, "go")
        best_move, ponder_move = get_bestmove(engine)

        if best_move is None:
            success = False
            break

        engine_moves.append(best_move)

        # Compare engine move with puzzle's next expected move
        if best_move == solution_moves[current_move_index]:
            current_move_index += 1
        else:
            success = False
            break

        # Automatically apply the opponent's next move if it exists
        if current_move_index < len(solution_moves):
            current_move_index += 1
            if current_move_index >= len(solution_moves):
                break

    return engine_moves, success

def main():
    if len(sys.argv) > 1:
        try:
            num_puzzles = int(sys.argv[1])
        except ValueError:
            num_puzzles = 100
    else:
        num_puzzles = 100

    # Arbitrary popularity threshold:
    popularity_threshold = 20
    rating_threshold = 2000

    # Load desired columns, including a 'Popularity' column
    df = pd.read_csv("scripts/lichess_db_puzzle.csv", usecols=["FEN", "Moves", "Popularity", "Rating"])

    df = df[df["Popularity"] >= popularity_threshold]
    df = df[df["Rating"] >= rating_threshold]

    # Randomly sample from these popular mate puzzles
    df_sampled = df.sample(n=num_puzzles, random_state=42).reset_index(drop=True)

    engine = subprocess.Popen(
        ["build/bin/main"],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        universal_newlines=True,
        bufsize=1
    )

    puzzle_results = []
    correct_count = 0
    total_puzzles = len(df_sampled)

    for idx, row in enumerate(tqdm(df_sampled.itertuples(index=False), total=total_puzzles, desc="Processing puzzles", unit=" puzzle")):
        fen = row.FEN
        puzzle_solution = row.Moves

        engine_moves, success = play_puzzle(engine, fen, puzzle_solution)
        puzzle_results.append((fen, puzzle_solution, " ".join(engine_moves), success))

        if success:
            correct_count += 1

    with open("puzzle_results.csv", "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(["fen", "correct_moves", "engine_moves", "engine_correct"])
        for fen, correct_moves, engine_moves_str, success in puzzle_results:
            writer.writerow([fen, correct_moves, engine_moves_str, success])

    percentage_correct = (correct_count / total_puzzles) * 100
    print(f"Engine solved {correct_count} out of {total_puzzles} puzzles.")
    print(f"Success rate: {percentage_correct:.2f}%")

    send_command(engine, "quit")
    engine.wait()

if __name__ == "__main__":
    main()
