
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX 500
#define DOCTORS 5
#define BEDS 5
#define FILE_NAME "patients.dat"

typedef struct {
    char id[20];
    char name[50];
    int age;
    char condition[100];
    int priority;
    char status[20];
    char doctor[30];
    int bed;
    time_t registeredAt;
    time_t calledAt;
} Patient;

Patient patients[MAX];
int count = 0;

const char *doctorNames[DOCTORS] = {
    "Doctor A", "Doctor B", "Doctor C",
    "Doctor D", "Doctor E"
};

int validStatus(const char *status) {
    return strcmp(status, "Waiting") == 0 ||
           strcmp(status, "In Treatment") == 0 ||
           strcmp(status, "Treated") == 0;
}

void readLine(const char *prompt, char *buffer, int size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    if (strchr(buffer, '\0') == NULL)
        buffer[size - 1] = '\0';
}

int readNumber(const char *prompt, int min, int max) {
    char line[100];
    int value;
    char extra;

    while (1) {
        readLine(prompt, line, sizeof(line));

        if (sscanf(line, "%d %c", &value, &extra) == 1 &&
            value >= min && value <= max)
            return value;

        printf("Enter a number from %d to %d.\n", min, max);
    }
}

void saveData(void) {
    FILE *fp = fopen(FILE_NAME, "wb");

    if (fp == NULL) {
        printf("Warning: Could not save patient records.\n");
        return;
    }

    fwrite(&count, sizeof(count), 1, fp);
    fwrite(patients, sizeof(Patient), count, fp);
    fclose(fp);
}

void loadData(void) {
    FILE *fp = fopen(FILE_NAME, "rb");

    if (fp == NULL)
        return;

    int savedCount;

    if (fread(&savedCount, sizeof(int), 1, fp) != 1 ||
        savedCount < 0 || savedCount > MAX) {
        fclose(fp);
        printf("Invalid patient data file. Starting empty.\n");
        return;
    }

    if (fread(patients, sizeof(Patient), savedCount, fp)
        != (size_t)savedCount) {
        count = 0;
        printf("Could not read patient records.\n");
    } else {
        count = savedCount;
    }

    fclose(fp);
}

int findPatient(const char *id) {
    for (int i = 0; i < count; i++) {
        if (strcmp(patients[i].id, id) == 0)
            return i;
    }
    return -1;
}

int activeDoctorIndex(const char *doctor) {
    for (int i = 0; i < count; i++) {
        if (strcmp(patients[i].doctor, doctor) == 0 &&
            strcmp(patients[i].status, "In Treatment") == 0)
            return 1;
    }
    return 0;
}

int doctorAvailable(const char *doctor) {
    return !activeDoctorIndex(doctor);
}

int bedAvailable(int bed) {
    for (int i = 0; i < count; i++) {
        if (patients[i].bed == bed &&
            strcmp(patients[i].status, "In Treatment") == 0)
            return 0;
    }
    return 1;
}

void displayPatient(const Patient *p) {
    printf("\nPatient ID : %s\n", p->id);
    printf("Name       : %s\n", p->name);
    printf("Age        : %d\n", p->age);
    printf("Condition  : %s\n", p->condition);

    const char *levels[] = {
        "", "Critical", "High", "Medium", "Low"
    };

    printf("Priority   : %d - %s\n",
           p->priority, levels[p->priority]);
    printf("Status     : %s\n", p->status);
    printf("Doctor     : %s\n",
           strlen(p->doctor) ? p->doctor : "Not assigned");
    printf("Bed        : ");

    if (p->bed > 0)
        printf("%d\n", p->bed);
    else
        printf("Not assigned\n");

    printf("------------------------------\n");
}

