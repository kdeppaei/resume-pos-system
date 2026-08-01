CC = gcc
CFLAGS = -std=gnu90 -Wall -Wextra -Iinclude

APP_SOURCES = src/main.c src/input.c src/core.c src/storage.c src/catalog.c src/transaction.c src/checkout.c src/reports.c
TEST_SOURCES = tests/test_logic.c src/input.c src/core.c src/storage.c src/transaction.c

APP = resume_pos
TEST_APP = pos_tests

all: $(APP)

$(APP): $(APP_SOURCES)
	$(CC) $(CFLAGS) $(APP_SOURCES) -o $(APP)

$(TEST_APP): $(TEST_SOURCES)
	$(CC) $(CFLAGS) $(TEST_SOURCES) -o $(TEST_APP)

test: $(TEST_APP)
	./$(TEST_APP)

clean:
	rm -f $(APP) $(TEST_APP) *.exe *.o src/*.o tests/*.o

.PHONY: all test clean
