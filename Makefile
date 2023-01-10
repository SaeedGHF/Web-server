CC=g++ -std=c++11
FLAGS=-w
SRC_DIR=src

server: $(SRC_DIR)/main.cpp
	$(CC) $(FLAGS) $(SRC_DIR)/main.cpp -o server

clean:
	-rm -f *.o
	-rm ./server