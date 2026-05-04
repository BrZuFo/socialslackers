import random

# -----------------------------
# OPTIONS
# -----------------------------
settings = ["workplace", "school", "public", "bathroom"]
people = ["boss", "friend", "co-worker", "stranger"]
vibes = ["polite", "tense", "sarcastic", "rude", "awkward"]

responses = {}

def set_response(key, options):
    responses[key] = options


# -----------------------------
# MEMORY SYSTEM
# -----------------------------
memory = {}


# -----------------------------
# SAFE INPUT HELPERS
# -----------------------------
def safe_choice(options, prompt):
    """
    Displays options and safely returns a valid selection.
    Defaults to first option if input is invalid.
    """
    print(f"\nChoose {prompt}:")
    for i, opt in enumerate(options):
        print(f"{i+1}. {opt}")

    choice = input("> ")

    if not choice.isdigit():
        print("Invalid input → using default.")
        return options[0]

    idx = int(choice) - 1

    if idx < 0 or idx >= len(options):
        print("Out of range → using default.")
        return options[0]

    return options[idx]


# -----------------------------
# GAME LOOP
# -----------------------------
def run_game():
    print("\n=== SOCIAL SIM ===")

    setting = safe_choice(settings, "setting")
    person = safe_choice(people, "person")

    print("\n====== SCENARIO ======")
    print(f"You are talking to a {person} in a {setting}.\n")

    chosen_vibes = random.sample(vibes, 3)
    options = []

    # -----------------------------
    # BUILD OPTIONS
    # -----------------------------
    for vibe in chosen_vibes:
        key = (setting, person, vibe)

        if key in responses:
            text = random.choice(responses[key])
        else:
            text = f"(No custom line for {vibe})"

        stats = {
            "INT": random.randint(1, 10),
            "Speech": random.randint(1, 10),
            "CHA": random.randint(1, 10)
        }

        options.append((vibe, text, stats))

    # -----------------------------
    # DISPLAY OPTIONS
    # -----------------------------
    for i, (vibe, text, stats) in enumerate(options):
        print(f"\nOption {i+1}:")
        print(f"[{vibe.upper()}] {text}")
        print(f"(INT:{stats['INT']} | Speech:{stats['Speech']} | CHA:{stats['CHA']})")

    # -----------------------------
    # SAFE CHOICE INPUT
    # -----------------------------
    choice_input = input("\nPick an option (1-3): ")

    if not choice_input.isdigit():
        print("Invalid input → defaulting to option 1.")
        choice_index = 0
    else:
        choice_index = int(choice_input) - 1
        if choice_index < 0 or choice_index >= len(options):
            print("Out of range → defaulting to option 1.")
            choice_index = 0

    choice = options[choice_index]

    print("\n=== YOU CHOSE ===\n")
    print(f"[{choice[0].upper()}] {choice[1]}\n")

    # -----------------------------
    # REACTION SYSTEM
    # -----------------------------
    handle_reaction(person, choice[0])


# -----------------------------
# REACTION SYSTEM
# -----------------------------
def handle_reaction(person, vibe):
    print("How did that go?")
    print("1. Positive")
    print("2. Neutral")
    print("3. Negative")

    result_input = input("> ")

    if result_input == "1":
        result = "Positive"
    elif result_input == "3":
        result = "Negative"
    else:
        result = "Neutral"

    if person not in memory:
        memory[person] = {}

    if vibe not in memory[person]:
        memory[person][vibe] = 0

    if result == "Positive":
        memory[person][vibe] += 1
    elif result == "Negative":
        memory[person][vibe] -= 1

    print(f"\n{person}'s reaction to '{vibe}': {memory[person][vibe]}")


# -----------------------------
# RUN PROGRAM
# -----------------------------
run_game()