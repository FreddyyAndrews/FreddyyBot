import os
import csv


def parse_chess_logs(logs_directory, output_csv):
    # Columns: position, remaining_time, time_increment, allocated_time, depth_reached, actual_time_taken
    # Assumptions based on example logs:
    # 1. "Received: position startpos moves ..." line provides the full position.
    #    Format: "Received: position startpos moves e2e3 c7c5 ..."
    #    We'll join all tokens after 'moves' into a single moves list.
    # 2. "Received: go wtime 52560 btime 64451 winc 2000 binc 2000"
    #    Provides white time left (wtime), black time left (btime), white increment (winc), black increment (binc).
    # 3. "Debug: Allocated 2574 milliseconds"
    #    Provides allocated_time.
    # 4. "Searched depth 7 in 8669 milliseconds"
    #    Provides depth_reached (7) and actual_time_taken (8669).
    # To determine who's turn it is based on the number of moves:
    # If number_of_moves is even, it's White's turn; if odd, it's Black's turn.
    #
    # We'll parse each file line by line, extracting needed data once all required lines are found.
    #
    # Once we have:
    # position (startpos followed by moves),
    # remaining_time (either wtime or btime based on turn),
    # time_increment (winc or binc based on turn),
    # allocated_time, depth_reached, actual_time_taken
    # we write a row to the CSV.

    rows = []

    for filename in os.listdir(logs_directory):
        if filename.endswith(".txt"):
            filepath = os.path.join(logs_directory, filename)

            position = None
            wtime = None
            btime = None
            winc = None
            binc = None
            allocated_time = None
            depth_reached = None
            actual_time_taken = None

            with open(filepath, "r", encoding="utf-8") as f:
                for line in f:
                    line = line.strip()

                    # Parse position and moves
                    # Example: "Received: position startpos moves e2e3 c7c5 ..."
                    if line.startswith("Received: position"):
                        parts = line.split()
                        # find 'moves' index
                        if "moves" in parts:
                            moves_index = parts.index("moves")
                            # Moves after 'moves' keyword
                            moves = parts[moves_index + 1 :]
                            # position is basically "startpos moves " + all moves
                            # or just store the moves list
                            # For our column: let's store a string "startpos moves <move1> <move2>..."
                            position = "startpos moves " + " ".join(moves)
                        else:
                            # No moves means empty position
                            position = "startpos"

                    # Parse go line: "Received: go wtime 52560 btime 64451 winc 2000 binc 2000"
                    elif line.startswith("Received: go"):
                        parts = line.split()
                        # Extract times and increments
                        if "wtime" in parts:
                            wtime = parts[parts.index("wtime") + 1]
                        if "btime" in parts:
                            btime = parts[parts.index("btime") + 1]
                        if "winc" in parts:
                            winc = parts[parts.index("winc") + 1]
                        if "binc" in parts:
                            binc = parts[parts.index("binc") + 1]

                    # Parse allocated time: "Debug: Allocated 2574 milliseconds"
                    elif "Debug: Allocated" in line and "milliseconds" in line:
                        # Split and extract the number before "milliseconds"
                        parts = line.split()
                        # Find 'Allocated' index
                        if "Allocated" in parts:
                            idx = parts.index("Allocated")
                            allocated_time = parts[
                                idx + 1
                            ]  # the number before milliseconds

                    # Parse depth and actual time: "Searched depth 7 in 8669 milliseconds"
                    elif "Searched depth" in line and "milliseconds" in line:
                        parts = line.split()
                        # Format: "Searched depth X in Y milliseconds"
                        if "depth" in parts:
                            depth_idx = parts.index("depth")
                            depth_reached = parts[depth_idx + 1]
                        if "in" in parts:
                            in_idx = parts.index("in")
                            actual_time_taken = parts[
                                in_idx + 1
                            ]  # number before 'milliseconds'

            # After parsing the file, if position is found, determine who's turn it is
            # Count the moves in position
            # position string looks like: "startpos moves e2e3 c7c5 ..."
            # Split by spaces and count moves after 'moves'
            remaining_time = None
            time_increment = None
            if position and "moves" in position:
                moves_list = position.split()[2:]  # after 'startpos moves'
                num_moves = len(moves_list)
                # If even number of moves, white to move; odd => black to move
                if num_moves % 2 == 0:
                    # White's turn
                    remaining_time = wtime
                    time_increment = winc
                else:
                    # Black's turn
                    remaining_time = btime
                    time_increment = binc
            else:
                # If no moves provided, it's the start of the game, white to move
                remaining_time = wtime
                time_increment = winc

            # Add row if we have all or most important data
            # The user did not specify if all fields are guaranteed, but we assume all logs have the needed info
            rows.append(
                [
                    position,
                    remaining_time,
                    time_increment,
                    allocated_time,
                    depth_reached,
                    actual_time_taken,
                ]
            )

    # Write to CSV
    with open(output_csv, "w", newline="", encoding="utf-8") as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(
            [
                "position",
                "remaining_time",
                "time_increment",
                "allocated_time",
                "depth_reached",
                "actual_time_taken",
            ]
        )
        for row in rows:
            writer.writerow(row)


if __name__ == "__main__":
    parse_chess_logs("logs/", "logs_summarized.csv")
