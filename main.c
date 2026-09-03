#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define TICKS_GREEN     5U
#define TICKS_YELLOW    2U
#define TICKS_RED       4U
#define QUEUE_BUSY      6U   /* more cars than this means "busy" */
#define LOG_LEN         20U

typedef enum { LIGHT_GREEN = 0, LIGHT_YELLOW, LIGHT_RED } LightState_t;

/* status bits */
#define BIT_NIGHT       0U
#define BIT_BUSY        1U
#define BIT_BLINK_ON    2U

#define SET_BIT(reg, n)      ((reg) |= (uint8_t)(1U << (n)))
#define CLR_BIT(reg, n)      ((reg) &= (uint8_t)~(1U << (n)))
#define TOGGLE_BIT(reg, n)   ((reg) ^= (uint8_t)(1U << (n)))
#define READ_BIT(reg, n)     ((uint8_t)(((reg) >> (n)) & 1U))

static LightState_t light;
static uint8_t      status;       /* NIGHT / BUSY / BLINK_ON */
static uint8_t      ticksLeft;    /* time left in this colour */
static uint8_t      carsWaiting;
static uint32_t     carsPassed;
static char         logLine[LOG_LEN]; /* last 20 colours as letters */

/* Forward declarations */
static void         resetCrossing(void);
static uint8_t      ticksFor(LightState_t s);
static LightState_t nextState(LightState_t s);
static void         drawLight(void);
static void         tick(void);
static void         addCars(void);
static void         toggleNight(void);
static void         pushLog(char c);
static void         showLog(void);
static void         crossingReport(void);

static void resetCrossing(void) {
    light = LIGHT_RED;
    status = 0U;
    ticksLeft = ticksFor(light);
    carsWaiting = 0U;
    carsPassed = 0U;
    memset(logLine, ' ', LOG_LEN);
}

static uint8_t ticksFor(LightState_t s) {
    if (s == LIGHT_GREEN) {
        if (READ_BIT(status, BIT_BUSY)) {
            return TICKS_GREEN + 2U;
        }
        return TICKS_GREEN;
    } else if (s == LIGHT_YELLOW) {
        return TICKS_YELLOW;
    } else if (s == LIGHT_RED) {
        return TICKS_RED;
    }
    return TICKS_GREEN;
}

static LightState_t nextState(LightState_t s) {
    if (s == LIGHT_GREEN)  return LIGHT_YELLOW;
    if (s == LIGHT_YELLOW) return LIGHT_RED;
    return LIGHT_GREEN;
}

static void pushLog(char c) {
    for (uint32_t i = 0; i < LOG_LEN - 1; i++) {
        logLine[i] = logLine[i + 1];
    }
    logLine[LOG_LEN - 1] = c;
}

static void showLog(void) {
    for (uint32_t i = 0; i < LOG_LEN; i++) {
        putchar(logLine[i]);
    }
    putchar('\n');
}

static void drawLight(void) {
    int gOn = 0, yOn = 0, rOn = 0;

    if (READ_BIT(status, BIT_NIGHT)) {
        if (READ_BIT(status, BIT_BLINK_ON)) {
            yOn = 1;
        }
    } else {
        if (light == LIGHT_GREEN)  gOn = 1;
        if (light == LIGHT_YELLOW) yOn = 1;
        if (light == LIGHT_RED)    rOn = 1;
    }

    printf("\n      ( %s )     -- GREEN  [%s]\n", gOn ? "O" : " ", gOn ? "ON" : "off");
    printf("      ( %s )     -- YELLOW [%s]\n", yOn ? "O" : " ", yOn ? "ON" : "off");
    printf("      ( %s )     -- RED    [%s]\n", rOn ? "O" : " ", rOn ? "ON" : "off");
    
    if (READ_BIT(status, BIT_NIGHT)) {
        printf("Current State: NIGHT MODE (Blinking Yellow)\n");
    } else {
        const char *name = (light == LIGHT_GREEN) ? "Green" : (light == LIGHT_YELLOW) ? "Yellow" : "Red";
        printf("Colour: %s | Ticks Left: %u | Cars Waiting: %u\n", name, ticksLeft, carsWaiting);
    }
}

