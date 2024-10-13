import pandas as pd

# Read the Excel file into a DataFrame
df = pd.read_csv("move_timings.csv")

# Group the data by the "move" column and calculate the average time
average_time_per_move = df.groupby('move')['time'].mean().reset_index()

average_time_per_move.columns = ['move', 'average_time']

average_time_per_move.to_csv("move_timing_averages_iteration_1.csv", index=False)
