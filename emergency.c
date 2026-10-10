#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FILE_NAME "emergency_requests.dat"
#define MAX_TEXT 100
#define MAX_TEAMS 5

typedef struct
{
    char id[20];
    char location[MAX_TEXT];
    int people;
    char emergency[20];
    char resources[MAX_TEXT];
    char team[30];
    char status[30];
} Request;

Request *requests = NULL;
int count = 0;

const char *teams[MAX_TEAMS] = {
    "Team A", "Team B", "Team C", "Team D", "Team E"
};

void readText(const char *message, char *text, int size)
{
    printf("%s", message);
    if (fgets(text, size, stdin) != NULL)
    {
        text[strcspn(text, "\n")] = '\0';
    }
}

int readNumber(const char *message)
{
    char input[50];
    int value;

    while (1)
    {
        readText(message, input, sizeof(input));

        char *end;
        long number = strtol(input, &end, 10);

        while (isspace((unsigned char)*end))
            end++;

        if (end != input && *end == '\0' && number >= 0 &&
            number <= 1000000)
        {
            value = (int)number;
            return value;
        }

        printf("Enter a valid non-negative number.\n");
    }
}

int sameText(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (tolower((unsigned char)*a) !=
            tolower((unsigned char)*b))
            return 0;

        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}

void normalizeEmergency(char *level)
{
    if (sameText(level, "critical"))
        strcpy(level, "Critical");
    else if (sameText(level, "high"))
        strcpy(level, "High");
    else if (sameText(level, "medium"))
        strcpy(level, "Medium");
    else if (sameText(level, "low"))
        strcpy(level, "Low");
}

int getPriority(const char *level)
{
    if (sameText(level, "Critical"))
        return 1;
    if (sameText(level, "High"))
        return 2;
    if (sameText(level, "Medium"))
        return 3;
    if (sameText(level, "Low"))
        return 4;

    return 5;
}

void saveRequests(void)
{
    FILE *fp = fopen(FILE_NAME, "wb");

    if (fp == NULL)
    {
        printf("Error saving records.\n");
        return;
    }

    fwrite(&count, sizeof(int), 1, fp);

    if (count > 0)
        fwrite(requests, sizeof(Request), count, fp);

    fclose(fp);
}

void loadRequests(void)
{
    FILE *fp = fopen(FILE_NAME, "rb");

    if (fp == NULL)
        return;

    int savedCount = 0;

    if (fread(&savedCount, sizeof(int), 1, fp) != 1 ||
        savedCount < 0 || savedCount > 100000)
    {
        fclose(fp);
        printf("Invalid data file. Starting with empty records.\n");
        return;
    }

    if (savedCount > 0)
    {
        requests = malloc(savedCount * sizeof(Request));

        if (requests == NULL)
        {
            fclose(fp);
            printf("Memory allocation failed.\n");
            exit(1);
        }

        count = (int)fread(requests, sizeof(Request),
                           savedCount, fp);

        if (count != savedCount)
        {
            free(requests);
            requests = NULL;
            count = 0;
            printf("Incomplete data file. Starting with empty records.\n");
        }
    }

    fclose(fp);
}

int findRequest(const char *id)
{
    for (int i = 0; i < count; i++)
    {
        if (sameText(requests[i].id, id))
            return i;
    }

    return -1;
}

void registerRequest(void)
{
    Request r = {0};

    readText("Enter Request ID: ", r.id, sizeof(r.id));

    if (strlen(r.id) == 0 || findRequest(r.id) != -1)
    {
        printf("Invalid or duplicate Request ID.\n");
        return;
    }

    readText("Enter Location: ", r.location,
             sizeof(r.location));

    r.people = readNumber("Enter number of people affected: ");

    if (r.people == 0)
    {
        printf("People affected must be greater than zero.\n");
        return;
    }

    readText("Emergency level (Critical/High/Medium/Low): ",
             r.emergency, sizeof(r.emergency));

    normalizeEmergency(r.emergency);

    if (getPriority(r.emergency) == 5)
    {
        printf("Invalid emergency level.\n");
        return;
    }

    readText("Required resources: ", r.resources,
             sizeof(r.resources));

    strcpy(r.team, "None");
    strcpy(r.status, "Pending");

    Request *temp = realloc(requests,
                            (count + 1) * sizeof(Request));

    if (temp == NULL)
    {
        printf("Memory allocation failed.\n");
        return;
    }

    requests = temp;
    requests[count++] = r;

    saveRequests();
    printf("Emergency request registered successfully!\n");
}

int compareRequests(const void *a, const void *b)
{
    const Request *r1 = a;
    const Request *r2 = b;

    return getPriority(r1->emergency) -
           getPriority(r2->emergency);
}

void displayOne(Request r)
{
    printf("\nRequest ID : %s\n", r.id);
    printf("Location   : %s\n", r.location);
    printf("People     : %d\n", r.people);
    printf("Emergency  : %s\n", r.emergency);
    printf("Priority   : %d\n", getPriority(r.emergency));
    printf("Resources  : %s\n", r.resources);
    printf("Team       : %s\n", r.team);
    printf("Status     : %s\n", r.status);
}

void displayRequests(void)
{
    if (count == 0)
    {
        printf("No emergency requests found.\n");
        return;
    }

    qsort(requests, count, sizeof(Request), compareRequests);

    printf("\n--- Requests by Priority ---\n");

    for (int i = 0; i < count; i++)
        displayOne(requests[i]);
}

