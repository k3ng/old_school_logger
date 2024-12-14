
/*

  Old School Command Line Logger !

  By Anthony Good K3NG

  How to compile:

  gcc logger.c -o logger -lsqlite3

  How to run:

  ./logger

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>  // For toupper
#include <sqlite3.h> // For SQLite3 database functions

#define INPUT_BUFFER_SIZE 256
#define CODE_VERSION "2024.12.14.16.01"


// Define a structure to hold contact details
typedef struct {
    char callsign_worked[50];
    char frequency[20];
    char sent_report[10];
    char received_report[10];
    char mode[20];
    char contact_date[20];
    char contact_time[20];
    char comment[100];
    unsigned int id;
} Contact;

// Function declarations
void display_help();
void display_title();
void display_current_contact(Contact *contact);
void get_current_date(char *buffer, size_t buffer_size);
void get_current_time(char *buffer, size_t buffer_size);
int initialize_database(const char *db_name);
int log_contact(const char *db_name, Contact *contact);
int view_contacts(const char *db_name, const char *params);

// Function to display the help message
void display_help() {
    printf("\nCommands:\n");
    printf("  h - Show this help message\n");
    printf("  a <ID> - erAse a contact by its ID (e.g., d 5)\n");
    printf("  e <filename> - Export logged contacts to a CSV file (e.g., e contacts.csv)\n");   
    printf("  i <filename> - Export the database in ADIF format (e.g., i log.adif)\n"); 
    printf("  l - Log a contact with the current settings\n");
    printf("  u <ID> - Load a contact by its ID for editing (e.g., u 5)\n");
    printf("  v - View logged contacts (options: v, v +N, v -N, v ID, v ID1-ID2)\n");
    printf("  x - Exit the program\n");

    printf("\nField Commands:\n");
    printf("  c - Set the callsign worked (e.g., c W3ABC)\n");
    printf("  f - Set the frequency (e.g., f 14.250)\n");
    printf("  s - Set the sent report (e.g., s 59)\n");
    printf("  r - Set the received report (e.g., r 59)\n");
    printf("  m - Set the mode (e.g., m USB, CW)\n");
    printf("  d - Set the contact date (default: today's date)\n");
    printf("  t - Set the contact time (default: current time)\n");
    printf("  n - Add a note (e.g., n This is my note; l)\n");

    printf("\nUsage:\n");
    printf("  Use the field commands to set individual fields.\n");
    printf("  Once all fields are set to your satisfaction, use 'l' to log the contact.\n");
    printf("  Use 'v' to view logged contacts in a tabular format.\n");
    printf("  Type 'x' to exit the program.\n\n");
}

// Function to display the program title
void display_title() {
    printf("K3NG's Old School Logger\n");
    printf(CODE_VERSION);
    printf("\r\n\r\n");
}

// Function to display the current contact details
void display_current_contact(Contact *contact) {
    printf("\nCurrent Contact Details:\n");
    printf("  Callsign Worked: %s\n", contact->callsign_worked);
    printf("  Frequency: %s\n", contact->frequency);
    printf("  Sent Report: %s\n", contact->sent_report);
    printf("  Received Report: %s\n", contact->received_report);
    printf("  Mode: %s\n", contact->mode);
    printf("  Contact Date: %s\n", contact->contact_date);
    printf("  Contact Time: %s\n", contact->contact_time);
    printf("  Note: %s\n\n", contact->comment);
}

// Function to get the current date in YYYY-MM-DD format
void get_current_date(char *buffer, size_t buffer_size) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, buffer_size, "%Y-%m-%d", tm_info);
}

// Function to get the current time in HH:MM:SS format
void get_current_time(char *buffer, size_t buffer_size) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, buffer_size, "%H:%M", tm_info);
}

// Function to initialize the SQLite3 database and create the table if it doesn't exist
int initialize_database(const char *db_name) {
    sqlite3 *db;
    char *err_msg = NULL;

    const char *sql_create_table = 
        "CREATE TABLE IF NOT EXISTS contacts ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "callsign TEXT NOT NULL, "
        "frequency TEXT, "
        "mode TEXT, "
        "sent_report TEXT, "
        "received_report TEXT, "
        "date_time TEXT NOT NULL, "
        "comment TEXT);";

    int rc = sqlite3_open(db_name, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    rc = sqlite3_exec(db, sql_create_table, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return rc;
    }

    sqlite3_close(db);
    return SQLITE_OK;
}

// Function to log the current contact into the SQLite3 database

int log_contact(const char *db_name, Contact *contact) {
    sqlite3 *db;
    char *err_msg = NULL;
    int rc;
    sqlite3_stmt *stmt;

    const char *sql_insert = 
        "INSERT INTO contacts (callsign, frequency, mode, sent_report, received_report, date_time, comment) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    const char *sql_update =
        "UPDATE contacts SET callsign = ?, frequency = ?, mode = ?, sent_report = ?, received_report = ?, "
        "date_time = ?, comment = ? WHERE id = ?";

    // Check if this is an update (comment contains an ID marker)
    //int update_id = 0;
    //if (sscanf(contact->comment, "UPDATE:%d", &update_id) == 1 && update_id > 0) {
    if (contact->id > 0) {
        rc = sqlite3_open(db_name, &db);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
            return rc;
        }

        rc = sqlite3_prepare_v2(db, sql_update, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare update statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return rc;
        }

        // Bind parameters for update
        sqlite3_bind_text(stmt, 1, contact->callsign_worked, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, contact->frequency, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, contact->mode, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, contact->sent_report, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, contact->received_report, -1, SQLITE_STATIC);

        char date_time[40];
        snprintf(date_time, sizeof(date_time), "%s %s", contact->contact_date, contact->contact_time);
        sqlite3_bind_text(stmt, 6, date_time, -1, SQLITE_STATIC);
        //sqlite3_bind_text(stmt, 7, contact->comment + 7, -1, SQLITE_STATIC); // Skip "UPDATE:"
        sqlite3_bind_text(stmt, 7, contact->comment, -1, SQLITE_STATIC);

        // Bind the ID
        sqlite3_bind_int(stmt, 8, contact->id);

        // Execute the statement
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return rc == SQLITE_DONE ? SQLITE_OK : rc;
    } else {
        rc = sqlite3_open(db_name, &db);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
            return rc;
        }

        rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return rc;
        }

        // Bind parameters to the prepared statement
        sqlite3_bind_text(stmt, 1, contact->callsign_worked, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, contact->frequency, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, contact->mode, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, contact->sent_report, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, contact->received_report, -1, SQLITE_STATIC);

        // Combine date and time into a single string
        char date_time[40];
        snprintf(date_time, sizeof(date_time), "%s %s", contact->contact_date, contact->contact_time);
        sqlite3_bind_text(stmt, 6, date_time, -1, SQLITE_STATIC);

        sqlite3_bind_text(stmt, 7, contact->comment, -1, SQLITE_STATIC);

        // Execute the statement
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "Failed to insert contact: %s\n", sqlite3_errmsg(db));
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return rc == SQLITE_DONE ? SQLITE_OK : rc;       
    }

    return 0;
}


// Function to view logged contacts in the SQLite3 database
int view_contacts(const char *db_name, const char *params) {
    // Query construction based on parameters (not shown for brevity)
    // Refer to previous code for detailed handling of 'v +N', 'v -N', etc.
    return 0; // Placeholder for simplicity
}

#include <ctype.h>  // For toupper
#include <string.h> // For strtok

#define INPUT_BUFFER_SIZE 256

#include <stdio.h>

// Function to export contacts to a CSV file
int export_contacts(const char *db_name, const char *file_name) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    FILE *file;
    int rc;

    const char *sql = "SELECT id, callsign, frequency, mode, sent_report, received_report, date_time, comment FROM contacts";

    rc = sqlite3_open(db_name, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    file = fopen(file_name, "w");
    if (file == NULL) {
        fprintf(stderr, "Cannot open file '%s' for writing.\n", file_name);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }

    // Write the CSV header
    fprintf(file, "ID,Callsign,Frequency,Mode,Sent Report,Received Report,Date/Time,Note\n");

    // Write each row to the file
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        fprintf(file, "%d,%s,%s,%s,%s,%s,%s,%s\n",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2),
                sqlite3_column_text(stmt, 3),
                sqlite3_column_text(stmt, 4),
                sqlite3_column_text(stmt, 5),
                sqlite3_column_text(stmt, 6),
                sqlite3_column_text(stmt, 7) ? (const char *)sqlite3_column_text(stmt, 7) : "");
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to retrieve contacts: %s\n", sqlite3_errmsg(db));
    }

    fclose(file);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    printf("Contacts successfully exported to '%s'.\n", file_name);
    return SQLITE_OK;
}

// Function to delete a contact by ID
int delete_contact(const char *db_name, int contact_id) {
    sqlite3 *db;
    char *err_msg = NULL;
    int rc;

    const char *sql = "DELETE FROM contacts WHERE id = ?";

    rc = sqlite3_open(db_name, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // Bind the contact ID to the prepared statement
    sqlite3_bind_int(stmt, 1, contact_id);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to delete contact: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Contact with ID %d has been deleted.\n", contact_id);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

// Function to load a contact from the database by ID
int load_contact(const char *db_name, int contact_id, Contact *contact) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    const char *sql = "SELECT callsign, frequency, mode, sent_report, received_report, "
                      "date_time, comment FROM contacts WHERE id = ?";

    rc = sqlite3_open(db_name, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    // Bind the contact ID to the prepared statement
    sqlite3_bind_int(stmt, 1, contact_id);

    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        snprintf(contact->callsign_worked, sizeof(contact->callsign_worked), "%s", (const char *)sqlite3_column_text(stmt, 0));
        snprintf(contact->frequency, sizeof(contact->frequency), "%s", (const char *)sqlite3_column_text(stmt, 1));
        snprintf(contact->mode, sizeof(contact->mode), "%s", (const char *)sqlite3_column_text(stmt, 2));
        snprintf(contact->sent_report, sizeof(contact->sent_report), "%s", (const char *)sqlite3_column_text(stmt, 3));
        snprintf(contact->received_report, sizeof(contact->received_report), "%s", (const char *)sqlite3_column_text(stmt, 4));

        const char *date_time = (const char *)sqlite3_column_text(stmt, 5);
        sscanf(date_time, "%19s %19s", contact->contact_date, contact->contact_time);

        snprintf(contact->comment, sizeof(contact->comment), "%s",
                 sqlite3_column_text(stmt, 6) ? (const char *)sqlite3_column_text(stmt, 6) : "");

        printf("Contact ID %d loaded into current fields.\n", contact_id);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return SQLITE_OK;
    } else {
        printf("No contact found with ID %d.\n", contact_id);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return SQLITE_NOTFOUND;
    }
}

// Function to set the contact date
void set_contact_date(Contact *contact) {
    printf("Enter the contact date (YYYY-MM-DD): ");
    scanf(" %19s", contact->contact_date); // Read a string with a max length of 19
    printf("Contact date set to '%s'.\n", contact->contact_date);
}

// Function to set the contact time
void set_contact_time(Contact *contact) {
    printf("Enter the contact time (HH:MM:SS): ");
    scanf(" %19s", contact->contact_time); // Read a string with a max length of 19
    printf("Contact time set to '%s'.\n", contact->contact_time);
}


int main() {
    char input[INPUT_BUFFER_SIZE];
    char *token;
    int running = 1;

    // Initialize a Contact structure with default values
    Contact current_contact = {"", "", "", "", "", "", "", "", 0};
    get_current_date(current_contact.contact_date, sizeof(current_contact.contact_date));
    get_current_time(current_contact.contact_time, sizeof(current_contact.contact_time));

    display_title();

    // Initialize the SQLite database
    const char *db_name = "contacts_logger.db";
    if (initialize_database(db_name) != SQLITE_OK) {
        fprintf(stderr, "Failed to initialize the database. Exiting.\n");
        return 1;
    }

    while (running) {
        display_current_contact(&current_contact);
        printf("> ");
        if (fgets(input, INPUT_BUFFER_SIZE, stdin) == NULL) {
            continue; // Handle EOF or empty input
        }

        // Remove trailing newline character from fgets
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        // Tokenize the input line
        token = strtok(input, " ");
        while (token != NULL) {
            switch (token[0]) {
                case 'a': {
                    token = strtok(NULL, " "); // Get the ID
                    if (token) {
                        int contact_id = atoi(token);
                        if (contact_id > 0) {
                            if (delete_contact(db_name, contact_id) == SQLITE_OK) {
                                printf("Delete successful.\n");
                            } else {
                                printf("Failed to delete contact.\n");
                            }
                        } else {
                            printf("Error: Invalid contact ID. ID must be a positive integer.\n");
                        }
                    } else {
                        printf("Error: No contact ID provided. Usage: d <ID>\n");
                    }
                    break;
                }                    
                case 'c':
                    token = strtok(NULL, " ");
                    if (token) {
                        // Convert the callsign to uppercase
                        for (int i = 0; token[i] != '\0'; i++) {
                            token[i] = toupper(token[i]);
                        }

                        // Copy the capitalized callsign to the structure
                        strncpy(current_contact.callsign_worked, token, sizeof(current_contact.callsign_worked) - 1);
                        current_contact.callsign_worked[sizeof(current_contact.callsign_worked) - 1] = '\0';
                        printf("Callsign set to '%s'.\n", current_contact.callsign_worked);
                    } else {
                        printf("Error: Callsign not provided.\n");
                        token = NULL;
                    }
                    break;

                case 'd': {
                    // Check if there's a space or directly appended date
                    token = strtok(NULL, " ");
                    if (!token) {
                        token = &input[1]; // Skip the 'd' character and use the rest of the input
                    }

                    if (token && strlen(token) > 0) {
                        int year = -1, month = -1, day = -1;

                        if (strchr(token, '-')) {
                            // Handle formats like YYYY-MM-DD
                            if (sscanf(token, "%4d-%2d-%2d", &year, &month, &day) == 3 &&
                                year >= 1900 && year <= 2100 &&
                                month >= 1 && month <= 12 &&
                                day >= 1 && day <= 31) {
                                snprintf(current_contact.contact_date, sizeof(current_contact.contact_date), "%04d-%02d-%02d", year, month, day);
                                printf("Contact date set to '%s'.\n", current_contact.contact_date);
                                break;
                            } else {
                                printf("Error: Invalid date format. Use YYYY-MM-DD.\n");
                            }
                        } else if (strchr(token, '/')) {
                            // Handle formats like YYYY/MM/DD
                            if (sscanf(token, "%4d/%2d/%2d", &year, &month, &day) == 3 &&
                                year >= 1900 && year <= 2100 &&
                                month >= 1 && month <= 12 &&
                                day >= 1 && day <= 31) {
                                snprintf(current_contact.contact_date, sizeof(current_contact.contact_date), "%04d-%02d-%02d", year, month, day);
                                printf("Contact date set to '%s'.\n", current_contact.contact_date);
                                break;
                            } else {
                                printf("Error: Invalid date format. Use YYYY/MM/DD.\n");
                            }
                        } else if (strlen(token) == 8) {
                            // Handle formats like YYYYMMDD
                            if (sscanf(token, "%4d%2d%2d", &year, &month, &day) == 3 &&
                                year >= 1900 && year <= 2100 &&
                                month >= 1 && month <= 12 &&
                                day >= 1 && day <= 31) {
                                snprintf(current_contact.contact_date, sizeof(current_contact.contact_date), "%04d-%02d-%02d", year, month, day);
                                printf("Contact date set to '%s'.\n", current_contact.contact_date);
                                break;
                            } else {
                                printf("Error: Invalid date format. Use YYYYMMDD.\n");
                            }
                        } else {
                            printf("Error: Unrecognized date format.\n");
                        }
                    } else {
                        // If no valid date was provided, set to today's date
                        get_current_date(current_contact.contact_date, sizeof(current_contact.contact_date));
                        printf("Contact date set to today's date: '%s'.\n", current_contact.contact_date);
                    }
                    break;
                }


                case 'e': {
                    token = strtok(NULL, " "); // Get the filename
                    if (token) {
                        if (export_contacts(db_name, token) == SQLITE_OK) {
                            printf("Export successful.\n");
                        } else {
                            printf("Export failed.\n");
                        }
                    } else {
                        printf("Error: No filename provided. Usage: e <filename>\n");
                    }
                    break;
                }

                case 'f':
                    token = strtok(NULL, " ");
                    if (token) {
                        strncpy(current_contact.frequency, token, sizeof(current_contact.frequency) - 1);
                        current_contact.frequency[sizeof(current_contact.frequency) - 1] = '\0';
                        printf("Frequency set to '%s'.\n", current_contact.frequency);
                    } else {
                        printf("Error: Frequency not provided.\n");
                    }
                    break;
                case 'h':
                    display_help();
                    break;



case 'i': {
    token = strtok(NULL, " "); // Get the filename
    if (token) {
        FILE *file = fopen(token, "w");
        if (!file) {
            printf("Error: Unable to open file '%s' for writing.\n", token);
            break;
        }

        // Open the database
        sqlite3 *db;
        sqlite3_stmt *stmt;
        const char *sql = "SELECT callsign, frequency, mode, sent_report, received_report, date_time, comment FROM contacts";
        if (sqlite3_open(db_name, &db) != SQLITE_OK) {
            fprintf(stderr, "Error: Cannot open database: %s\n", sqlite3_errmsg(db));
            fclose(file);
            break;
        }

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
            fprintf(stderr, "Error: Failed to prepare SQL statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            fclose(file);
            break;
        }

        // Write ADIF header
        fprintf(file, "<ADIF_VER:5>3.1.2\n");
        fprintf(file, "PROGRAMID:OSL\n");
        fprintf(file, "PROGRAMVERSION:%s\n\n", CODE_VERSION);

        // Process each row and write in ADIF format
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *callsign = (const char *)sqlite3_column_text(stmt, 0);
            const char *frequency = (const char *)sqlite3_column_text(stmt, 1);
            const char *mode = (const char *)sqlite3_column_text(stmt, 2);
            const char *sent_report = (const char *)sqlite3_column_text(stmt, 3);
            const char *received_report = (const char *)sqlite3_column_text(stmt, 4);
            const char *date_time = (const char *)sqlite3_column_text(stmt, 5);
            const char *comment = (const char *)sqlite3_column_text(stmt, 6);

            // Parse date_time to extract date (YYYYMMDD) and time (HHMM)
            char date[9] = "";
            char time[5] = "";
            int year, month, day, hour, minute;
            if (sscanf(date_time, "%4d-%2d-%2d %2d:%2d", &year, &month, &day, &hour, &minute) == 5) {
                snprintf(date, sizeof(date), "%04d%02d%02d", year, month, day); // Format as YYYYMMDD
                snprintf(time, sizeof(time), "%02d%02d", hour, minute);         // Format as HHMM
            } else {
                printf("Error: Invalid date_time format in database: '%s'\n", date_time);
                continue;
            }

            // Write ADIF entry
            fprintf(file,
                    "<QSO_DATE:8>%s <TIME_ON:4>%s <CALL:%d>%s "
                    "<MODE:%d>%s "
                    "<RST_SENT:%d>%s <RST_RCVD:%d>%s <COMMENT:%d>%s <EOR>\n",
                    date,
                    time,
                    (int)strlen(callsign), callsign,
                    (int)strlen(mode), mode,
                    (int)strlen(sent_report), sent_report,
                    (int)strlen(received_report), received_report,
                    comment ? (int)strlen(comment) : 0, comment ? comment : "");
        }


        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(file);
        printf("Database exported to '%s' in ADIF format.\n", token);
    } else {
        printf("Error: No filename provided. Usage: i <filename>\n");
    }
    break;
}


                case 'l':
                    if (log_contact(db_name, &current_contact) == SQLITE_OK) {
                        printf("Contact has been logged to the database.\n");

                        // Preserve frequency, mode, and date as defaults
                        char previous_frequency[20];
                        char previous_mode[20];
                        char previous_date[20];

                        strncpy(previous_frequency, current_contact.frequency, sizeof(previous_frequency));
                        strncpy(previous_mode, current_contact.mode, sizeof(previous_mode));
                        strncpy(previous_date, current_contact.contact_date, sizeof(previous_date));

                        // Reset current_contact but keep preserved defaults
                        current_contact = (Contact){"", "", "", "", "", "", "", "", 0};
                        strncpy(current_contact.frequency, previous_frequency, sizeof(current_contact.frequency));
                        strncpy(current_contact.mode, previous_mode, sizeof(current_contact.mode));
                        strncpy(current_contact.contact_date, previous_date, sizeof(current_contact.contact_date));

                        // Set current time as a default
                        get_current_time(current_contact.contact_time, sizeof(current_contact.contact_time));

                        printf("Ready for a new contact.\n");
                    } else {
                        printf("Failed to log the contact.\n");
                    }
                    break;                    
                case 'm':
                    token = strtok(NULL, " ");
                    if (token) {
                        strncpy(current_contact.mode, token, sizeof(current_contact.mode) - 1);
                        current_contact.mode[sizeof(current_contact.mode) - 1] = '\0';
                        for (int i = 0; current_contact.mode[i] != '\0'; i++) {
                            current_contact.mode[i] = toupper(current_contact.mode[i]);
                        }
                        printf("Mode set to '%s'.\n", current_contact.mode);
                    } else {
                        printf("Error: Mode not provided.\n");
                    }
                    break;
                case 'n': {
                    // The token already points to the "n" command, so get the rest of the line for the comment
                    token = strtok(NULL, ""); // Get the rest of the line after "n"
                    if (token) {
                        // Find the semicolon in the token
                        char *semicolon_pos = strchr(token, ';');
                        if (semicolon_pos) {
                            // Null-terminate the comment at the semicolon position
                            *semicolon_pos = '\0';

                            // Move to the next part of the input for further processing
                            char *remaining_commands = semicolon_pos + 1;

                            // Check if the current comment is empty
                            if (strlen(current_contact.comment) > 0) {
                                // Append the new comment with a separator
                                strncat(current_contact.comment, " | ", sizeof(current_contact.comment) - strlen(current_contact.comment) - 1);
                            }
                            // Append the new comment
                            strncat(current_contact.comment, token, sizeof(current_contact.comment) - strlen(current_contact.comment) - 1);

                            printf("Note updated: %s\n", current_contact.comment);

                            // Process the remaining commands
                            token = strtok(remaining_commands, " ");
                            continue;
                        } else {
                            // No semicolon; treat the entire token as the comment
                            if (strlen(current_contact.comment) > 0) {
                                // Append the new comment with a separator
                                strncat(current_contact.comment, " | ", sizeof(current_contact.comment) - strlen(current_contact.comment) - 1);
                            }
                            // Append the new comment
                            strncat(current_contact.comment, token, sizeof(current_contact.comment) - strlen(current_contact.comment) - 1);

                            printf("Comment updated: %s\n", current_contact.comment);
                        }
                    } else {
                        printf("Error: No comment provided.\n");
                    }
                    break;
                }
                case 'r':
                    token = strtok(NULL, " ");
                    if (token) {
                        strncpy(current_contact.received_report, token, sizeof(current_contact.received_report) - 1);
                        current_contact.received_report[sizeof(current_contact.received_report) - 1] = '\0';
                        printf("Received report set to '%s'.\n", current_contact.received_report);
                    } else {
                        printf("Error: Received report not provided.\n");
                    }
                    break;  
                case 's':
                    token = strtok(NULL, " ");
                    if (token) {
                        strncpy(current_contact.sent_report, token, sizeof(current_contact.sent_report) - 1);
                        current_contact.sent_report[sizeof(current_contact.sent_report) - 1] = '\0';
                        printf("Sent report set to '%s'.\n", current_contact.sent_report);
                    } else {
                        printf("Error: Sent report not provided.\n");
                    }
                    break;

                case 't': {
                    // Check if there's a space or directly appended time
                    token = strtok(NULL, " ");
                    if (!token) {
                        token = &input[1]; // Skip the 't' character and use the rest of the input
                    }

                    if (token && strlen(token) > 0) {
                        int hours = -1, minutes = 0, seconds = 0; // Default values for time

                        if (strchr(token, ':')) {
                            // Handle formats like HH:MM:SS or HH:MM
                            int count = sscanf(token, "%2d:%2d:%2d", &hours, &minutes, &seconds);
                            if ((count == 2 || count == 3) &&
                                hours >= 0 && hours <= 23 &&
                                minutes >= 0 && minutes <= 59 &&
                                seconds >= 0 && seconds <= 59) {
                                snprintf(current_contact.contact_time, sizeof(current_contact.contact_time), "%02d:%02d:%02d", hours, minutes, seconds);
                                printf("Contact time set to '%s'.\n", current_contact.contact_time);
                                break;
                            } else {
                                printf("Error: Invalid time format. Use HH:MM or HH:MM:SS.\n");
                            }
                        } else if (strlen(token) == 6) {
                            // Handle HHMMSS format
                            if (sscanf(token, "%2d%2d%2d", &hours, &minutes, &seconds) == 3 &&
                                hours >= 0 && hours <= 23 &&
                                minutes >= 0 && minutes <= 59 &&
                                seconds >= 0 && seconds <= 59) {
                                snprintf(current_contact.contact_time, sizeof(current_contact.contact_time), "%02d:%02d:%02d", hours, minutes, seconds);
                                printf("Contact time set to '%s'.\n", current_contact.contact_time);
                                break;
                            } else {
                                printf("Error: Invalid time format. Use HHMMSS.\n");
                            }
                        } else if (strlen(token) == 4) {
                            // Handle HHMM format
                            if (sscanf(token, "%2d%2d", &hours, &minutes) == 2 &&
                                hours >= 0 && hours <= 23 &&
                                minutes >= 0 && minutes <= 59) {
                                snprintf(current_contact.contact_time, sizeof(current_contact.contact_time), "%02d:%02d:%02d", hours, minutes, 0);
                                printf("Contact time set to '%s'.\n", current_contact.contact_time);
                                break;
                            } else {
                                printf("Error: Invalid time format. Use HHMM.\n");
                            }
                        } else if (strlen(token) == 2) {
                            // Handle HH format (assume 00 minutes and 00 seconds)
                            if (sscanf(token, "%2d", &hours) == 1 && hours >= 0 && hours <= 23) {
                                snprintf(current_contact.contact_time, sizeof(current_contact.contact_time), "%02d:%02d:%02d", hours, 0, 0);
                                printf("Contact time set to '%s'.\n", current_contact.contact_time);
                                break;
                            } else {
                                printf("Error: Invalid time format. Use HH.\n");
                            }
                        } else {
                            printf("Error: Unrecognized time format.\n");
                        }
                    } else {
                        // If no valid time was provided, set to the current time
                        get_current_time(current_contact.contact_time, sizeof(current_contact.contact_time));
                        printf("Contact time set to current time: '%s'.\n", current_contact.contact_time);
                    }
                    break;
                }





                case 'u': {
                    token = strtok(NULL, " "); // Get the ID
                    if (token) {
                        int contact_id = atoi(token);
                        if (contact_id > 0) {
                            if (load_contact(db_name, contact_id, &current_contact) == SQLITE_OK) {
                                // Set the comment field to indicate this is an update
                                //snprintf(current_contact.comment, sizeof(current_contact.comment), "UPDATE:%d", contact_id);
                                current_contact.id = contact_id;
                                printf("You can now edit the current fields and use 'l' to save the changes.\n");
                            }
                        } else {
                            printf("Error: Invalid contact ID. ID must be a positive integer.\n");
                        }
                    } else {
                        printf("Error: No contact ID provided. Usage: u <ID>\n");
                    }
                    break;
                }

                case 'v': {
                    token = strtok(NULL, ""); // Get the rest of the line after "v"
                    char query[512] = "SELECT id, callsign, frequency, mode, sent_report, received_report, date_time, comment FROM contacts";
                    int has_condition = 0;

                    if (token) {
                        // Handle different types of parameters
                        if (token[0] == '+') {
                            // First N contacts
                            int limit = atoi(token + 1);
                            if (limit > 0) {
                                snprintf(query + strlen(query), sizeof(query) - strlen(query), " ORDER BY id LIMIT %d", limit);
                                has_condition = 1;
                            } else {
                                printf("Invalid parameter for '+N'. Showing all contacts.\n");
                            }
                        } else if (token[0] == '-') {
                            // Last N contacts
                            int limit = atoi(token + 1);
                            if (limit > 0) {
                                snprintf(query + strlen(query), sizeof(query) - strlen(query), " ORDER BY id DESC LIMIT %d", limit);
                                has_condition = 1;
                            } else {
                                printf("Invalid parameter for '-N'. Showing all contacts.\n");
                            }
                        } else if (strchr(token, '-')) {
                            // Range of IDs
                            int start_id, end_id;
                            if (sscanf(token, "%d-%d", &start_id, &end_id) == 2 && start_id > 0 && end_id >= start_id) {
                                snprintf(query + strlen(query), sizeof(query) - strlen(query),
                                         " WHERE id BETWEEN %d AND %d ORDER BY id", start_id, end_id);
                                has_condition = 1;
                            } else {
                                printf("Invalid range for 'ID-Range'. Showing all contacts.\n");
                            }
                        } else if (isdigit(token[0])) {
                            // Single ID
                            int id = atoi(token);
                            if (id > 0) {
                                snprintf(query + strlen(query), sizeof(query) - strlen(query), " WHERE id = %d ORDER BY id", id);
                                has_condition = 1;
                            } else {
                                printf("Invalid parameter for 'ID'. Showing all contacts.\n");
                            }
                        } else {
                            printf("Unknown parameter. Showing all contacts.\n");
                        }
                    }

                    // Add a default ORDER BY clause if no specific condition was provided
                    if (!has_condition) {
                        snprintf(query + strlen(query), sizeof(query) - strlen(query), " ORDER BY id");
                    }

                    // Execute the query
                    sqlite3 *db;
                    sqlite3_stmt *stmt;
                    int rc = sqlite3_open(db_name, &db);
                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
                        break;
                    }

                    printf("Executing query: %s\n", query); // Debugging line to check the query

                    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
                        sqlite3_close(db);
                        break;
                    }

                    printf("\nLogged Contacts:\n");
                    printf("| ID | Call Sign  | Frequency | Mode | Sent Rpt  | Recv Rpt  | Date/Time         | Notes \n");

                    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                        int id = sqlite3_column_int(stmt, 0);
                        const char *callsign = (const char *)sqlite3_column_text(stmt, 1);
                        const char *frequency = (const char *)sqlite3_column_text(stmt, 2);
                        const char *mode = (const char *)sqlite3_column_text(stmt, 3);
                        const char *sent_report = (const char *)sqlite3_column_text(stmt, 4);
                        const char *received_report = (const char *)sqlite3_column_text(stmt, 5);
                        const char *date_time = (const char *)sqlite3_column_text(stmt, 6);
                        const char *comment = (const char *)sqlite3_column_text(stmt, 7);

                        printf("| %-2d | %-10s | %-9s | %-4s | %-9s | %-9s | %-17s | %-12s \n",
                               id, callsign ? callsign : "",
                               frequency ? frequency : "",
                               mode ? mode : "",
                               sent_report ? sent_report : "",
                               received_report ? received_report : "",
                               date_time ? date_time : "",
                               comment ? comment : "");
                    }


                    if (rc != SQLITE_DONE) {
                        fprintf(stderr, "Failed to retrieve contacts: %s\n", sqlite3_errmsg(db));
                    }

                    sqlite3_finalize(stmt);
                    sqlite3_close(db);
                    break;
                }

                case 'x':
                    printf("Exiting the program.\n");
                    running = 0;
                    break;
                default:
                    printf("Unknown command '%s'. Type 'h' for help.\n", token);
            }

            token = strtok(NULL, " "); // Move to the next token
        }
    }

    return 0;
}

