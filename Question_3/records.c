#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define FILE_NAME "employees.dat"
#define NAME_LEN 32

typedef struct {
    int  id;
    char name[NAME_LEN];
    float salary;
} Employee;

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void create_and_write(void) {
    int fd = open(FILE_NAME, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd < 0) die("open (create)");

    Employee staff[] = {
        {1, "Alice",   55000.0},
        {2, "Bob",     48000.0},
        {3, "Charlie", 62000.0}
    };

    for (int i = 0; i < 3; i++) {
        ssize_t written = write(fd, &staff[i], sizeof(Employee));
        if (written != sizeof(Employee)) die("write");
    }

    close(fd);
    printf("Created %s with %d records.\n", FILE_NAME, 3);
}

void update_salary(int id, float new_salary) {
    int fd = open(FILE_NAME, O_RDWR);
    if (fd < 0) die("open (update)");

    off_t offset = (id - 1) * sizeof(Employee);
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) die("lseek");

    Employee e;
    if (read(fd, &e, sizeof(Employee)) != sizeof(Employee)) die("read (update)");

    e.salary = new_salary;

    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) die("lseek (rewind)");
    if (write(fd, &e, sizeof(Employee)) != sizeof(Employee)) die("write (update)");

    close(fd);
    printf("Updated record id=%d -> salary=%.2f\n", id, new_salary);
}

void read_record(int id) {
    int fd = open(FILE_NAME, O_RDONLY);
    if (fd < 0) die("open (read)");

    off_t offset = (id - 1) * sizeof(Employee);
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) die("lseek (read)");

    Employee e;
    ssize_t r = read(fd, &e, sizeof(Employee));
    if (r == 0) {
        printf("No record found for id=%d\n", id);
    } else if (r != sizeof(Employee)) {
        die("read (record)");
    } else {
        printf("Record id=%d: name=%s, salary=%.2f\n", e.id, e.name, e.salary);
    }

    close(fd);
}

int main(void) {
    create_and_write();
    read_record(2);
    update_salary(2, 51000.0);
    read_record(2);
    read_record(3);
    return 0;
}
