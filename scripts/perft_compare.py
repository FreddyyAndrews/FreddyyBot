# compare_perft.py
def parse_perft_output(filename):
    perft_dict = {}
    with open(filename, 'r') as file:
        for line in file:
            if ':' in line:
                move, count = line.strip().split(':')
                perft_dict[move.strip()] = int(count.strip())
    return perft_dict

def compare_perft_outputs(stockfish_file, my_perft_file):
    stockfish_perft = parse_perft_output(stockfish_file)
    my_perft = parse_perft_output(my_perft_file)

    discrepancies = []
    for move in stockfish_perft:
        stockfish_count = stockfish_perft[move]
        my_count = my_perft.get(move)
        if my_count is None:
            discrepancies.append(f"Move {move} is missing in your Perft output.")
        elif stockfish_count != my_count:
            discrepancies.append(f"Discrepancy in move {move}: Stockfish={stockfish_count}, Yours={my_count}")

    for move in my_perft:
        if move not in stockfish_perft:
            discrepancies.append(f"Extra move {move} in your Perft output not found in Stockfish's output.")

    if discrepancies:
        print("Found discrepancies:")
        for discrepancy in discrepancies:
            print(discrepancy)
    else:
        print("No discrepancies found. The outputs match.")

if __name__ == "__main__":
    compare_perft_outputs('stockfish_perft.txt', 'my_perft.txt')
