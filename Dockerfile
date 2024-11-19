# Use the Lichess bot image as the base image
FROM lichessbotdevs/lichess-bot:latest

# Copy your engine and config file into the container
COPY ./build/bin/main /lichess-bot/config/freddyybot_5
COPY ./config.yml /lichess-bot/config/config.yml

# Set execution permissions for the engine binary
RUN chmod +x /lichess-bot/config/your_engine_binary

# Set the working directory
WORKDIR /lichess-bot

# Run the bot with the given config
ENTRYPOINT ["python3", "lichess-bot.py"]