int teamIsAvailable(const char *team)
{
    for (int i = 0; i < count; i++)
    {
        if (sameText(requests[i].team, team) &&
            !sameText(requests[i].status, "Completed"))
        {
            return 0;
        }
    }

    return 1;
}

void assignTeam(void)
{
    char id[20];
    readText("Enter Request ID: ", id, sizeof(id));

    int index = findRequest(id);

    if (index == -1)
    {
        printf("Request not found.\n");
        return;
    }

    if (sameText(requests[index].status, "Completed"))
    {
        printf("This request is already completed.\n");
        return;
    }

    if (sameText(requests[index].team, "None"))
    {
        /* No team is assigned yet. */
    }
    else if (!teamIsAvailable(requests[index].team))
    {
        printf("This request already has an active team.\n");
        return;
    }

    printf("\nAvailable rescue teams:\n");

    int availableCount = 0;

    for (int i = 0; i < MAX_TEAMS; i++)
    {
        if (teamIsAvailable(teams[i]))
        {
            printf("%d. %s\n", i + 1, teams[i]);
            availableCount++;
        }
    }

    if (availableCount == 0)
    {
        printf("No rescue teams are currently available.\n");
        return;
    }

    int choice = readNumber("Select team number: ");

    if (choice < 1 || choice > MAX_TEAMS ||
        !teamIsAvailable(teams[choice - 1]))
    {
        printf("Invalid or unavailable team.\n");
        return;
    }

    strcpy(requests[index].team, teams[choice - 1]);
    strcpy(requests[index].status, "Assigned");

    saveRequests();
    printf("%s assigned to request %s.\n",
           requests[index].team, requests[index].id);
}

void updateStatus(void)
{
    char id[20];

    readText("Enter Request ID: ", id, sizeof(id));

    int index = findRequest(id);

    if (index == -1)
    {
        printf("Request not found.\n");
        return;
    }

    printf("\n1. Pending\n");
    printf("2. Assigned\n");
    printf("3. In Progress\n");
    printf("4. Completed\n");

    int choice = readNumber("Choose new status: ");

    switch (choice)
    {
        case 1:
            strcpy(requests[index].status, "Pending");
            strcpy(requests[index].team, "None");
            break;

        case 2:
            if (sameText(requests[index].team, "None"))
            {
                printf("Assign a rescue team first.\n");
                return;
            }
            strcpy(requests[index].status, "Assigned");
            break;

        case 3:
            if (sameText(requests[index].team, "None"))
            {
                printf("Assign a rescue team first.\n");
                return;
            }
            strcpy(requests[index].status, "In Progress");
            break;

        case 4:
            strcpy(requests[index].status, "Completed");
            break;

        default:
            printf("Invalid choice.\n");
            return;
    }

    saveRequests();
    printf("Request status updated successfully.\n");
}

void searchRequest(void)
{
    char id[20];

    readText("Enter Request ID to search: ", id, sizeof(id));

    int index = findRequest(id);

    if (index == -1)
        printf("Request not found.\n");
    else
        displayOne(requests[index]);
}

void emergencySummary(void)
{
    int critical = 0, high = 0, medium = 0, low = 0;
    int pending = 0, assigned = 0, progress = 0, completed = 0;
    int people = 0;

    for (int i = 0; i < count; i++)
    {
        people += requests[i].people;

        switch (getPriority(requests[i].emergency))
        {
            case 1: critical++; break;
            case 2: high++; break;
            case 3: medium++; break;
            case 4: low++; break;
        }

        if (sameText(requests[i].status, "Pending"))
            pending++;
        else if (sameText(requests[i].status, "Assigned"))
            assigned++;
        else if (sameText(requests[i].status, "In Progress"))
            progress++;
        else if (sameText(requests[i].status, "Completed"))
            completed++;
    }

    printf("\n--- Emergency Summary ---\n");
    printf("Total requests       : %d\n", count);
    printf("Total people affected: %d\n", people);
    printf("Critical requests    : %d\n", critical);
    printf("High requests        : %d\n", high);
    printf("Medium requests      : %d\n", medium);
    printf("Low requests         : %d\n", low);
    printf("Pending              : %d\n", pending);
    printf("Assigned             : %d\n", assigned);
    printf("In Progress          : %d\n", progress);
    printf("Completed            : %d\n", completed);
}

int main(void)
{
    loadRequests();

    int choice;

    do
    {
        printf("\n===== EMERGENCY RESCUE MANAGEMENT =====\n");
        printf("1. Register emergency request\n");
        printf("2. Display requests by priority\n");
        printf("3. Assign rescue team\n");
        printf("4. Update request status\n");
        printf("5. Search request\n");
        printf("6. Emergency summary\n");
        printf("7. Exit\n");

        choice = readNumber("Enter your choice: ");

        switch (choice)
        {
            case 1: registerRequest(); break;
            case 2: displayRequests(); break;
            case 3: assignTeam(); break;
            case 4: updateStatus(); break;
            case 5: searchRequest(); break;
            case 6: emergencySummary(); break;
            case 7:
                saveRequests();
                printf("Records saved. Exiting program.\n");
                break;
            default:
                printf("Invalid choice. Try again.\n");
        }
        
    } while (choice != 7);

    free(requests);
    return 0;
}