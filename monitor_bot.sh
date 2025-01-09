#!/bin/bash

# Name of the tmux session
SESSION="mysession"

# Command to check if the script is running
SCRIPT_NAME="lichess-bot.py"

# Log file
LOG_FILE="bot_status.log"

# Check if the tmux session exists
if tmux has-session -t $SESSION 2>/dev/null; then
    # Check if the script is running inside the tmux session
    if ! tmux capture-pane -pt $SESSION | grep -q "$SCRIPT_NAME"; then
        # If the script is not running, restart it
        tmux send-keys -t $SESSION "python3 lichess-bot.py -v" Enter
        # Log the offline status and restart event
        echo "$(date): Bot was offline and restarted." >> $LOG_FILE
    else
        # If the script is running, log that it is online
        echo "$(date): Bot is online." >> $LOG_FILE
    fi
else
    # If the tmux session doesn't exist, create it and run the script
    tmux new-session -d -s $SESSION "python3 lichess-bot.py -v"
    # Log the session creation and start of the bot
    echo "$(date): Bot session created and started." >> $LOG_FILE
fi