void registerPatient(void) {
    if (count >= MAX) {
        printf("Patient storage is full.\n");
        return;
    }

    Patient p = {0};

    readLine("Enter patient ID: ", p.id, sizeof(p.id));

    if (strlen(p.id) == 0) {
        printf("Patient ID cannot be empty.\n");
        return;
    }

    if (findPatient(p.id) != -1) {
        printf("Patient ID already exists.\n");
        return;
    }

    readLine("Enter patient name: ", p.name, sizeof(p.name));

    if (strlen(p.name) == 0) {
        printf("Patient name cannot be empty.\n");
        return;
    }

    p.age = readNumber("Enter age (1-120): ", 1, 120);

    readLine("Enter condition: ",
             p.condition, sizeof(p.condition));

    if (strlen(p.condition) == 0) {
        printf("Condition cannot be empty.\n");
        return;
    }

    printf("\nPriority levels:\n");
    printf("1. Critical\n2. High\n3. Medium\n4. Low\n");

    p.priority = readNumber("Choose priority: ", 1, 4);

    strcpy(p.status, "Waiting");
    p.registeredAt = time(NULL);
    p.calledAt = 0;
    p.bed = 0;
    p.doctor[0] = '\0';

    patients[count++] = p;
    saveData();

    printf("\nPatient registered successfully!\n");
    displayPatient(&p);
}

void displayByPriority(void) {
    if (count == 0) {
        printf("No patients registered.\n");
        return;
    }

    Patient sorted[MAX];

    memcpy(sorted, patients, count * sizeof(Patient));

    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            int a = strcmp(sorted[j].status, "Waiting") == 0;
            int b = strcmp(sorted[j + 1].status, "Waiting") == 0;

            if ((!a && b) ||
                (a == b &&
                 sorted[j].priority > sorted[j + 1].priority)) {
                Patient temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }

    printf("\n=== PATIENTS BY PRIORITY ===\n");

    for (int i = 0; i < count; i++)
        displayPatient(&sorted[i]);
}

int findFreeDoctor(void) {
    for (int i = 0; i < DOCTORS; i++) {
        if (doctorAvailable(doctorNames[i]))
            return i;
    }
    return -1;
}

int findFreeBed(void) {
    for (int bed = 1; bed <= BEDS; bed++) {
        if (bedAvailable(bed))
            return bed;
    }
    return 0;
}

int assignResources(int index) {
    int doctor = findFreeDoctor();
    int bed = findFreeBed();

    if (doctor == -1 || bed == 0) {
        printf("No available doctor or bed currently.\n");
        return 0;
    }

    strcpy(patients[index].doctor, doctorNames[doctor]);
    patients[index].bed = bed;
    strcpy(patients[index].status, "In Treatment");

    if (patients[index].calledAt == 0)
        patients[index].calledAt = time(NULL);

    saveData();

    printf("Assigned %s and Bed %d.\n",
           doctorNames[doctor], bed);
    return 1;
}

void callNextPatient(void) {
    int index = -1;

    for (int i = 0; i < count; i++) {
        if (strcmp(patients[i].status, "Waiting") == 0 &&
            (index == -1 ||
             patients[i].priority < patients[index].priority)) {
            index = i;
        }
    }

    if (index == -1) {
        printf("No waiting patients.\n");
        return;
    }

    printf("\nNext highest-priority patient:\n");
    displayPatient(&patients[index]);

    printf("Assigning available doctor and bed...\n");

    if (assignResources(index))
        printf("Patient called successfully.\n");
}

void assignPatientResources(void) {
    char id[20];

    readLine("Enter patient ID: ", id, sizeof(id));

    int index = findPatient(id);

    if (index == -1) {
        printf("Patient not found.\n");
        return;
    }

    if (strcmp(patients[index].status, "Treated") == 0) {
        printf("This patient has already been treated.\n");
        return;
    }

    if (strcmp(patients[index].status, "In Treatment") == 0) {
        printf("Resources are already assigned.\n");
        return;
    }

    assignResources(index);
}