static void tick(void) {
    if (READ_BIT(status, BIT_NIGHT)) {
        if (READ_BIT(status, BIT_BLINK_ON)) {
            CLR_BIT(status, BIT_BLINK_ON);
        } else {
            SET_BIT(status, BIT_BLINK_ON);
        }
        pushLog('y');
        return;
    }

    /* Daytime logging & logic */
    char logChar = 'R';
    if (light == LIGHT_GREEN)  logChar = 'G';
    else if (light == LIGHT_YELLOW) logChar = 'Y';
    else if (light == LIGHT_RED)    logChar = 'R';
    pushLog(logChar);

    if (light == LIGHT_GREEN) {
        if (carsWaiting >= 2U) {
            carsWaiting -= 2U;
            carsPassed += 2U;
        } else if (carsWaiting == 1U) {
            carsWaiting -= 1U;
            carsPassed += 1U;
        }
    }

    if (ticksLeft > 0U) {
        ticksLeft--;
    }

    if (ticksLeft == 0U) {
        light = nextState(light);
        ticksLeft = ticksFor(light);
    }
}

static void addCars(void) {
    int input = -1;
    printf("Enter number of arriving cars: ");
    if (scanf("%d", &input) != 1 || input < 0 || input > 100) {
        printf("Refuse silly numbers! Invalid input.\n");
        while (getchar() != '\n');
        return;
    }

    carsWaiting += (uint8_t)input;
    if (carsWaiting > QUEUE_BUSY) {
        SET_BIT(status, BIT_BUSY);
    } else {
        CLR_BIT(status, BIT_BUSY);
    }
}

static void toggleNight(void) {
    if (READ_BIT(status, BIT_NIGHT)) {
        CLR_BIT(status, BIT_NIGHT);
        CLR_BIT(status, BIT_BLINK_ON);
        light = LIGHT_RED;
        ticksLeft = ticksFor(light);
        printf("Returned to Day mode. Light reset to Red with full timer.\n");
    } else {
        SET_BIT(status, BIT_NIGHT);
        SET_BIT(status, BIT_BLINK_ON);
        printf("Entered Night mode (blinking yellow).\n");
    }
}

static void crossingReport(void) {
    printf("\n=== Crossing Report ===\n");
    printf("Cars Passed: %u\n", carsPassed);
    printf("Cars Still Waiting: %u\n", carsWaiting);
    printf("Is Night: %s\n", READ_BIT(status, BIT_NIGHT) ? "Yes" : "No");
    printf("Is Busy: %s\n", READ_BIT(status, BIT_BUSY) ? "Yes" : "No");
    
    printf("Status Byte (Binary): ");
    for (int i = 7; i >= 0; i--) {
        printf("%d", (status >> i) & 1);
    }
    printf("\nStatus Byte (Hex): 0x%02X\n", status);
    
    printf("Log history: ");
    showLog();
}

int main(void) {
    int choice = 0;
    resetCrossing();

    do {
        drawLight();
        printf("\n--- Traffic Light Menu ---\n");
        printf("1. Simulate 1 Tick\n");
        printf("2. Simulate Multiple Ticks\n");
        printf("3. Add Cars\n");
        printf("4. Toggle Night Mode\n");
        printf("5. Show Crossing Report & Log\n");
        printf("6. Reset Crossing\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1:
                tick();
                break;
            case 2: {
                int t = 0;
                printf("Enter number of ticks to run: ");
                if (scanf("%d", &t) != 1 || t <= 0) {
                    printf("Invalid tick count.\n");
                    while (getchar() != '\n');
                    break;
                }
                for (int i = 0; i < t; i++) {
                    tick();
                }
                break;
            }
            case 3:
                addCars();
                break;
            case 4:
                toggleNight();
                break;
            case 5:
                crossingReport();
                break;
            case 6:
                resetCrossing();
                printf("Crossing reset to fresh state.\n");
                break;
            case 7:
                printf("Exiting program.\n");
                break;
            default:
                printf("Unknown option, try again.\n");
        }
    } while (choice != 7);

    return 0;
}