
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 500
#define FILE_NAME "complaints.dat"

typedef struct {
    char id[20];
    char category[40];
    char problem[150];
    char location[80];
    char priority[20];
    char status[20];
} Complaint;

Complaint complaints[MAX];
int count = 0;

int priorityValue(const char *p) {
    if (strcmp(p, "Critical") == 0) return 4;
    if (strcmp(p, "High") == 0) return 3;
    if (strcmp(p, "Medium") == 0) return 2;
    return 1;
}

void saveData(void) {
    FILE *fp = fopen(FILE_NAME, "wb");
    if (fp == NULL) {
        printf("Error saving file.\n");
        return;
    }
    fwrite(&count, sizeof(int), 1, fp);
    fwrite(complaints, sizeof(Complaint), count, fp);
    fclose(fp);
}

void loadData(void) {
    FILE *fp = fopen(FILE_NAME, "rb");
    if (fp == NULL) return;

    if (fread(&count, sizeof(int), 1, fp) != 1 ||
        count < 0 || count > MAX) {
        count = 0;
        fclose(fp);
        return;
    }

    if (fread(complaints, sizeof(Complaint), count, fp) !=
        (size_t)count) {
        count = 0;
    }
    fclose(fp);
}

int findComplaint(const char *id) {
    for (int i = 0; i < count; i++) {
        if (strcmp(complaints[i].id, id) == 0)
            return i;
    }
    return -1;
}

void displayComplaint(Complaint c) {
    printf("\nID       : %s\n", c.id);
    printf("Category : %s\n", c.category);
    printf("Problem  : %s\n", c.problem);
    printf("Location : %s\n", c.location);
    printf("Priority : %s\n", c.priority);
    printf("Status   : %s\n", c.status);
    printf("------------------------------\n");
}

void registerComplaint(void) {
    if (count >= MAX) {
        printf("Complaint storage is full.\n");
        return;
    }

    Complaint c;

    printf("Enter complaint ID: ");
    scanf(" %19s", c.id);

    if (findComplaint(c.id) != -1) {
        printf("Complaint ID already exists.\n");
        return;
    }

    printf("Enter category (Laboratory/Classroom/Hostel/Transport/Network/Electricity/Other): ");
    scanf(" %39[^\n]", c.category);

    printf("Enter problem: ");
    scanf(" %149[^\n]", c.problem);

    printf("Enter location: ");
    scanf(" %79[^\n]", c.location);

    int severity;
    printf("Select severity (1=Low, 2=Medium, 3=High, 4=Critical): ");
    if (scanf("%d", &severity) != 1) {
        printf("Invalid severity input.\n");
        while (getchar() != '\n');
        return;
    }

    if (severity < 1 || severity > 4) {
        printf("Invalid severity. Use 1 to 4.\n");
        return;
    }

    const char *levels[] = {"", "Low", "Medium", "High", "Critical"};
    strcpy(c.priority, levels[severity]);
    strcpy(c.status, "Pending");

    complaints[count++] = c;
    saveData();

    printf("Complaint registered successfully!\n");
    displayComplaint(c);
}

void displayAll(void) {
    if (count == 0) {
        printf("No complaints registered.\n");
        return;
    }

    for (int i = 0; i < count; i++)
        displayComplaint(complaints[i]);
}

void sortComplaints(void) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (priorityValue(complaints[j].priority) <
                priorityValue(complaints[j + 1].priority)) {
                Complaint temp = complaints[j];
                complaints[j] = complaints[j + 1];
                complaints[j + 1] = temp;
            }
        }
    }

    saveData();
    printf("\nComplaints sorted by priority (highest first):\n");
    displayAll();
}

void searchComplaint(void) {
    char id[20];
    printf("Enter complaint ID to search: ");
    scanf(" %19s", id);

    int index = findComplaint(id);
    if (index == -1)
        printf("Complaint not found.\n");
    else
        displayComplaint(complaints[index]);
}

void updateStatus(void) {
    char id[20];
    int choice;

    printf("Enter complaint ID: ");
    scanf(" %19s", id);

    int index = findComplaint(id);
    if (index == -1) {
        printf("Complaint not found.\n");
        return;
    }

    printf("1. Pending\n2. In Progress\n3. Resolved\n");
    printf("Choose new status: ");
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }

    if (choice == 1)
        strcpy(complaints[index].status, "Pending");
    else if (choice == 2)
        strcpy(complaints[index].status, "In Progress");
    else if (choice == 3)
        strcpy(complaints[index].status, "Resolved");
    else {
        printf("Invalid choice.\n");
        return;
    }

    saveData();
    printf("Status updated successfully.\n");
    displayComplaint(complaints[index]);
}

void displayPending(void) {
    int found = 0;

    printf("\n--- PENDING COMPLAINTS ---\n");
    for (int i = 0; i < count; i++) {
        if (strcmp(complaints[i].status, "Pending") == 0) {
            displayComplaint(complaints[i]);
            found = 1;
        }
    }

    if (!found)
        printf("No pending complaints.\n");
}

void filterCategory(void) {
    char category[40];
    int found = 0;

    printf("Enter category to filter: ");
    scanf(" %39[^\n]", category);

    for (int i = 0; i < count; i++) {
        if (strcmp(complaints[i].category, category) == 0) {
            displayComplaint(complaints[i]);
            found = 1;
        }
    }

    if (!found)
        printf("No complaints found in that category.\n");
}

void summary(void) {
    int pending = 0, progress = 0, resolved = 0;
    int critical = 0, high = 0, medium = 0, low = 0;

    for (int i = 0; i < count; i++) {
        if (strcmp(complaints[i].status, "Pending") == 0)
            pending++;
        else if (strcmp(complaints[i].status, "In Progress") == 0)
            progress++;
        else if (strcmp(complaints[i].status, "Resolved") == 0)
            resolved++;

        if (strcmp(complaints[i].priority, "Critical") == 0)
            critical++;
        else if (strcmp(complaints[i].priority, "High") == 0)
            high++;
        else if (strcmp(complaints[i].priority, "Medium") == 0)
            medium++;
        else
            low++;
    }

    printf("\n===== COMPLAINT SUMMARY =====\n");
    printf("Total complaints : %d\n", count);
    printf("Pending          : %d\n", pending);
    printf("In Progress      : %d\n", progress);
    printf("Resolved         : %d\n", resolved);
    printf("Critical         : %d\n", critical);
    printf("High             : %d\n", high);
    printf("Medium           : %d\n", medium);
    printf("Low              : %d\n", low);

    if (count > 0)
        printf("Resolution rate  : %.2f%%\n",
               (double)resolved * 100 / count);
}

int main(void) {
    int choice;
    loadData();

    do {
        printf("\n===== SMART CAMPUS COMPLAINT SYSTEM =====\n");
        printf("1. Register Complaint\n");
        printf("2. Display All Complaints\n");
        printf("3. Sort by Priority\n");
        printf("4. Search Complaint\n");
        printf("5. Update Complaint Status\n");
        printf("6. Display Pending Complaints\n");
        printf("7. Filter by Category\n");
        printf("8. Generate Summary\n");
        printf("9. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Enter a number.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1: registerComplaint(); break;
            case 2: displayAll(); break;
            case 3: sortComplaints(); break;
            case 4: searchComplaint(); break;
            case 5: updateStatus(); break;
            case 6: displayPending(); break;
            case 7: filterCategory(); break;
            case 8: summary(); break;
            case 9: saveData(); printf("Data saved. Exiting...\n"); break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 9);

    return 0;
}
