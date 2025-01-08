#!/bin/bash

# Name of the tmux session
SESSION="mysession"

# Command to check if the script is running
SCRIPT_NAME="lichess-bot.py"

# Check if the tmux session exists
if tmux has-session -t $SESSION 2>/dev/null; then
    # Check if the script is running inside the tmux session
    if ! tmux capture-pane -pt $SESSION | grep -q "$SCRIPT_NAME"; then
        # If the script is not running, restart it
        tmux send-keys -t $SESSION "python3 lichess-bot.py -v" Enter
    fi
else
    # If the tmux session doesn't exist, create it and run the script
    tmux new-session -d -s $SESSION "python3 lichess-bot.py -v"
fi
