TARGET = js
SRC = main.cpp
DIR = /usr/local/bin

all:
	g++ -std=c++17 -o $(TARGET) $(SRC)
	sudo mv $(TARGET) $(DIR)

clean:
	rm -f $(TARGET)