void updateStatus(void) {
    char id[20];

    readLine("Enter patient ID: ", id, sizeof(id));

    int index = findPatient(id);

    if (index == -1) {
        printf("Patient not found.\n");
        return;
    }

    printf("1. Waiting\n2. In Treatment\n3. Treated\n");

    int choice = readNumber("Choose new status: ", 1, 3);

    if (choice == 1) {
        if (strcmp(patients[index].status, "In Treatment") == 0) {
            patients[index].doctor[0] = '\0';
            patients[index].bed = 0;
        }
        strcpy(patients[index].status, "Waiting");
    } else if (choice == 2) {
        if (strcmp(patients[index].status, "In Treatment") != 0) {
            if (!assignResources(index))
                return;
        }
    } else {
        strcpy(patients[index].status, "Treated");
        patients[index].doctor[0] = '\0';
        patients[index].bed = 0;
    }

    saveData();
    printf("Patient status updated.\n");
    displayPatient(&patients[index]);
}

void searchPatient(void) {
    char id[20];

    readLine("Enter patient ID: ", id, sizeof(id));

    int index = findPatient(id);

    if (index == -1)
        printf("Patient not found.\n");
    else
        displayPatient(&patients[index]);
}

void summary(void) {
    int waiting = 0, treating = 0, treated = 0;
    int priorityCount[5] = {0};
    double totalWait = 0;
    int waitCount = 0;

    for (int i = 0; i < count; i++) {
        if (strcmp(patients[i].status, "Waiting") == 0)
            waiting++;
        else if (strcmp(patients[i].status, "In Treatment") == 0)
            treating++;
        else if (strcmp(patients[i].status, "Treated") == 0)
            treated++;

        priorityCount[patients[i].priority]++;

        if (patients[i].calledAt > 0 &&
            patients[i].registeredAt > 0) {
            double seconds = difftime(
                patients[i].calledAt,
                patients[i].registeredAt
            );

            if (seconds >= 0) {
                totalWait += seconds / 60.0;
                waitCount++;
            }
        }
    }

    printf("\n=== HOSPITAL SUMMARY ===\n");
    printf("Total patients : %d\n", count);
    printf("Waiting        : %d\n", waiting);
    printf("In Treatment   : %d\n", treating);
    printf("Treated        : %d\n", treated);

    printf("\nPriority-wise statistics:\n");
    printf("Critical       : %d\n", priorityCount[1]);
    printf("High           : %d\n", priorityCount[2]);
    printf("Medium         : %d\n", priorityCount[3]);
    printf("Low            : %d\n", priorityCount[4]);

    printf("\nDoctors available: ");
    int freeDoctors = 0;

    for (int i = 0; i < DOCTORS; i++) {
        if (doctorAvailable(doctorNames[i]))
            freeDoctors++;
    }

    printf("%d of %d\n", freeDoctors, DOCTORS);

    int freeBeds = 0;
    for (int bed = 1; bed <= BEDS; bed++) {
        if (bedAvailable(bed))
            freeBeds++;
    }

    printf("Beds available   : %d of %d\n", freeBeds, BEDS);

    if (waitCount > 0)
        printf("Average wait until called: %.2f minutes\n",
               totalWait / waitCount);
    else
        printf("Average wait until called: No data yet\n");
}

int main(void) {
    loadData();

    int choice;

    do {
        printf("\n===== SMART HOSPITAL EMERGENCY QUEUE =====\n");
        printf("1. Register Patient\n");
        printf("2. Display Patients by Priority\n");
        printf("3. Call Next Highest-Priority Patient\n");
        printf("4. Assign Doctor or Bed\n");
        printf("5. Update Patient Status\n");
        printf("6. Search Patient\n");
        printf("7. Waiting and Treatment Summary\n");
        printf("8. Exit\n");

        choice = readNumber("Enter your choice: ", 1, 8);

        switch (choice) {
            case 1: registerPatient(); break;
            case 2: displayByPriority(); break;
            case 3: callNextPatient(); break;
            case 4: assignPatientResources(); break;
            case 5: updateStatus(); break;
            case 6: searchPatient(); break;
            case 7: summary(); break;
            case 8:
                saveData();
                printf("Patient records saved. Exiting...\n");
                break;
        }
    } while (choice != 8);

    return 0;
}

