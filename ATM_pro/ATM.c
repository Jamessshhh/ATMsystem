#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

#define MAX_USERS 5
#define PIN_LENGTH 4
#define TRANSACTION_HISTORY_SIZE 10
#define MAX_LOGIN_ATTEMPTS 3

typedef struct {
    char username[20];
    char pin[PIN_LENGTH + 1];
    float balance;
    char transactionHistory[TRANSACTION_HISTORY_SIZE][100];
    int transactionCount;
} User;

// Function to securely take PIN input (Linux/macOS)
void getMaskedInput(char *pin, int length) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    for (int i = 0; i < length; i++) {
        pin[i] = getchar();
        printf("*");
    }
    pin[length] = '\0';

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}

// Function to authenticate user
int authenticateUser(User users[], int numUsers, char username[], char pin[]) {
    for (int i = 0; i < numUsers; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].pin, pin) == 0) {
            return i;
        }
    }
    return -1;
}

// Function to record a transaction
void recordTransaction(User *user, const char *transactionType, float amount) {
    time_t t;
    struct tm *timestamp;
    time(&t);
    timestamp = localtime(&t);

    char transaction[100];
    snprintf(transaction, sizeof(transaction), "%02d-%02d-%04d %02d:%02d:%02d - %s $%.2f | Balance: $%.2f",
             timestamp->tm_mday, timestamp->tm_mon + 1, timestamp->tm_year + 1900,
             timestamp->tm_hour, timestamp->tm_min, timestamp->tm_sec,
             transactionType, amount, user->balance);

    if (user->transactionCount < TRANSACTION_HISTORY_SIZE) {
        strcpy(user->transactionHistory[user->transactionCount], transaction);
        user->transactionCount++;
    } else {
        for (int i = 1; i < TRANSACTION_HISTORY_SIZE; i++) {
            strcpy(user->transactionHistory[i - 1], user->transactionHistory[i]);
        }
        strcpy(user->transactionHistory[TRANSACTION_HISTORY_SIZE - 1], transaction);
    }
}

// Function to display transaction history
void displayTransactionHistory(User *user) {
    printf("\nTransaction History for %s:\n", user->username);
    for (int i = 0; i < user->transactionCount; i++) {
        printf("%s\n", user->transactionHistory[i]);
    }
}

// Function to print account details
void printAccountDetails(User *user) {
    printf("\nUsername: %s\n", user->username);
    printf("Balance: $%.2f\n", user->balance);
}

int main() {
    User users[MAX_USERS] = {
        {"Rohit", "1234", 10000.0, {0}, 0},
        {"Prakash", "5002", 10500.0, {0}, 0},
        {"Ritesh", "1212", 150000.0, {0}, 0},
        {"Eshita", "1111", 1500000.0, {0}, 0},
        {"Karan", "2222", 20000.0, {0}, 0}
    };

    char username[20], pin[PIN_LENGTH + 1];
    int userIndex = -1;

    // User Authentication
    for (int attempts = 1; attempts <= MAX_LOGIN_ATTEMPTS; attempts++) {
        printf("\nATM LOGIN (%d/%d attempts)\n", attempts, MAX_LOGIN_ATTEMPTS);
        printf("Enter username: ");
        scanf("%s", username);
        getchar(); // Clear newline left by scanf

        printf("Enter PIN: ");
        getMaskedInput(pin, PIN_LENGTH);

        userIndex = authenticateUser(users, MAX_USERS, username, pin);

        if (userIndex != -1) {
            printf("\nLogin successful!\n");
            break;
        } else {
            printf("Authentication failed. Please try again.\n");
        }
    }

    if (userIndex == -1) {
        printf("\nMaximum login attempts reached. Exiting ATM.\n");
        return 0;
    }

    int choice;
    float amount;
    char newPin[PIN_LENGTH + 1];

    // ATM Menu
    while (1) {
        printf("\n1. Display Balance\n");
        printf("2. Deposit\n");
        printf("3. Withdraw\n");
        printf("4. Transfer Funds\n");
        printf("5. Change PIN\n");
        printf("6. Transaction History\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("\nCurrent Balance: $%.2f\n", users[userIndex].balance);
                break;

            case 2:
                printf("Enter amount to deposit: $");
                scanf("%f", &amount);
                if (amount > 0) {
                    users[userIndex].balance += amount;
                    printf("Deposited $%.2f\n", amount);
                    recordTransaction(&users[userIndex], "Deposit", amount);
                } else {
                    printf("Invalid amount!\n");
                }
                break;

            case 3:
                printf("Enter amount to withdraw: $");
                scanf("%f", &amount);
                if (amount > 0 && amount <= users[userIndex].balance) {
                    users[userIndex].balance -= amount;
                    printf("Withdrawn $%.2f\n", amount);
                    recordTransaction(&users[userIndex], "Withdraw", amount);
                } else {
                    printf("Insufficient funds or invalid amount!\n");
                }
                break;

            case 4: {
                char recipient[20];
                printf("Enter recipient's username: ");
                scanf("%s", recipient);

                int recipientIndex = -1;
                for (int i = 0; i < MAX_USERS; i++) {
                    if (strcmp(users[i].username, recipient) == 0) {
                        recipientIndex = i;
                        break;
                    }
                }

                if (recipientIndex != -1 && recipientIndex != userIndex) {
                    printf("Enter amount to transfer: $");
                    scanf("%f", &amount);
                    if (amount > 0 && amount <= users[userIndex].balance) {
                        users[userIndex].balance -= amount;
                        users[recipientIndex].balance += amount;
                        printf("Transferred $%.2f to %s\n", amount, users[recipientIndex].username);
                        recordTransaction(&users[userIndex], "Transfer", amount);
                        recordTransaction(&users[recipientIndex], "Received", amount);
                    } else {
                        printf("Insufficient funds or invalid amount!\n");
                    }
                } else {
                    printf("Invalid recipient!\n");
                }
                break;
            }

            case 5:
                printf("Enter new 4-digit PIN: ");
                getMaskedInput(newPin, PIN_LENGTH);
                strncpy(users[userIndex].pin, newPin, PIN_LENGTH);
                users[userIndex].pin[PIN_LENGTH] = '\0';
                printf("PIN changed successfully!\n");
                break;

            case 6:
                displayTransactionHistory(&users[userIndex]);
                break;

            case 7:
                printf("\nThank you for using MyATM. Have a nice day!\n");
                return 0;

            default:
                printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}
