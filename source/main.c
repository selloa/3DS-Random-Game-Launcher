#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <3ds.h>
#include <3ds/svc.h>

// Console handles
PrintConsole topScreen, bottomScreen;

// Function declarations
void typewriter_text(const char* text, int delay_ms);
void typewriter_colored_text(const char* text, const char* color_code, int delay_ms);
void display_story_with_animation(const char* story_text);
void dramatic_story_display(const char* story_text);
void display_feedback(const char* message);

// Sound functions
void play_beep(int frequency, int duration_ms) {
    // Simple beep sound using CSND
    u32 sampleRate = 32768;
    u32 samples = (sampleRate * duration_ms) / 1000;
    
    // Generate a simple sine wave beep
    s16* buffer = (s16*)malloc(samples * sizeof(s16));
    if (buffer) {
        for (u32 i = 0; i < samples; i++) {
            buffer[i] = (s16)(sin(2.0 * M_PI * frequency * i / sampleRate) * 16383);
        }
        
        // Play the sound
        csndPlaySound(8, SOUND_FORMAT_16BIT | SOUND_ONE_SHOT, sampleRate, 1.0, 0.0, buffer, buffer, samples);
        free(buffer);
    }
}

void play_monolith_sound() {
    // Mysterious, low-frequency sound for monolith
    play_beep(220, 500);  // A3 note
    svcSleepThread(100000000LL); // 100ms delay
    play_beep(185, 500);  // F#3 note
    svcSleepThread(100000000LL);
    play_beep(165, 800);  // E3 note
}

void play_tool_sound() {
    // Sharp, metallic sound for picking up tools
    play_beep(880, 200);  // A5 note
    svcSleepThread(50000000LL); // 50ms delay
    play_beep(1100, 200); // C#6 note
}

void play_conflict_sound() {
    // Harsh, discordant sound for conflict
    play_beep(330, 300);  // E4 note
    svcSleepThread(50000000LL);
    play_beep(370, 300);  // F#4 note
    svcSleepThread(50000000LL);
    play_beep(415, 400);  // G#4 note
}

void play_transformation_sound() {
    // Ascending sound for the bone-to-spaceship transformation
    play_beep(220, 200);  // A3
    svcSleepThread(100000000LL);
    play_beep(330, 200);  // E4
    svcSleepThread(100000000LL);
    play_beep(440, 200);  // A4
    svcSleepThread(100000000LL);
    play_beep(660, 400);  // E5
}

void play_hal_sound() {
    // Electronic, robotic sound for HAL
    play_beep(440, 150);  // A4
    svcSleepThread(75000000LL);
    play_beep(440, 150);
    svcSleepThread(75000000LL);
    play_beep(440, 150);
    svcSleepThread(75000000LL);
    play_beep(330, 300);  // E4
}

// Game state structure
typedef struct {
    int current_scene;
    int has_hal_9000;
    int has_monolith;
    int has_bone;
    int has_touched_monolith;
    int has_learned_tools;
    int has_seen_war;
} game_state_t;

// Scene descriptions
const char* scene_descriptions[] = {
    "You find yourself in a prehistoric landscape. The sun beats down on the African savanna. In the distance, you see a group of apes gathered around a mysterious black monolith. The monolith stands tall and smooth, its surface reflecting the harsh sunlight. Something about it calls to you...",
    "You cautiously move toward the monolith. As you get closer, you feel a strange energy emanating from it. The apes seem to be learning from it - you watch as one picks up a bone and begins to use it as a tool. The monolith's smooth surface shows no seams or markings, yet it pulses with an otherworldly presence.",
    "You observe the apes from a safe distance. They seem different now - more aware, more intelligent. One ape picks up a bone and begins using it to dig in the ground. Another uses a rock to crack open nuts. The monolith has taught them something profound about tools and survival.",
    "You search the area for anything that could be used as a tool. You find several bones scattered around, some sharp rocks, and sturdy sticks. The apes are now using these primitive tools with surprising skill. The monolith has awakened something in all of you.",
    "You wait patiently and observe the scene. Time seems to slow as you watch the transformation. The apes are evolving before your eyes - they're using tools, working together, and showing signs of intelligence. The monolith glows faintly, and you feel your own consciousness expanding.",
    "As your hand makes contact with the monolith, reality shifts around you. You see visions of the future - spacecraft, computers, and beings of pure energy. The monolith is a teacher, a guide for evolution. You understand now that this is just the beginning of a much larger journey.",
    "You pick up a bone, and suddenly you understand its potential. You can use it as a weapon, a tool, a lever. The monolith has given you knowledge. You watch as the apes begin to use their tools more effectively, and you realize you're witnessing the birth of technology.",
    "The scene has changed. The apes are now in conflict, using their tools as weapons. Bones clash against bones, and the savanna echoes with the sounds of the first war. The monolith stands silent, its work done. You realize that with intelligence comes both creation and destruction.",
    "As the conflict rages, you notice something extraordinary. One ape throws a bone high into the air, and as it spins, the scene transforms. The bone becomes a spacecraft, the savanna becomes the void of space, and you realize you're witnessing the leap from primitive tools to advanced technology.",
    "You are aboard the Discovery One spacecraft, en route to Jupiter. The ship's computer, HAL 9000, monitors all systems. The crew is in hibernation."
};

