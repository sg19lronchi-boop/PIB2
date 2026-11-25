# Predictive Input Buffer

A small input buffer for cube/ball and wave.

- **Cube/ball:** if you click and release while falling, the release is delayed
  (up to the buffer window) so the click is still registered on landing.
- **Wave:** a very short tap is held for at least the buffer window.

Window is measured in physics steps and is capped at 4. Check your leaderboard's
rules before using input-modifying mods on submitted runs.
