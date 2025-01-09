#!/bin/bash

# Name of the tmux session
SESSION="mysession"

# Command to check if the script is running
SCRIPT_NAME="lichess-bot.py"

# Log file
LOG_FILE="bot_offline.txt"

echo "$(date): Checking status..." >> $LOG_FILE

# Check if the tmux session exists
if tmux has-session -t $SESSION 2>/dev/null; then
    # Check if the script is running inside the tmux session
    if ! pgrep -f "$SCRIPT_NAME" > /dev/null; then
        # If the script is not running, restart it
        tmux send-keys -t $SESSION "python3 lichess-bot.py -v" Enter
        # Log the offline status and restart event
        echo "$(date): Bot was offline and restarted." >> $LOG_FILE
    else
        # If the script is running, log that it is online
        echo "$(date): Bot is online." >> $LOG_FILE
    fi
fi

tmux clear-history -t $SESSION