// Scene options
const char* scene_options[][4] = {
    {"Approach the monolith", "Watch the apes", "Look for tools", "Wait and observe"},
    {"Touch the monolith", "Pick up a bone", "Try to communicate", "Step back"},
    {"Approach the apes", "Find your own bone", "Watch the monolith", "Look for resources"},
    {"Pick up the sharpest bone", "Choose a heavy rock", "Take a sturdy stick", "Gather multiple tools"},
    {"Join the apes", "Approach the monolith", "Continue observing", "Try to understand"},
    {"Embrace the visions", "Pull away", "Try to communicate", "Share with apes"},
    {"Use as weapon", "Use as tool", "Teach the apes", "Experiment"},
    {"Try to stop conflict", "Join stronger group", "Find safe place", "Return to monolith"},
    {"Follow bone to space", "Stay on ground", "Understand transformation", "Look for monolith"},
    {"Explore spacecraft", "Check on crew", "Talk to HAL", "Look at Jupiter"}
};

// Function to clear both screens and show header
void clear_and_show_header(void) {
    // Clear top screen
    consoleClear();
    printf("\n\n\x1b[37m   2001: A SPACE ODYSSEY\x1b[0m\n");
    printf("\x1b[90m    Text Adventure Game\x1b[0m\n");
    printf("\x1b[90m    by selloa (2025)\x1b[0m\n\n");
    
    // Clear bottom screen
    consoleSelect(&bottomScreen);
    consoleClear();
    consoleSelect(&topScreen);
}

// Function to display current scene
void display_scene(game_state_t* state) {
    // Use dramatic effect for the first scene
    if (state->current_scene == 0) {
        dramatic_story_display(scene_descriptions[state->current_scene]);
    } else {
        clear_and_show_header();
        display_story_with_animation(scene_descriptions[state->current_scene]);
    }
    
    // Display options on bottom screen with colored buttons
    consoleSelect(&bottomScreen);
    printf("\n\n\x1b[37m    What do you do?\x1b[0m\n\n");
    printf("\x1b[90m    \x1b[31mA\x1b[90m. %s\x1b[0m\n\n", scene_options[state->current_scene][0]); // A in red
    printf("\x1b[90m    \x1b[33mB\x1b[90m. %s\x1b[0m\n\n", scene_options[state->current_scene][1]); // B in orange-yellow
    printf("\x1b[90m    \x1b[36mX\x1b[90m. %s\x1b[0m\n\n", scene_options[state->current_scene][2]); // X in sky blue
    printf("\x1b[90m    \x1b[32mY\x1b[90m. %s\x1b[0m\n\n", scene_options[state->current_scene][3]); // Y in dark green
    printf("\x1b[90m    Press START to exit\x1b[0m\n");
    
    // Switch back to top screen for future text output
    consoleSelect(&topScreen);
}

// Function to display text with typewriter effect
void typewriter_text(const char* text, int delay_ms) {
    consoleSelect(&topScreen);
    
    int len = strlen(text);
    for (int i = 0; i < len; i++) {
        printf("%c", text[i]);
        gfxFlushBuffers();
        gfxSwapBuffers();
        svcSleepThread(delay_ms * 1000000LL); // Convert ms to nanoseconds
    }
}

// Function to display colored text with typewriter effect
void typewriter_colored_text(const char* text, const char* color_code, int delay_ms) {
    consoleSelect(&topScreen);
    printf("%s", color_code); // Set color
    
    int len = strlen(text);
    for (int i = 0; i < len; i++) {
        printf("%c", text[i]);
        gfxFlushBuffers();
        gfxSwapBuffers();
        svcSleepThread(delay_ms * 1000000LL); // Convert ms to nanoseconds
    }
    
    printf("\x1b[0m"); // Reset color
}

