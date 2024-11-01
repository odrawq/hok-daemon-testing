CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude/ -lcurl -lcjson
SRC_DIR = src/
BUILD_DIR = bin/
TARGET = hok-daemon
INSTALL_DIR = /usr/local/bin/
DATA_DIR = /var/lib/$(TARGET)/
DATA_FILE = data.json
LOG_DIR = /var/log/$(TARGET)/
LOG_FILE = error_log

build:
	mkdir -p $(BUILD_DIR)
	$(CC) $(shell find $(SRC_DIR) -type f -name "*.c") -o $(BUILD_DIR)$(TARGET) $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR)

install:
	@if ! id -u $(TARGET) >/dev/null 2>&1; then \
	    sudo useradd -r -s /bin/false $(TARGET); \
	fi

	sudo mkdir -p $(DATA_DIR)

	@if [ ! -f $(DATA_DIR)$(DATA_FILE) ]; then \
	    sudo touch $(DATA_DIR)$(DATA_FILE); \
	    echo "{}" | sudo tee $(DATA_DIR)$(DATA_FILE) > /dev/null; \
	fi

	sudo mkdir -p $(LOG_DIR)

	@if [ ! -f $(LOG_DIR)$(LOG_FILE) ]; then \
	    sudo touch $(LOG_DIR)$(LOG_FILE); \
	fi

	sudo chown -R $(TARGET):$(TARGET) $(LOG_DIR)
	sudo chown -R $(TARGET):$(TARGET) $(DATA_DIR)
	sudo cp $(BUILD_DIR)$(TARGET) $(INSTALL_DIR)
	sudo chown $(TARGET):$(TARGET) $(INSTALL_DIR)$(TARGET)

uninstall:
	sudo rm $(INSTALL_DIR)$(TARGET)
