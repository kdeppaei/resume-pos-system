#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage.h"

void database_initialize(Database *database) {
    memset(database, 0, sizeof(Database));
    database->version = DATABASE_VERSION;
    database->next_transaction_id = 1;
}

Bool database_validate(const Database *database) {
    int i;

    if (database == NULL || database->version != DATABASE_VERSION) {
        return FALSE;
    }

    if (database->transaction_start < 0 ||
        database->transaction_start >= MAX_TRANSACTIONS ||
        database->transaction_count < 0 ||
        database->transaction_count > MAX_TRANSACTIONS ||
        database->next_transaction_id < 1 ||
        database->total_revenue < 0) {
        return FALSE;
    }

    for (i = 0; i < MAX_ITEMS; i++) {
        if (!database->items[i].occupied) {
            continue;
        }

        if (database->items[i].code[0] == '\0' ||
            database->items[i].name[0] == '\0' ||
            database->items[i].stock < 0 ||
            database->items[i].price < 0) {
            return FALSE;
        }
    }

    return TRUE;
}

static Bool load_database_file(const char *filename, Database *target) {
    FILE *file;
    size_t read_count;
    int extra_byte;

    file = fopen(filename, "rb");
    if (file == NULL) {
        return FALSE;
    }

    read_count = fread(target, sizeof(Database), 1, file);
    extra_byte = fgetc(file);
    fclose(file);

    if (read_count != 1 || extra_byte != EOF || !database_validate(target)) {
        return FALSE;
    }

    return TRUE;
}

Bool database_save(const Database *database) {
    FILE *file;
    size_t written;
    int flush_result;
    int close_result;
    Bool original_exists;

    if (!database_validate(database)) {
        printf("Database validation failed; save was cancelled.\n");
        return FALSE;
    }

    file = fopen(TEMP_FILE, "wb");
    if (file == NULL) {
        perror("Could not create temporary database");
        return FALSE;
    }

    written = fwrite(database, sizeof(Database), 1, file);
    flush_result = fflush(file);
    close_result = fclose(file);

    if (written != 1 || flush_result != 0 || close_result != 0) {
        remove(TEMP_FILE);
        printf("Database write failed.\n");
        return FALSE;
    }

    original_exists = FALSE;
    file = fopen(DATA_FILE, "rb");
    if (file != NULL) {
        original_exists = TRUE;
        fclose(file);
    }

    remove(BACKUP_FILE);

    if (original_exists) {
        if (rename(DATA_FILE, BACKUP_FILE) != 0) {
            remove(TEMP_FILE);
            perror("Could not create database backup");
            return FALSE;
        }
    }

    if (rename(TEMP_FILE, DATA_FILE) != 0) {
        perror("Could not activate new database");

        if (original_exists) {
            rename(BACKUP_FILE, DATA_FILE);
        }

        remove(TEMP_FILE);
        return FALSE;
    }

    return TRUE;
}

void database_load(Database *database) {
    Database *loaded;

    loaded = (Database *)malloc(sizeof(Database));
    if (loaded == NULL) {
        printf("Not enough memory to load database.\n");
        exit(EXIT_FAILURE);
    }

    if (load_database_file(DATA_FILE, loaded)) {
        memcpy(database, loaded, sizeof(Database));
        free(loaded);
        printf("POS database loaded.\n");
        return;
    }

    if (load_database_file(BACKUP_FILE, loaded)) {
        memcpy(database, loaded, sizeof(Database));
        free(loaded);
        printf("Main database was unavailable. Backup restored.\n");
        database_save(database);
        return;
    }

    free(loaded);
    database_initialize(database);
    printf("New POS database created.\n");
}