// Function to display story text with typewriter effect
void display_story_with_animation(const char* story_text) {
    consoleSelect(&topScreen);
    printf("\n");
    typewriter_colored_text(story_text, "\x1b[36m", 30); // 30ms delay, cyan color
    printf("\n\n");
}

// Function to clear screen and type story with dramatic effect
void dramatic_story_display(const char* story_text) {
    consoleSelect(&topScreen);
    consoleClear();
    printf("\n\n   2001: A SPACE ODYSSEY\n");
    printf("   Text Adventure Game\n");
    printf("   by selloa (2025)\n\n");
    
    // Pause for dramatic effect
    svcSleepThread(1000000000LL); // 1 second pause
    
    printf("\n");
    typewriter_colored_text(story_text, "\x1b[36m", 35); // Slightly slower for drama
    printf("\n\n");
}

// Function to display feedback message on top screen
void display_feedback(const char* message) {
    consoleSelect(&topScreen);
    printf("\n\n");
    typewriter_colored_text(message, "\x1b[33m", 25); // 25ms delay, yellow color
    printf("\n\n");
}

// Function to handle scene transitions
void handle_choice(game_state_t* state, int choice) {
    switch (state->current_scene) {
        case 0: // Initial prehistoric landscape
            switch (choice) {
                case 1: // Approach the monolith
                    state->current_scene = 1;
                    break;
                case 2: // Watch the apes
                    state->current_scene = 2;
                    break;
                case 3: // Look for tools
                    state->current_scene = 3;
                    break;
                case 4: // Wait and observe
                    state->current_scene = 4;
				break;
			}
            break;
            
        case 1: // Approaching the monolith
            switch (choice) {
                case 1: // Touch the monolith
                    state->current_scene = 5;
                    state->has_touched_monolith = 1;
                    play_monolith_sound();
                    break;
                case 2: // Pick up a bone
                    state->current_scene = 6;
                    state->has_bone = 1;
                    state->has_learned_tools = 1;
                    play_tool_sound();
                    break;
                case 3: // Try to communicate
                    display_feedback("You attempt to communicate with the apes. They seem to understand you better now.");
                    state->current_scene = 7;
                    break;
                case 4: // Step back
                    display_feedback("You step back, overwhelmed by the monolith's power.");
                    state->current_scene = 7;
                    break;
            }
            break;
            
        case 2: // Watching the apes
            switch (choice) {
                case 1: // Approach the apes
                    display_feedback("You approach the apes. They seem more intelligent now.");
                    state->current_scene = 6;
                    break;
                case 2: // Find your own bone
                    state->current_scene = 6;
                    state->has_bone = 1;
                    break;
                case 3: // Watch the monolith
                    display_feedback("You focus on the monolith. It pulses with energy.");
                    state->current_scene = 1;
                    break;
                case 4: // Look for resources
                    state->current_scene = 3;
                    break;
            }
            break;
            
        case 3: // Searching for tools
            switch (choice) {
                case 1: // Pick up sharpest bone
                    state->current_scene = 6;
                    state->has_bone = 1;
                    state->has_learned_tools = 1;
                    play_tool_sound();
                    break;
                case 2: // Choose heavy rock
                    display_feedback("You pick up a heavy rock. It could be useful as a tool or weapon.");
                    state->current_scene = 6;
                    break;
                case 3: // Take sturdy stick
                    display_feedback("You select a sturdy stick. The apes are using similar tools effectively.");
                    state->current_scene = 6;
                    break;
                case 4: // Gather multiple tools
                    display_feedback("You gather several tools. You're well-equipped now.");
                    state->current_scene = 6;
                    state->has_bone = 1;
                    state->has_learned_tools = 1;
                    break;
            }
            break;
            
        case 4: // Waiting and observing
            switch (choice) {
                case 1: // Join the apes
                    display_feedback("You join the apes in their activities. They welcome you.");
                    state->current_scene = 6;
                    break;
                case 2: // Approach the monolith
                    state->current_scene = 1;
                    break;
                case 3: // Continue observing
                    display_feedback("You continue to observe. The transformation is remarkable.");
                    state->current_scene = 7;
                    break;
                case 4: // Try to understand
                    display_feedback("You try to understand what's happening. The monolith is teaching evolution.");
                    state->current_scene = 1;
                    break;
            }
            break;
            
        case 5: // Touching the monolith
            switch (choice) {
                case 1: // Embrace the visions
                    display_feedback("You embrace the visions. You see the future of humanity.");
                    state->current_scene = 7;
                    break;
                case 2: // Pull away
                    display_feedback("You pull away from the monolith, overwhelmed by the experience.");
                    state->current_scene = 1;
                    break;
                case 3: // Try to communicate
                    display_feedback("You try to communicate with the monolith. It responds with more visions.");
                    state->current_scene = 7;
                    break;
                case 4: // Share with apes
                    display_feedback("You try to share your experience with the apes.");
                    state->current_scene = 7;
                    break;
            }
            break;
            
        case 6: // Picking up a bone
            switch (choice) {
                case 1: // Use as weapon
                    display_feedback("You use the bone as a weapon. The apes are impressed.");
                    state->current_scene = 7;
                    break;
                case 2: // Use as tool
                    display_feedback("You use the bone as a tool. You're learning to be more efficient.");
                    state->current_scene = 7;
                    break;
                case 3: // Teach the apes
                    display_feedback("You teach the apes new techniques. They're eager to learn.");
                    state->current_scene = 7;
                    break;
                case 4: // Experiment
                    display_feedback("You experiment with different uses for the bone.");
                    state->current_scene = 7;
				break;
			}
            break;
            
        case 7: // The first war
            switch (choice) {
                case 1: // Try to stop conflict
                    display_feedback("You try to stop the conflict, but the apes are too aggressive.");
                    state->current_scene = 8;
                    break;
                case 2: // Join stronger group
                    display_feedback("You join the stronger group. Survival is key.");
                    state->current_scene = 8;
                    break;
                case 3: // Find safe place
                    display_feedback("You find a safe place to observe the conflict.");
                    state->current_scene = 8;
                    break;
                case 4: // Return to monolith
                    display_feedback("You return to the monolith for guidance.");
                    state->current_scene = 1;
                    break;
            }
            break;
            
        case 8: // The leap forward
            switch (choice) {
                case 1: // Follow bone to space
                    state->current_scene = 9;
                    play_transformation_sound();
                    break;
                case 2: // Stay on ground
                    display_feedback("You choose to stay on the ground. The transformation continues around you.");
                    state->current_scene = 9;
                    break;
                case 3: // Understand transformation
                    display_feedback("You try to understand the transformation. The bone becomes a spacecraft!");
                    state->current_scene = 9;
                    break;
                case 4: // Look for monolith
                    display_feedback("You look for the monolith, but it's gone. Its work is done.");
                    state->current_scene = 9;
				break;
			}
            break;
            
        case 9: // Discovery One
            switch (choice) {
                case 1: // Explore spacecraft
                    display_feedback("You explore the Discovery One. It's a marvel of technology.");
                    break;
                case 2: // Check on crew
                    display_feedback("The crew is in hibernation pods. They appear to be in deep sleep.");
                    break;
                case 3: // Talk to HAL
                    display_feedback("HAL 9000: 'I'm sorry Dave, I'm afraid I can't do that.'");
                    state->has_hal_9000 = 1;
                    play_hal_sound();
                    break;
                case 4: // Look at Jupiter
                    display_feedback("Jupiter looms large in the viewport. The gas giant's storms are mesmerizing.");
				break;
			}
            break;
    }
    
    // Small delay before showing next scene
    svcSleepThread(1000000000LL); // 1 second delay
}

int main()
{
	// Initialize graphics system
	gfxInitDefault();
	
	// Initialize audio system
	csndInit();
	
	// Initialize both consoles
	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);
	
    // Initialize game state
    game_state_t game_state = {0};
    
    // Show initial scene
    display_scene(&game_state);
    
    // Main game loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown();

        // Handle input
        if (kDown & KEY_START) {
            break; // Exit game
        }
        
        if (kDown & KEY_A) {
            handle_choice(&game_state, 1);
            display_scene(&game_state);
        }
        
        if (kDown & KEY_B) {
            handle_choice(&game_state, 2);
            display_scene(&game_state);
        }
        
        if (kDown & KEY_X) {
            handle_choice(&game_state, 3);
            display_scene(&game_state);
        }
        
        if (kDown & KEY_Y) {
            handle_choice(&game_state, 4);
            display_scene(&game_state);
        }
    }
    
    // Cleanup
	csndExit();
	gfxExit();
	return 0;
}